//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_HPP
#define MRI_TOOLS_MRI_CONVERT_LIB_HPP

#include <boost/program_options.hpp>
#include <gsl/multi_span>

#undef GET_PROGRAM_NAME
#ifdef __GLIBC__
#define GET_PROGRAM_NAME() program_invocation_short_name
#else /* BSD and OS X */
#include <cstdlib>
#define GET_PROGRAM_NAME() getprogname()
#endif

namespace fs::util::cli {

namespace pocl = boost::program_options::command_line_style;
namespace po = boost::program_options;

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
void create_gdb_file(gsl::multi_span<char *> args);
} // namespace fs::dbg

#include "mri_convert_lib.tpp"

#endif // MRI_TOOLS_MRI_CONVERT_LIB_HPP
