//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#include "mri_convert_lib.hpp"

namespace fs::utils::cli {

auto returnAnInt(int toReturn) -> int { return toReturn; }

auto usage_message(FILE *stream, char const *Progname) -> bool {
  if (stream == stdin) { return false;}
  if (stream != nullptr && Progname != nullptr) {
    fmt::fprintf(stream, "\ntype %s -u for usage\n\n", Progname);
    return true;
  }

  return false;
}

auto usage_message(FILE *stream, std::string const Progname) -> bool {
  return usage_message(stream, Progname.c_str());
}

auto usage_message(std::ostream &stream, char const *Progname) -> bool {

  if (stream && Progname != nullptr) {
    fmt::print(stream, "\ntype {} -u for usage\n\n", Progname);
    return true;
  }
  return false;
}

auto usage_message(std::ostream &stream, const std::string Progname) -> bool {
  return usage_message(stream, Progname.c_str());
}
} // namespace fs::utils
