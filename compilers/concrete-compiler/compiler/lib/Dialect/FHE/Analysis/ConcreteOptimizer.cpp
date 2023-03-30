// Part of the Concrete Compiler Project, under the BSD3 License with Zama
// Exceptions. See
// https://github.com/zama-ai/concrete-compiler-internal/blob/main/LICENSE.txt
// for license information.

#include <chrono>
#include <cmath>
#include <initializer_list>
#include <optional>
#include <vector>

#include "boost/outcome.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Pass/PassManager.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "concrete-optimizer.hpp"

#include "concretelang/Common/Error.h"
#include "concretelang/Dialect/FHE/Analysis/ConcreteOptimizer.h"
#include "concretelang/Dialect/FHE/Analysis/utils.h"
#include "concretelang/Dialect/FHE/IR/FHEOps.h"
#include "concretelang/Dialect/FHE/IR/FHETypes.h"
#include "concretelang/Dialect/FHELinalg/IR/FHELinalgOps.h"
#include "concretelang/Dialect/Tracing/IR/TracingOps.h"
#include "concretelang/Support/V0Parameters.h"
#include "concretelang/Support/logging.h"

#define GEN_PASS_CLASSES
#include "concretelang/Dialect/FHE/Analysis/ConcreteOptimizer.h.inc"

namespace mlir {
namespace concretelang {
namespace optimizer {

namespace {

template <typename T> rust::Slice<const T> slice(const std::vector<T> &vec) {
  return rust::Slice<const T>(vec.data(), vec.size());
}

template <typename T> rust::Slice<const T> slice(const llvm::ArrayRef<T> &vec) {
  return rust::Slice<const T>(vec.data(), vec.size());
}

struct FunctionToDag {
  // Inputs of operators
  using Inputs = std::vector<concrete_optimizer::dag::OperatorIndex>;

  const double NEGLIGIBLE_COMPLEXITY = 0.0;

  mlir::func::FuncOp func;
  optimizer::Config config;
  llvm::DenseMap<mlir::Value, concrete_optimizer::dag::OperatorIndex> index;
  bool setOptimizerID;

  FunctionToDag(mlir::func::FuncOp func, optimizer::Config config)
      : func(func), config(config) {
    setOptimizerID = config.strategy == optimizer::Strategy::DAG_MULTI;
  }

#define DEBUG(MSG)                                                             \
  if (mlir::concretelang::isVerbose()) {                                       \
    mlir::concretelang::log_verbose() << MSG << "\n";                          \
  }

  outcome::checked<std::optional<optimizer::Dag>,
                   ::concretelang::error::StringError>
  build() {
    auto dag = concrete_optimizer::dag::empty();
    // Converting arguments as Input
    mlir::Builder builder(func.getContext());
    for (size_t i = 0; i < func.getNumArguments(); i++) {
      auto arg = func.getArgument(i);
      auto optimizerIdx = addArg(dag, arg);
      if (optimizerIdx.has_value() && setOptimizerID) {
        func.setArgAttr(i, "TFHE.OId",
                        builder.getI32IntegerAttr(optimizerIdx->index));
      }
    }
    // Converting ops
    for (auto &bb : func.getBody().getBlocks()) {
      for (auto &op : bb.getOperations()) {
        addOperation(dag, op);
      }
    }
    for (auto &bb : func.getBody().getBlocks()) {
      for (auto &op : bb.getOperations()) {
        op.removeAttr("SMANP");
      }
    }
    if (index.empty()) {
      // Dag is empty <=> classical function without encryption
      DEBUG("!!! concrete-optimizer: nothing to do in " << func.getName()
                                                        << "\n");
      return std::nullopt;
    };
    DEBUG(std::string(dag->dump()));
    return std::move(dag);
  }

