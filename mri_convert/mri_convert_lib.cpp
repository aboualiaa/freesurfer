//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"

#include <eigen3/Eigen/Dense>

#include <cstdint>
#include <iostream>

using uint64 = uint64_t;

namespace fs::utils::cli {

auto usage_message(FILE *stream, char const *Progname) -> bool {
  if (stream == stdin) {
    return false;
  }
  if (stream != nullptr && Progname != nullptr) {
    fmt::fprintf(stream, "\ntype %s -u for usage\n\n", Progname);
    return true;
  }

  return false;
}

auto usage_message(FILE *stream, std::string const &Progname) -> bool {
  return usage_message(stream, Progname.c_str());
}

auto usage_message(std::ostream &stream, char const *Progname) -> bool {

  if (stream && Progname != nullptr) {
    fmt::print(stream, "\ntype {} -u for usage\n\n", Progname);
    return true;
  }
  return false;
}

auto usage_message(std::ostream &stream, std::string const &Progname) -> bool {
  return usage_message(stream, Progname.c_str());
}
} // namespace fs::utils::cli

namespace fs::math {

auto frobenius_norm(const std::vector<double> *matrix) -> double {
  return Eigen::VectorXd::Map((*matrix).data(),
                              static_cast<Eigen::Index>((*matrix).size()))
      .norm();
}

auto frobenius_normalize(std::vector<double> *matrix) -> void {
  Eigen::VectorXd::Map((*matrix).data(),
                       static_cast<Eigen::Index>((*matrix).size()))
      .normalize();
}
} // namespace fs::math
