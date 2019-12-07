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

template <typename T>
auto check_value_range(T const value, T lower, T upper = 0) -> bool {
  upper = upper == 0 ? lower : upper;
  if (lower > upper) {
    auto message = fmt::format(
        "lower bound {} must be less than upper bound {}.\n", lower, upper);
    throw std::logic_error(message);
  }
  if (value < lower || value > upper) {
    auto message =
        fmt::format("given values not within range: have {}, expected value "
                    "between {} and {}.\n",
                    value, lower, upper);
    throw std::logic_error(message);
  }
  return true;
}

template <typename T>
auto check_value(T const value, T comp, std::string opt,
                 std::function<bool(bool, bool)> fn) -> bool {
  if (!fn(value, comp)) {
    auto message = fmt::format(
        "value {} disallowed for option {}. try --help for more detail\n",
        value, opt);
    throw std::logic_error(message);
  }
  return true;
}

auto checkRange = [](size_t min, size_t max = 0) {
  return [min, max](auto v) { check_vector_range(v, min, max); };
};

auto checkValue = [](auto min, auto opt, std::function<bool(bool, bool)> fn) {
  return [min, opt, fn](auto value) { check_value(value, min, opt, fn); };
};

} // namespace fs::util::cli
#endif
