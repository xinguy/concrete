// RUN: zamacompiler %s --run-jit --jit-args 3 2>&1| FileCheck %s

// CHECK-LABEL: 42
func @main(%arg0: !HLFHE.eint<7>) -> !HLFHE.eint<7> {
  %0 = constant 45 : i8
  %1 = "HLFHE.sub_int_eint"(%0, %arg0): (i8, !HLFHE.eint<7>) -> (!HLFHE.eint<7>)
  return %1: !HLFHE.eint<7>
}