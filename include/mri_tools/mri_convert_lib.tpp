//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_TPP
#define MRI_TOOLS_MRI_CONVERT_LIB_TPP

#include <fmt/printf.h>

namespace fs::util::cli {
template <typename T>
auto check_vector_range(const std::vector<T> &vector, std::string opt,
                        size_t lower, size_t upper = 0) -> bool {
  upper = upper == 0 ? lower : upper;
  if (lower > upper) {
    auto message = fmt::format("lower bound '{}' must be less than upper bound "
                               "'{}' for argument \"{}\".\n",
                               lower, upper, opt);
    throw std::logic_error(message);
  }
  if (vector.size() < lower || vector.size() > upper) {
    auto message =
        fmt::format("given values for argument \"{}\" not within range: "
                    "have '{}', expected value "
                    "in range '{}...{}'.\n",
                    opt, vector.size(), lower, upper);
    throw std::logic_error(message);
  }
  return true;
}

template <typename T>
auto check_value_range(T const value, std::string opt, T lower, T upper = 0)
    -> bool {
  upper = upper == 0 ? lower : upper;
  if (lower > upper) {
    auto message = fmt::format("lower bound '{}' must be less than upper bound "
                               "'{}' for argumet \"{}\".\n",
                               lower, upper, opt);
    throw std::logic_error(message);
  }
  if (value < lower || value > upper) {
    auto message =
        fmt::format("given values for argument \"{}\" not within range: "
                    "have '{}', expected value "
                    "in range'{}...{}'.\n",
                    opt, value, lower, upper);
    throw std::logic_error(message);
  }
  return true;
}

template <typename T>
auto check_value(T const value, T comp, std::string opt,
                 std::function<bool(T, T)> fn) -> bool {
  if (!fn(value, comp)) {
    auto message = fmt::format("value '{}' disallowed for option \"{}\". try "
                               "--help for more detail\n",
                               value, opt);
    throw std::logic_error(message);
  }
  return true;
}

auto checkRange = [](auto opt, size_t min, size_t max = 0) {
  return [opt, min, max](auto v) { check_vector_range(v, opt, min, max); };
};

auto checkValue = [](auto min, auto opt,
                     std::function<bool(typeof(min), typeof(min))> fn) {
  return [min, opt, fn](auto value) { check_value(value, min, opt, fn); };
};

} // namespace fs::util::cli
#endif
