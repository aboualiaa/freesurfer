//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_LIB_HPP
#define MRI_TOOLS_MRI_CONVERT_LIB_HPP

#include <boost/program_options.hpp>
#include <fmt/printf.h>
#include <fmt/format.h>
#include <cstdio>

namespace pocl = boost::program_options::command_line_style;

namespace fs::utils::cli {

// program option style
static auto const po_style =
    static_cast<unsigned>(pocl::allow_short) | pocl::short_allow_adjacent |
    pocl::short_allow_next | pocl::allow_long | pocl::long_allow_adjacent |
    pocl::long_allow_next | pocl::allow_sticky | pocl::allow_dash_for_short |
    pocl::allow_long_disguise;

auto returnAnInt(int toReturn) -> int;
auto usage_message(FILE *stream, char const *Progname) -> bool;
auto usage_message(FILE *stream, std::string const Progname) -> bool;
auto usage_message(std::ostream &stream, char const *Progname) -> bool;
auto usage_message(std::ostream &stream, const std::string Progname) -> bool;
} // namespace fs::utils
#endif // MRI_TOOLS_MRI_CONVERT_LIB_HPP
