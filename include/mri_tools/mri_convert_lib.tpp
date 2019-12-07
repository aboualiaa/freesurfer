//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_TPP
#define MRI_TOOLS_MRI_CONVERT_LIB_TPP

#include <fmt/printf.h>
#include <fstream>

#include "mri.h"

namespace fs::util::cli {

namespace {
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
} // namespace

auto checkSize = [](auto opt, size_t min, size_t max = 0) {
  return [opt, min, max](auto v) { check_vector_range(v, opt, min, max); };
};

auto checkValue = [](auto min, auto opt,
                     std::function<bool(typeof(min), typeof(min))> fn) {
  return [min, opt, fn](auto value) { check_value(value, min, opt, fn); };
};

auto allocateExternalString = [](char **var) mutable {
  return [var](std::string value) mutable {
    *var = static_cast<char *>(calloc(value.length() + 1, sizeof(char)));
    memmove(*var, value.data(), value.length());
  };
};

auto addConflicts = [](std::vector<std::string> conflicts, std::string name,
                       auto *args) {
  return [name, conflicts, args](auto value) {
    for (auto &conflict : conflicts) {
      args->conflict_map.insert(std::make_pair(name, conflict));
    }
  };
};

auto addDependencies = [](std::vector<std::string> dependencies,
                          std::string name, auto *args) {
  return [name, dependencies, args](auto value) {
    for (auto &dependency : dependencies) {
      args->dependency_map.insert(std::make_pair(name, dependency));
    }
  };
};
} // namespace fs::util::cli

namespace fs::util::mri {

auto checkOrientationString = [](std::string ostr) {
  auto errmsg = MRIcheckOrientationString(ostr.data());
  if (errmsg != nullptr) {
    auto err = fmt::format("ERROR: with orientation string %s\n", ostr.data());
    err += errmsg;
    throw std::logic_error(err);
  }
};

} // namespace fs::util::mri

namespace fs::util::io {
auto checkFileReadable = [](std::string file_name) {
  std::ifstream fptmp(file_name);
  if (!fptmp.is_open()) {
    auto message =
        fmt::format("ERROR: could not open {} for reading\n", file_name);
    throw std::logic_error(message);
  }
  fptmp.close();
};

auto checkFileWriteable = [](std::string file_name) {
  std::ofstream fptmp(file_name);
  if (!fptmp.is_open()) {
    auto message =
        fmt::format("ERROR: could not open {} for writing\n", file_name);
    throw std::logic_error(message);
  }
  fptmp << "0\n";
  fptmp.close();
};
} // namespace fs::util::io
#endif