  std::optional<concrete_optimizer::dag::OperatorIndex>
  addArg(optimizer::Dag &dag, mlir::Value &arg) {
    DEBUG("Arg " << arg << " " << arg.getType());
    if (!fhe::utils::isEncryptedValue(arg)) {
      return std::nullopt;
    }
    auto precision = fhe::utils::getEintPrecision(arg);
    auto shape = getShape(arg);
    auto opI = dag->add_input(precision, slice(shape));
    index[arg] = opI;
    return opI;
  }

  bool hasEncryptedResult(mlir::Operation &op) {
    for (auto val : op.getResults()) {
      if (fhe::utils::isEncryptedValue(val)) {
        return true;
      }
    }
    return false;
  }

  void addOperation(optimizer::Dag &dag, mlir::Operation &op) {
    DEBUG("Instr " << op);

    if (isReturn(op)) {
      // This op has no result
      return;
    }

    auto encrypted_inputs = encryptedInputs(op);
    if (!hasEncryptedResult(op)) {
      // This op is unrelated to FHE
      assert(encrypted_inputs.empty() ||
             mlir::isa<mlir::concretelang::Tracing::TraceCiphertextOp>(op));
      return;
    }
    assert(op.getNumResults() == 1);
    auto val = op.getResult(0);
    auto precision = fhe::utils::getEintPrecision(val);
    concrete_optimizer::dag::OperatorIndex index;
    if (auto inputType = isLut(op); inputType != nullptr) {
      addLut(dag, op, inputType, encrypted_inputs, precision);
      return;
    } else if (isRound(op)) {
      index = addRound(dag, val, encrypted_inputs, precision);
    } else if (auto dot = asDot(op)) {
      auto weightsOpt = dotWeights(dot);
      if (weightsOpt) {
        index = addDot(dag, val, encrypted_inputs, weightsOpt.value());
      } else {
        // If can't find weights return default leveled op
        DEBUG("Replace Dot by LevelledOp on " << op);
        index = addLevelledOp(dag, op, encrypted_inputs);
      }
    } else if (auto mul = asMul(op)) {
      // special case as mul are rewritten in several optimizer nodes
      addMul(dag, mul, encrypted_inputs, precision);
      return;
    } else if (auto mul = asMulTensor(op)) {
      // special case as mul are rewritten in several optimizer nodes
      addMul(dag, mul, encrypted_inputs, precision);
      return;
    } else if (auto max = asMax(op)) {
      // special case as max are rewritten in several optimizer nodes
      addMax(dag, max, encrypted_inputs, precision);
      return;
    } else if (auto maxpool2d = asMaxpool2d(op)) {
      // special case as max are rewritten in several optimizer nodes
      addMaxpool2d(dag, maxpool2d, encrypted_inputs, precision);
      return;
    } else {
      index = addLevelledOp(dag, op, encrypted_inputs);
    }
    mlir::Builder builder(op.getContext());
    if (setOptimizerID)
      op.setAttr("TFHE.OId", builder.getI32IntegerAttr(index.index));
  }

  void addLut(optimizer::Dag &dag, mlir::Operation &op,
              FHE::FheIntegerInterface inputType, Inputs &encrypted_inputs,
              int precision) {
    auto val = op.getResult(0);
    assert(encrypted_inputs.size() == 1);
    // No need to distinguish different lut kind until we do approximate
    // paradigm on outputs
    auto encrypted_input = encrypted_inputs[0];
    std::vector<std::uint64_t> unknowFunction;
    std::vector<int32_t> operatorIndexes;
    if (inputType.isSigned()) {
      // std::vector<std::int64_t> weights_vector{1};
      auto addIndex = dag->add_dot(slice(encrypted_inputs),
                                   concrete_optimizer::weights::vector(
                                       slice(std::vector<std::int64_t>{1})));
      encrypted_inputs[0] = addIndex;
      operatorIndexes.push_back(addIndex.index);
    }
    auto lutIndex =
        dag->add_lut(encrypted_input, slice(unknowFunction), precision);
    operatorIndexes.push_back(lutIndex.index);
    mlir::Builder builder(op.getContext());
    if (setOptimizerID)
      op.setAttr("TFHE.OId", builder.getDenseI32ArrayAttr(operatorIndexes));
    index[val] = lutIndex;
  }

