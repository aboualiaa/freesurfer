//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_TPP
#define MRI_TOOLS_MRI_CONVERT_LIB_TPP

#include <fmt/printf.h>

namespace fs::util::cli {
template <typename T>
auto check_vector_range(const std::vector<T> &vector, size_t lower,
                        size_t upper = 0) -> bool {
  upper = upper == 0 ? lower : upper;
  if (lower > upper) {
    auto message = fmt::sprintf(
        "lower bound (%d) must be less than upper bound (%d).\n", lower, upper);
    throw std::logic_error(message);
  }
  if (vector.size() < lower || vector.size() > upper) {
    auto message =
        fmt::sprintf("given values not within range: have (%d), expected value "
                     "between (%d) and (%d).\n",
                     vector.size(), lower, upper);
    throw std::logic_error(message);
  }
  return true;
}

auto checkRange = [](size_t min, size_t max = 0) {
  return [min, max](auto v) { check_vector_range(v, min, max); };
};

} // namespace fs::util::cli
#endif
