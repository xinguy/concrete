#include <cstdint>
#include <type_traits>

namespace concrete_optimizer {
  struct Solution;
}

namespace concrete_optimizer {
#ifndef CXXBRIDGE1_STRUCT_concrete_optimizer$Solution
#define CXXBRIDGE1_STRUCT_concrete_optimizer$Solution
struct Solution final {
  ::std::uint64_t input_lwe_dimension;
  ::std::uint64_t internal_ks_output_lwe_dimension;
  ::std::uint64_t ks_decomposition_level_count;
  ::std::uint64_t ks_decomposition_base_log;
  ::std::uint64_t glwe_polynomial_size;
  ::std::uint64_t glwe_dimension;
  ::std::uint64_t br_decomposition_level_count;
  ::std::uint64_t br_decomposition_base_log;
  double complexity;
  double noise_max;
  double p_error;

  using IsRelocatable = ::std::true_type;
};
#endif // CXXBRIDGE1_STRUCT_concrete_optimizer$Solution

extern "C" {
::concrete_optimizer::Solution concrete_optimizer$cxxbridge1$optimise_bootstrap(::std::uint64_t precision, ::std::uint64_t security_level, double noise_factor, double maximum_acceptable_error_probability) noexcept;
} // extern "C"

::concrete_optimizer::Solution optimise_bootstrap(::std::uint64_t precision, ::std::uint64_t security_level, double noise_factor, double maximum_acceptable_error_probability) noexcept {
  return concrete_optimizer$cxxbridge1$optimise_bootstrap(precision, security_level, noise_factor, maximum_acceptable_error_probability);
}
} // namespace concrete_optimizer