  concrete_optimizer::dag::OperatorIndex addRound(optimizer::Dag &dag,
                                                  mlir::Value &val,
                                                  Inputs &encrypted_inputs,
                                                  int rounded_precision) {
    assert(encrypted_inputs.size() == 1);
    // No need to distinguish different lut kind until we do approximate
    // paradigm on outputs
    auto encrypted_input = encrypted_inputs[0];
    index[val] = dag->add_round_op(encrypted_input, rounded_precision);
    return index[val];
  }

  concrete_optimizer::dag::OperatorIndex
  addDot(optimizer::Dag &dag, mlir::Value &val, Inputs &encrypted_inputs,
         std::vector<std::int64_t> &weights_vector) {
    assert(encrypted_inputs.size() == 1);
    auto weights = concrete_optimizer::weights::vector(slice(weights_vector));
    index[val] = dag->add_dot(slice(encrypted_inputs), std::move(weights));
    return index[val];
  }

  std::string loc_to_string(mlir::Location location) {
    std::string loc;
    llvm::raw_string_ostream loc_stream(loc);
    location.print(loc_stream);
    return loc;
  }

  concrete_optimizer::dag::OperatorIndex
  addLevelledOp(optimizer::Dag &dag, mlir::Operation &op, Inputs &inputs) {
    auto val = op.getResult(0);
    auto out_shape = getShape(val);
    if (inputs.empty()) {
      // Trivial encrypted constants encoding
      // There are converted to input + levelledop
      auto precision = fhe::utils::getEintPrecision(val);
      auto opI = dag->add_input(precision, slice(out_shape));
      inputs.push_back(opI);
    }
    // Default complexity is negligible
    double fixed_cost = NEGLIGIBLE_COMPLEXITY;
    double lwe_dim_cost_factor = NEGLIGIBLE_COMPLEXITY;
    auto smanp_int = op.getAttrOfType<mlir::IntegerAttr>("SMANP");
    auto loc = loc_to_string(op.getLoc());
    assert(smanp_int && "Missing manp value on a crypto operation");
    // TODO: use APIFloat.sqrt when it's available
    double manp = sqrt(smanp_int.getValue().roundToDouble());
    auto comment = std::string(op.getName().getStringRef()) + " " + loc;
    index[val] =
        dag->add_levelled_op(slice(inputs), lwe_dim_cost_factor, fixed_cost,
                             manp, slice(out_shape), comment);
    return index[val];
  }

  bool isSignedEint(mlir::Type type) {
    if (auto tensor = type.dyn_cast<RankedTensorType>(); tensor != nullptr) {
      type = tensor.getElementType();
    }
    return type.cast<FHE::FheIntegerInterface>().isSigned();
  }

