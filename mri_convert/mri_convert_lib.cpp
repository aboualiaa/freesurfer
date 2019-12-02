//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"

#include <eigen3/Eigen/Dense>

#include <cstdint>
#include <iostream>

using uint64 = uint64_t;

namespace fs::util::cli {

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

auto usage_message() -> std::string {
  std::ostringstream tmp;
  tmp << "\ntype " << GET_PROGRAM_NAME() << " -u for usage\n\n";
  return tmp.str();
}

auto usage_message(FILE *stream) -> bool {
  if (stream != nullptr) {
    fmt::fprintf(stream, usage_message());
    return true;
  }

  return false;
}

} // namespace fs::util::cli

namespace fs::math {

auto frobenius_norm(const std::vector<double> *matrix) -> double {
  auto size = static_cast<Eigen::Index>((*matrix).size());
  auto data = (*matrix).data();
  return Eigen::VectorXd::Map(data, size).norm();
}

auto frobenius_normalize(std::vector<double> *matrix) -> void {
  auto size = static_cast<Eigen::Index>((*matrix).size());
  auto data = (*matrix).data();
  Eigen::VectorXd::Map(data, size).normalize();
}
} // namespace fs::math
