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
    fmt::fprintf(
        stderr, "ERROR: lower bound (%d) must be less than upper bound (%d).\n",
        lower, upper);
    exit(1);
  } else if (vector.size() < lower || vector.size() > upper) {
    return false;
  }
  return true;
}
} // namespace fs::util::cli
#endif