  template <typename MulOp>
  void addMul(optimizer::Dag &dag, MulOp &mulOp, Inputs &inputs,
              int precision) {

    // x * y = ((x + y)^2 / 4) - ((x - y)^2 / 4) == tlu(x + y) - tlu(x - y)

    mlir::Value result = mulOp.getResult();
    const std::vector<uint64_t> resultShape = getShape(result);

    Operation *xOp = mulOp.getLhs().getDefiningOp();
    Operation *yOp = mulOp.getRhs().getDefiningOp();

    const double fixedCost = NEGLIGIBLE_COMPLEXITY;
    const double lweDimCostFactor = NEGLIGIBLE_COMPLEXITY;

    llvm::APInt xSmanp = llvm::APInt{1, 1, false};
    if (xOp != nullptr) {
      const auto xSmanpAttr = xOp->getAttrOfType<mlir::IntegerAttr>("SMANP");
      assert(xSmanpAttr && "Missing SMANP value on a crypto operation");
      xSmanp = xSmanpAttr.getValue();
    }

    llvm::APInt ySmanp = llvm::APInt{1, 1, false};
    if (yOp != nullptr) {
      const auto ySmanpAttr = yOp->getAttrOfType<mlir::IntegerAttr>("SMANP");
      assert(ySmanpAttr && "Missing SMANP value on a crypto operation");
      ySmanp = ySmanpAttr.getValue();
    }

    auto loc = loc_to_string(mulOp.getLoc());
    auto comment = std::string(mulOp->getName().getStringRef()) + " " + loc;

    // (x + y) and (x - y)
    const double addSubManp =
        sqrt(xSmanp.roundToDouble() + ySmanp.roundToDouble());

    // tlu(v)
    const double tluManp = 1;

    // tlu(v1) - tlu(v2)
    const double tluSubManp = sqrt(tluManp + tluManp);

    // for tlus
    const std::vector<std::uint64_t> unknownFunction;

    // tlu(x + y)
    auto addNode =
        dag->add_levelled_op(slice(inputs), lweDimCostFactor, fixedCost,
                             addSubManp, slice(resultShape), comment);
    std::optional<concrete_optimizer::dag::OperatorIndex> lhsCorrectionNode;
    if (isSignedEint(mulOp.getType())) {
      // If signed mul we need to add the addition node for correction of the
      // signed tlu
      addNode = dag->add_dot(
          slice(std::vector<concrete_optimizer::dag::OperatorIndex>{addNode}),
          concrete_optimizer::weights::vector(
              slice(std::vector<std::int64_t>{1})));
      lhsCorrectionNode = addNode;
    }
    auto lhsTluNode = dag->add_lut(addNode, slice(unknownFunction), precision);

    // tlu(x - y)
    auto subNode =
        dag->add_levelled_op(slice(inputs), lweDimCostFactor, fixedCost,
                             addSubManp, slice(resultShape), comment);
    // This is a signed tlu so we need to also add the addition for correction
    // signed tlu
    auto rhsCorrectionNode = dag->add_dot(
        slice(std::vector<concrete_optimizer::dag::OperatorIndex>{subNode}),
        concrete_optimizer::weights::vector(
            slice(std::vector<std::int64_t>{1})));
    auto rhsTluNode =
        dag->add_lut(rhsCorrectionNode, slice(unknownFunction), precision);

    // tlu(x + y) - tlu(x - y)
    const std::vector<concrete_optimizer::dag::OperatorIndex> subInputs = {
        lhsTluNode, rhsTluNode};
    auto resultNode =
        dag->add_levelled_op(slice(subInputs), lweDimCostFactor, fixedCost,
                             tluSubManp, slice(resultShape), comment);
    index[result] = resultNode;
    // Set attribute on the MLIR node
    mlir::Builder builder(mulOp.getContext());
    mlir::SmallVector<int32_t, 5> operatorIndexes = {
        (int32_t)addNode.index,    (int32_t)lhsTluNode.index,
        (int32_t)subNode.index,    (int32_t)rhsCorrectionNode.index,
        (int32_t)rhsTluNode.index, (int32_t)resultNode.index};
    if (lhsCorrectionNode.has_value()) {
      // We push that at the end by convention
      operatorIndexes.push_back(lhsCorrectionNode.value().index);
    }
    if (setOptimizerID)
      mulOp->setAttr("TFHE.OId", builder.getDenseI32ArrayAttr(operatorIndexes));
  }

