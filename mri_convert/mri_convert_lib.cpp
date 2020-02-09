//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"

#include <eigen3/Eigen/Dense>

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
    return fmt::fprintf(stream, usage_message()) >= 0;
  }
  return false;
}

void print_parsed_tokens(po::basic_parsed_options<char> const &parsed_opts) {
  for (auto &opt : parsed_opts.options) {
    std::string tmp{};
    for (auto &token : opt.original_tokens) {
      tmp += token;
      tmp += " ";
    }
    tmp.pop_back(); // last space
    fmt::print("{}\n", tmp);
  }
}
} // namespace fs::util::cli

namespace fs::math {

auto frobenius_norm(const std::vector<double> *matrix) -> double {
  auto size = static_cast<Eigen::VectorXd::Index>((*matrix).size());
  auto data = (*matrix).data();
  return Eigen::VectorXd::Map(data, size).norm();
}

auto frobenius_normalize(std::vector<double> *matrix) -> void {
  auto size = static_cast<Eigen::VectorXd::Index>((*matrix).size());
  auto data = (*matrix).data();
  Eigen::VectorXd::Map(data, size).normalize();
}
} // namespace fs::math

namespace fs::dbg {
void create_gdb_file(gsl::multi_span<char *> args) {
  std::ofstream fptmp("debug.gdb");
  fmt::fprintf(fptmp, "# source this file in gdb to debug\n");
  fmt::fprintf(fptmp, "file %s\n", args[0]);
  fmt::fprintf(fptmp, "run ");
  for (int j = 1; j < args.size(); j++) {
    std::string tmp{args[j]};
    if (tmp.find("debug") != std::string::npos) {
      continue;
    }
    fmt::fprintf(fptmp, "%s ", args[j]);
  }
  fmt::fprintf(fptmp, "\n");
  fptmp.close();
}
} // namespace fs::dbg
