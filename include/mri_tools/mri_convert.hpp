//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_HPP
#define MRI_TOOLS_MRI_CONVERT_HPP

#include "DICOMRead.h"
#include "cma.h"
#include "diag.h"
#include "error.h"
#include "fio.h"
#include "fmriutils.h"
#include "fmriutils.h"
#include "fsgdf.h"
#include "fsinit.h"
#include "gcamorph.h"
#include "macros.h"
#include "mri.h"
#include "mri2.h"
#include "mri_conform.h"
#include "mri_identify.h"
#include "stats.h"
#include "utils.h"
#include "version.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <fmt/printf.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

struct CMDARGS {
  CMDARGS(int argc, char *argv[]) : raw(argv, argc){}; // NOLINT

public:
  gsl::multi_span<char *> raw;
  std::vector<int> reorder_vals{};
  bool reorder_flag{};
  std::vector<int> reorder4_vals{};
  bool reorder4_flag{};
  bool debug{};
  int outside_val{};
  bool left_right_reverse{};
  bool left_right_reverse_pix{};
  bool left_right_mirror_flag{};
  std::string left_right_mirror{};
  std::string left_right_keep{};
  bool left_right_swap_label{};
  bool flip_cols{};
  bool slice_reverse{};
  bool in_stats_table{};
  bool out_stats_table{};
  float invert_contrast{};
  std::string input_volume{};
  std::string output_volume{};
  bool conform_flag{};
  int conf_keep_dc{};
  bool conform_width_256_flag{};
  bool delete_cmds{};
  std::string new_transform_fname{};
  bool DoNewTransformFname{};
  bool sphinx_flag{};
  std::string autoalign_file{};
  MATRIX *AutoAlign{};
  bool nochange{};
  bool conform_min_flag{};
  float conform_size{};
  bool parse_only_flag{};
  bool in_info_flag{};
  bool out_info_flag{};
  bool template_info_flag{};
  bool in_stats_flag{};
  bool out_stats_flag{};
  bool read_only_flag{};
  bool no_write_flag{};
  bool in_matrix_flag{};
  bool out_matrix_flag{};
  bool force_ras_good{};
  bool split_frames_flag{};
  std::string transform_fname{};
  bool transform_flag{};
  bool invert_transform_flag{};
  std::string out_like_name{};
  bool out_like_flag{};
  bool crop_flag{};
  std::vector<int> crop_center{};
  bool slice_crop_flag{};
  int slice_crop_start{};
  int slice_crop_stop{};
  std::vector<int> slice_crop{};
  std::vector<int> cropsize{};
  std::string devolvexfm{};
  int DevXFM{};
  int upsample_factor{};
  bool upsample_flag{};
  bool in_i_size_flag{};
  bool in_j_size_flag{};
  bool in_k_size_flag{};
  bool out_i_size_flag{};
  bool out_j_size_flag{};
  bool out_k_size_flag{};
  float in_i_size{};
  float in_j_size{};
  float in_k_size{};
  float out_i_size{};
  float out_j_size{};
  float out_k_size{};
  std::string colortablefile{};
};

struct ENV {
  std::string vcid =
      "$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $";
};

namespace boost::program_options {

///
/// \param vm variables map
/// \param for_what requiring option
/// \param required_option required option
inline void option_dependency(boost::program_options::variables_map const &vm,
                              std::string const &for_what,
                              std::string const &required_option) {
  if ((vm.count(for_what) != 0U) && !vm[for_what].defaulted()) {
    if (vm.count(required_option) == 0 || vm[required_option].defaulted()) {
      throw std::logic_error(std::string("Option '") + for_what +
                             "' requires option '" + required_option + "'.");
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
    throw std::logic_error(std::string("Conflicting options '") + opt1 +
                           "' and '" + opt2 + "'.");
  }
}
} // namespace boost::program_options

/// \brief does the housekeeping, use this to parse command lines,
///  do sanity and inconsistency checks, read files etc. After that
///  just start directly with your porgram logic
/// \param cmdargs struct to hold values of parsed args
/// \param env holds the vcid string
/// \return true if all logic is ok, false otherwise
static auto good_cmdline_args(CMDARGS *cmdargs, ENV *env) noexcept -> bool;

/// \brief initialize options description and save values in cmdargs
/// \param desc holds description of supported args
/// \param cmdargs holds the actual args
static void initArgDesc(boost::program_options::options_description *desc,
                        CMDARGS *cmdargs);

///
/// \param desc holds description of supported args
/// \param env holds the vcid string
inline static void
print_usage(boost::program_options::options_description const &desc, ENV *env) {
  std::cout << desc << "\n" << env->vcid << std::endl;
}

///
/// \param desc holds description of supported args
/// \param env holds the vcid string
inline static void
print_help(boost::program_options::options_description const &desc, ENV *env) {
  print_usage(desc, env);
  // TODO(aboualiaa): add tests and remove
  spdlog::get("stderr")->critical("this program is not yet tested!");
}

/* ----- determines tolerance of non-orthogonal basis vectors ----- */
constexpr auto CLOSE_ENOUGH{5e-3};

void get_ints(int argc, char *argv[], int *pos, int *vals, int nvals);
void get_floats(int argc, char *argv[], int *pos, float *vals, int nvals);
void get_string(int argc, char *argv[], int *pos, char *val);
void usage_message(FILE *stream);
void usage(FILE *stream);

static int debug{};

extern int errno;

const char *Progname;

#endif // MRI_TOOLS_MRI_CONVERT_HPP