  void addMax(optimizer::Dag &dag, FHE::MaxEintOp &maxOp, Inputs &inputs,
              int precision) {
    mlir::Value result = maxOp.getResult();
    const std::vector<uint64_t> resultShape = getShape(result);

    Operation *xOp = maxOp.getX().getDefiningOp();
    Operation *yOp = maxOp.getY().getDefiningOp();

    const double fixedCost = NEGLIGIBLE_COMPLEXITY;
    const double lweDimCostFactor = NEGLIGIBLE_COMPLEXITY;

    llvm::APInt xSmanp = llvm::APInt{1, 1, false};
    if (xOp != nullptr) {
      const auto xSmanpAttr = xOp->getAttrOfType<mlir::IntegerAttr>("SMANP");
      assert(xSmanpAttr && "Missing SMANP value on a crypto operation");
      xSmanp = xSmanpAttr.getValue();
    }

    llvm::APInt ySmanp = llvm::APInt{1, 1, false};
    if (yOp != nullptr) {
      const auto ySmanpAttr = yOp->getAttrOfType<mlir::IntegerAttr>("SMANP");
      assert(ySmanpAttr && "Missing SMANP value on a crypto operation");
      ySmanp = ySmanpAttr.getValue();
    }

    const double subManp =
        sqrt(xSmanp.roundToDouble() + ySmanp.roundToDouble());

    auto loc = loc_to_string(maxOp.getLoc());
    auto comment = std::string(maxOp->getName().getStringRef()) + " " + loc;

    auto subNode =
        dag->add_levelled_op(slice(inputs), lweDimCostFactor, fixedCost,
                             subManp, slice(resultShape), comment);

    const double tluNodeManp = 1;
    const std::vector<std::uint64_t> unknownFunction;
    auto tluNode = dag->add_lut(subNode, slice(unknownFunction), precision);

    const double addManp = sqrt(tluNodeManp + ySmanp.roundToDouble());
    const std::vector<concrete_optimizer::dag::OperatorIndex> addInputs = {
        tluNode, inputs[1]};
    auto resultNode =
        dag->add_levelled_op(slice(addInputs), lweDimCostFactor, fixedCost,
                             addManp, slice(resultShape), comment);
    index[result] = resultNode;

    // Set attribute on the MLIR node
    mlir::Builder builder(maxOp.getContext());
    mlir::SmallVector<int32_t, 3> operatorIndexes = {(int32_t)subNode.index,
                                                     (int32_t)tluNode.index,
                                                     (int32_t)resultNode.index};
    if (setOptimizerID)
      maxOp->setAttr("TFHE.OId", builder.getDenseI32ArrayAttr(operatorIndexes));
  }

  void addMaxpool2d(optimizer::Dag &dag, FHELinalg::Maxpool2dOp &maxpool2dOp,
                    Inputs &inputs, int precision) {
    mlir::Value result = maxpool2dOp.getResult();
    const std::vector<uint64_t> resultShape = getShape(result);

    // all TLUs are flattened into a dimension
    // to create a single TLU node in optimizer dag
    std::vector<uint64_t> fakeShape = resultShape;

    uint64_t numberOfComparisons = 1;
    for (auto dimensionSize :
         maxpool2dOp.getKernelShape().getValues<int64_t>()) {
      numberOfComparisons *= dimensionSize;
    }
    fakeShape.push_back(numberOfComparisons);

    Operation *inputOp = maxpool2dOp.getInput().getDefiningOp();

    const double fixedCost = NEGLIGIBLE_COMPLEXITY;
    const double lweDimCostFactor = NEGLIGIBLE_COMPLEXITY;

    llvm::APInt inputSmanp = llvm::APInt{1, 1, false};
    if (inputOp != nullptr) {
      const auto inputSmanpAttr =
          inputOp->getAttrOfType<mlir::IntegerAttr>("SMANP");
      assert(inputSmanpAttr && "Missing SMANP value on a crypto operation");
      inputSmanp = inputSmanpAttr.getValue();
    }

    const double subManp = sqrt(2 * inputSmanp.roundToDouble() + 1);

    auto loc = loc_to_string(maxpool2dOp.getLoc());
    auto comment =
        std::string(maxpool2dOp->getName().getStringRef()) + " " + loc;

    auto subNode =
        dag->add_levelled_op(slice(inputs), lweDimCostFactor, fixedCost,
                             subManp, slice(fakeShape), comment);

    const std::vector<std::uint64_t> unknownFunction;
    auto tluNode = dag->add_lut(subNode, slice(unknownFunction), precision);

    const double addManp = sqrt(inputSmanp.roundToDouble() + 1);
    const std::vector<concrete_optimizer::dag::OperatorIndex> addInputs = {
        tluNode, inputs[0]};

    auto resultNode =
        dag->add_levelled_op(slice(addInputs), lweDimCostFactor, fixedCost,
                             addManp, slice(resultShape), comment);
    index[result] = resultNode;
    // Set attribute on the MLIR node
    mlir::Builder builder(maxpool2dOp.getContext());
    mlir::SmallVector<int32_t, 3> operatorIndexes = {(int32_t)subNode.index,
                                                     (int32_t)tluNode.index,
                                                     (int32_t)resultNode.index};
    // TODO : The substraction of the signed case is not given to the optimizer
    // which could lead to some issue with the dag partitioning of the
    // optimizer.
    // Note: Should not be an issue while the partition are computed
    // on the precision.
    if (setOptimizerID)
      maxpool2dOp->setAttr("TFHE.OId",
                           builder.getDenseI32ArrayAttr(operatorIndexes));
  }

