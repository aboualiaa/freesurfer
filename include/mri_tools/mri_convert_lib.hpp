//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_HPP
#define MRI_TOOLS_MRI_CONVERT_LIB_HPP

#include <boost/program_options.hpp>
#include <fmt/format.h>

#undef GET_PROGRAM_NAME
#ifdef __GLIBC__
#define GET_PROGRAM_NAME() program_invocation_short_name
#else /* BSD and OS X */
#include <cstdlib>
#define GET_PROGRAM_NAME() getprogname()
#endif

namespace fs::util::cli {

namespace pocl = boost::program_options::command_line_style;
namespace po   = boost::program_options;

// program option style
constexpr auto po_style =
    static_cast<unsigned>(pocl::allow_short) | pocl::short_allow_adjacent |
    pocl::short_allow_next | pocl::allow_long | pocl::long_allow_adjacent |
    pocl::long_allow_next | pocl::allow_sticky | pocl::allow_dash_for_short |
    pocl::allow_long_disguise;

auto usage_message(FILE *stream, char const *Progname) -> bool;
auto usage_message(FILE *stream, std::string const &Progname) -> bool;
auto usage_message(std::ostream &stream, char const *Progname) -> bool;
auto usage_message(std::ostream &stream, std::string const &Progname) -> bool;
auto usage_message() -> std::string;
auto usage_message(FILE *stream) -> bool;
void print_parsed_tokens(po::basic_parsed_options<char> const &parsed_opts);
} // namespace fs::util::cli

namespace fs::math {
auto frobenius_norm(const std::vector<double> *matrix) -> double;
auto frobenius_normalize(std::vector<double> *matrix) -> void;
} // namespace fs::math

namespace fs::dbg {
void create_gdb_file(std::vector<char const *> args);
} // namespace fs::dbg

namespace boost::program_options {

///
/// \param vm variables map
/// \param for_what requiring option
/// \param required_option required option
inline void dependant_options(boost::program_options::variables_map const &vm,
                              std::string const &for_what,
                              std::string const &required_option) {
  if ((vm.count(for_what) != 0U) && !vm[for_what].defaulted()) {
    if (vm.count(required_option) == 0 || vm[required_option].defaulted()) {
      auto msg = fmt::format("Option '{}' requires option '{}'.", for_what,
                             required_option);
      throw std::logic_error(msg);
    }
  }
}

///
/// \param vm variables map
/// \param opt1 conflicting option 1
/// \param opt2 conflicting option 2
inline void conflicting_options(boost::program_options::variables_map const &vm,
                                std::string const &opt1,
                                std::string const &opt2) {
  if ((vm.count(opt1) != 0U) && !vm[opt1].defaulted() &&
      (vm.count(opt2) != 0U) && !vm[opt2].defaulted()) {
    auto msg = fmt::format("Conflicting options '{}' and '{}'.", opt1, opt2);
    throw std::logic_error(msg);
  }
}
} // namespace boost::program_options

#include "mri_convert_lib.tpp"

#endif // MRI_TOOLS_MRI_CONVERT_LIB_HPP
