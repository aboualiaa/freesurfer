//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_HPP
#define MRI_TOOLS_MRI_CONVERT_HPP

#include "mri.h"
#include "mri2.h"
#include "mri_convert_lib.hpp"
#include "version.h"

#include <boost/program_options.hpp>
#include <utility>

constexpr int default_cropsize = 256;
constexpr int default_crop_center = 128;
namespace po = boost::program_options;
struct CMDARGS {
  explicit CMDARGS(std::vector<char const *> args) { raw = std::move(args); }

  void check_conflicts(po::variables_map const &vm) {
    for (auto const &[key, val] : conflict_map) {
      po::conflicting_options(vm, key, val);
    }
  }

  void check_dependencies(po::variables_map const &vm) {
    for (auto const &[key, val] : dependency_map) {
      po::dependant_options(vm, key, val);
    }
  }

public:
  std::multimap<std::string, std::string> conflict_map;
  std::multimap<std::string, std::string> dependency_map;
  MATRIX *AutoAlign{};
  double fwhm{-1.0};
  double gstd{-1.0};
  std::vector<char const *> raw;
  std::vector<int> reorder_vals{};
  std::vector<int> reorder4_vals{};
  std::string left_right_mirror_hemi{}; // which half to mirror (lh, rh)
  std::string left_right_keep_hemi{};
  std::string input_volume{};  //?
  std::string output_volume{}; //?
  std::string new_transform_fname{};
  std::string autoalign_file{};
  std::string transform_fname{};
  std::string out_like_name{};
  std::vector<int> crop_center{default_crop_center, default_crop_center,
                               default_crop_center};
  std::vector<int> slice_crop{}; //?
  std::vector<int> cropsize{default_cropsize, default_cropsize,
                            default_cropsize};
  std::string devolvexfm_subject{};
  std::string colortablefile{};
  std::string dil_seg_mask{};
  std::string in_name{};
  std::string out_name{};
  std::string statusfile{}; //?
  std::string sdcmlist{};   //?
  std::vector<int> fsubsample{};
  std::vector<int> in_center{};
  std::vector<int> delta_in_center{};
  std::vector<int> out_center{};
  std::vector<int> voxel_size{};
  std::vector<int> downsample_factor{};
  std::string subject_name{};
  std::string gdf_image_stem{};
  std::string reslice_like_name{};
  std::string in_like_name{};
  std::string color_file_name{};
  std::string in_orientation_string{};
  std::string out_orientation_string{};
  std::string out_data_type_string{};
  std::string resample_type{};
  std::vector<double> in_i_directions{};
  std::vector<double> in_j_directions{};
  std::vector<double> in_k_directions{};
  std::vector<double> out_i_directions{};
  std::vector<double> out_j_directions{};
  std::vector<double> out_k_directions{};
  std::string in_type_string{};
  std::string out_type_string{};
  std::string template_type_string{};
  std::vector<int> frames{};
  int outside_val{};
  float invert_contrast{}; //?
  int conf_keep_dc{};
  float conform_size{1.0};
  int slice_crop_start{};
  int slice_crop_stop{};
  int DevXFM{};
  int upsample_factor{};
  float in_i_size{};
  float in_j_size{};
  float in_k_size{};
  float out_i_size{};
  float out_j_size{};
  float out_k_size{};
  int nthframe{-1};
  int n_erode_seg{};
  int n_dil_seg{};
  int ncutends{};
  int out_n_i{};
  int in_n_i{};
  int in_n_j{};
  int in_n_k{};
  float in_tr{};
  float in_ti{};
  float in_te{};
  float in_flip_angle{};
  int out_n_j{};
  int out_n_k{};
  int nskip{};            // number of frames to skip from start
  int ndrop{};            // number of frames to skip from end
  int nslices_override{}; //?
  int ncols_override{};   //?
  int nrows_override{};   //?
  int SubSampStart{};
  int SubSampDelta{};
  int SubSampEnd{};
  int reduce{};
  float rescale_factor{1};
  float scale_factor{1};
  float out_scale_factor{1};
  float SliceBiasAlpha{1.0};
  int out_data_type{};
  int resample_type_val{SAMPLE_TRILINEAR};
  int forced_out_type{MRI_VOLUME_TYPE_UNKNOWN};
  int forced_in_type{MRI_VOLUME_TYPE_UNKNOWN};
  int forced_template_type{MRI_VOLUME_TYPE_UNKNOWN};
  int smooth_parcellation_count{};
  int ascii_flag{};
  bool debug{};
  bool conform_width_256_flag{};
  bool left_right_reverse{}; // where does this come from
  bool left_right_reverse_pix{};
  bool left_right_mirror_flag{}; // mirror half of the image
  bool left_right_keep_flag{};   // keep half of the image
  bool left_right_swap_label{};
  bool flip_cols{};
  bool slice_reverse{};
  bool in_stats_table_flag{};
  bool out_stats_table_flag{};
  bool conform_flag{};
  bool delete_cmds{};
  bool DoNewTransformFname{};
  bool sphinx_flag{};
  bool nochange_flag{};
  bool conform_min_flag{}; // conform to the smallest dimension
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
  bool transform_flag{};
  bool invert_transform_flag{};
  bool out_like_flag{};
  bool crop_flag{};
  bool slice_crop_flag{};
  bool upsample_flag{};
  bool in_i_size_flag{};
  bool in_j_size_flag{};
  bool in_k_size_flag{};
  bool out_i_size_flag{};
  bool out_j_size_flag{};
  bool out_k_size_flag{};
  bool translate_labels_flag{};
  bool zero_outlines_flag{};
  bool fill_parcellation_flag{};
  bool roi_flag{};
  bool erode_seg_flag{};
  bool dil_seg_flag{};
  bool cutends_flag{};
  bool out_n_i_flag{};
  bool out_n_j_flag{};
  bool out_n_k_flag{};
  bool in_n_i_flag{};
  bool in_n_j_flag{};
  bool in_n_k_flag{};
  bool in_tr_flag{};
  bool in_ti_flag{};
  bool in_te_flag{};
  bool in_flip_angle_flag{};
  bool downsample2_flag{};
  bool zero_ge_z_offset_flag{}; // E/
  bool no_zero_ge_z_offset_flag{};
  bool subsample_flag{};
  bool frame_flag{};
  bool mid_frame_flag{};
  bool reslice_like_flag{};
  bool SliceBias{};
  bool in_like_flag{};
  bool color_file_flag{};
  bool no_scale_flag{};
  bool in_i_direction_flag{};
  bool in_j_direction_flag{};
  bool out_j_direction_flag{};
  bool out_i_direction_flag{};
  bool out_k_direction_flag{};
  bool in_k_direction_flag{};
  bool force_in_type_flag{};
  bool force_out_type_flag{};
  bool force_template_type_flag{};
};

struct ENV {
  std::string vcid = "mri_convert";
};

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
                        CMDARGS *cmdargs, ENV *env);

///
/// \param desc holds description of supported args
/// \param env holds the vcid string
inline static void
print_usage(boost::program_options::options_description const &desc, ENV *env) {
  std::cout << desc << std::endl;
}

///
/// \param desc holds description of supported args
/// \param env holds the vcid string
inline static void
print_help(boost::program_options::options_description const &desc, ENV *env) {
  print_usage(desc, env);
}

/* ----- determines tolerance of non-orthogonal basis vectors ----- */
constexpr auto CLOSE_ENOUGH{5e-3};

void usage(FILE *stream);

extern int errno;

const char *Progname;

#endif // MRI_TOOLS_MRI_CONVERT_HPP