  Inputs encryptedInputs(mlir::Operation &op) {
    Inputs inputs;
    for (auto operand : op.getOperands()) {
      auto entry = index.find(operand);
      if (entry == index.end()) {
        assert(!fhe::utils::isEncryptedValue(operand));
        DEBUG("Ignoring as input " << operand);
        continue;
      }
      inputs.push_back(entry->getSecond());
    }
    return inputs;
  }

  template <typename LinalgApplyLookupTable>
  FHE::FheIntegerInterface getEintTypeOfLut(LinalgApplyLookupTable op) {
    auto tensorType = op.getT().getType().template dyn_cast<mlir::TensorType>();
    auto eint = tensorType.getElementType()
                    .template dyn_cast<FHE::FheIntegerInterface>();
    assert(eint != nullptr);
    return eint;
  }

  // Returns the FHE integer type on which the lut is performed else return a
  // nullptr
  FHE::FheIntegerInterface isLut(mlir::Operation &op) {
    if (auto lut =
            llvm::dyn_cast<mlir::concretelang::FHE::ApplyLookupTableEintOp>(op);
        lut != nullptr) {
      auto eint = lut.getA().getType().dyn_cast<FHE::FheIntegerInterface>();
      assert(eint != nullptr);
      return eint;
    }
    if (auto lut = llvm::dyn_cast<
            mlir::concretelang::FHELinalg::ApplyLookupTableEintOp>(op);
        lut != nullptr) {
      return getEintTypeOfLut(lut);
    }
    if (auto lut = llvm::dyn_cast<
            mlir::concretelang::FHELinalg::ApplyMultiLookupTableEintOp>(op);
        lut != nullptr) {
      return getEintTypeOfLut(lut);
    }
    if (auto lut = llvm::dyn_cast<
            mlir::concretelang::FHELinalg::ApplyMappedLookupTableEintOp>(op);
        lut != nullptr) {
      return getEintTypeOfLut(lut);
    }
    return nullptr;
  }

  bool isRound(mlir::Operation &op) {
    return llvm::isa<mlir::concretelang::FHE::RoundEintOp>(op) ||
           llvm::isa<mlir::concretelang::FHELinalg::RoundOp>(op);
  }

  mlir::concretelang::FHELinalg::Dot asDot(mlir::Operation &op) {
    return llvm::dyn_cast<mlir::concretelang::FHELinalg::Dot>(op);
  }

  mlir::concretelang::FHE::MulEintOp asMul(mlir::Operation &op) {
    return llvm::dyn_cast<mlir::concretelang::FHE::MulEintOp>(op);
  }

  mlir::concretelang::FHELinalg::MulEintOp asMulTensor(mlir::Operation &op) {
    return llvm::dyn_cast<mlir::concretelang::FHELinalg::MulEintOp>(op);
  }

  mlir::concretelang::FHE::MaxEintOp asMax(mlir::Operation &op) {
    return llvm::dyn_cast<mlir::concretelang::FHE::MaxEintOp>(op);
  }

  mlir::concretelang::FHELinalg::Maxpool2dOp asMaxpool2d(mlir::Operation &op) {
    return llvm::dyn_cast<mlir::concretelang::FHELinalg::Maxpool2dOp>(op);
  }

  bool isReturn(mlir::Operation &op) {
    return llvm::isa<mlir::func::ReturnOp>(op);
  }

  bool isConst(mlir::Operation &op) {
    return llvm::isa<mlir::arith::ConstantOp>(op);
  }

  bool isArg(const mlir::Value &value) {
    return value.isa<mlir::BlockArgument>();
  }

  std::optional<std::vector<std::int64_t>>
  resolveConstantVectorWeights(mlir::arith::ConstantOp &cstOp) {
    std::vector<std::int64_t> values;
    mlir::DenseIntElementsAttr denseVals =
        cstOp->getAttrOfType<mlir::DenseIntElementsAttr>("value");

    for (llvm::APInt val : denseVals.getValues<llvm::APInt>()) {
      if (val.getActiveBits() > 64) {
        return std::nullopt;
      }
      values.push_back(val.getSExtValue());
    }
    return values;
  }

  std::optional<std::vector<std::int64_t>>
  resolveConstantWeights(mlir::Value &value) {
    if (auto cstOp = llvm::dyn_cast_or_null<mlir::arith::ConstantOp>(
            value.getDefiningOp())) {
      auto shape = getShape(value);
      switch (shape.size()) {
      case 1:
        return resolveConstantVectorWeights(cstOp);
      default:
        DEBUG("High-Rank tensor: rely on MANP and levelledOp");
        return std::nullopt;
      }
    } else {
      DEBUG("Dynamic Weights: rely on MANP and levelledOp");
      return std::nullopt;
    }
  }

  std::optional<std::vector<std::int64_t>>
  dotWeights(mlir::concretelang::FHELinalg::Dot &dot) {
    if (dot.getOperands().size() != 2) {
      return std::nullopt;
    }
    auto weights = dot.getOperands()[1];
    return resolveConstantWeights(weights);
  }

  std::vector<std::uint64_t> getShape(mlir::Value &value) {
    return getShape(value.getType());
  }

  std::vector<std::uint64_t> getShape(mlir::Type type_) {
    if (auto ranked_tensor = type_.dyn_cast_or_null<mlir::RankedTensorType>()) {
      std::vector<std::uint64_t> shape;
      for (auto v : ranked_tensor.getShape()) {
        shape.push_back(v);
      }
      return shape;
    } else {
      return {};
    }
  }
};

} // namespace

struct DagPass : ConcreteOptimizerBase<DagPass> {
  optimizer::Config config;
  optimizer::FunctionsDag &dags;

  void runOnOperation() override {
    mlir::func::FuncOp func = getOperation();
    auto name = std::string(func.getName());
    DEBUG("ConcreteOptimizer Dag: " << name);
    auto dag = FunctionToDag(func, config).build();
    if (dag) {
      dags.insert(
          optimizer::FunctionsDag::value_type(name, std::move(dag.value())));
    } else {
      this->signalPassFailure();
    }
  }

  DagPass() = delete;
  DagPass(optimizer::Config config, optimizer::FunctionsDag &dags)
      : config(config), dags(dags) {}
};

// Create an instance of the ConcreteOptimizerPass pass.
// A global pass result is communicated using `dags`.
// If `debug` is true, for each operation, the pass emits a
// remark containing the squared Minimal Arithmetic Noise Padding of
// the equivalent dot operation.
std::unique_ptr<mlir::Pass> createDagPass(optimizer::Config config,
                                          optimizer::FunctionsDag &dags) {
  return std::make_unique<optimizer::DagPass>(config, dags);
}

} // namespace optimizer
} // namespace concretelang
} // namespace mlir
