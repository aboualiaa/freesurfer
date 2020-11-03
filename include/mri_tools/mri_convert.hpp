//
// Created by Ahmed Abou-Aliaa on 29.11.19.
//

#ifndef MRI_TOOLS_MRI_CONVERT_HPP
#define MRI_TOOLS_MRI_CONVERT_HPP

#include "DICOMRead.h"
#include "cma.h"
#include "diag.h"
#include "fio.h"
#include "fmriutils.h"
#include "gcamorph.h"
#include "mri.h"
#include "mri2.h"
#include "mri2020.hpp"
#include "mri_conform.h"
#include "mri_convert_lib.hpp"
#include "mri_identify.h"
#include "mriio.hpp"
#include "version.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int default_cropsize    = 256;
constexpr int default_crop_center = 128;
namespace po                      = boost::program_options;
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
  MATRIX *                                AutoAlign{};
  double                                  fwhm{-1.0};
  double                                  gstd{-1.0};
  std::vector<char const *>               raw;
  std::vector<int>                        reorder_vals{};
  std::vector<int>                        reorder4_vals{};
  std::string         left_right_mirror_hemi{}; // which half to mirror (lh, rh)
  std::string         left_right_keep_hemi{};
  std::string         input_volume{};  //?
  std::string         output_volume{}; //?
  std::string         new_transform_fname{};
  std::string         autoalign_file{};
  std::string         transform_fname{};
  std::string         out_like_name{};
  std::vector<int>    crop_center{default_crop_center, default_crop_center,
                               default_crop_center};
  std::vector<int>    slice_crop{}; //?
  std::vector<int>    cropsize{default_cropsize, default_cropsize,
                            default_cropsize};
  std::string         devolvexfm_subject{};
  std::string         colortablefile{};
  std::string         dil_seg_mask{};
  std::string         in_name{};
  std::string         out_name{};
  std::string         statusfile{}; //?
  std::string         sdcmlist{};   //?
  std::vector<int>    fsubsample{};
  std::vector<int>    in_center{};
  std::vector<int>    delta_in_center{};
  std::vector<int>    out_center{};
  std::vector<int>    voxel_size{};
  std::vector<int>    downsample_factor{};
  std::string         subject_name{};
  std::string         gdf_image_stem{};
  std::string         reslice_like_name{};
  std::string         in_like_name{};
  std::string         color_file_name{};
  std::string         in_orientation_string{};
  std::string         out_orientation_string{};
  std::string         out_data_type_string{};
  std::string         resample_type{};
  std::vector<double> in_i_directions{};
  std::vector<double> in_j_directions{};
  std::vector<double> in_k_directions{};
  std::vector<double> out_i_directions{};
  std::vector<double> out_j_directions{};
  std::vector<double> out_k_directions{};
  std::string         in_type_string{};
  std::string         out_type_string{};
  std::string         template_type_string{};
  std::vector<int>    frames{};
  int                 outside_val{};
  float               invert_contrast{}; //?
  int                 conf_keep_dc{};
  float               conform_size{1.0};
  int                 slice_crop_start{};
  int                 slice_crop_stop{};
  int                 DevXFM{};
  int                 upsample_factor{};
  float               in_i_size{};
  float               in_j_size{};
  float               in_k_size{};
  float               out_i_size{};
  float               out_j_size{};
  float               out_k_size{};
  int                 nthframe{-1};
  int                 n_erode_seg{};
  int                 n_dil_seg{};
  int                 ncutends{};
  int                 out_n_i{};
  int                 in_n_i{};
  int                 in_n_j{};
  int                 in_n_k{};
  float               in_tr{};
  float               in_ti{};
  float               in_te{};
  float               in_flip_angle{};
  int                 out_n_j{};
  int                 out_n_k{};
  int                 nskip{};            // number of frames to skip from start
  int                 ndrop{};            // number of frames to skip from end
  int                 nslices_override{}; //?
  int                 ncols_override{};   //?
  int                 nrows_override{};   //?
  int                 SubSampStart{};
  int                 SubSampDelta{};
  int                 SubSampEnd{};
  int                 reduce{};
  float               rescale_factor{1};
  float               scale_factor{1};
  float               out_scale_factor{1};
  float               SliceBiasAlpha{1.0};
  int                 out_data_type{-1};
  int                 resample_type_val{SAMPLE_TRILINEAR};
  int                 forced_out_type{MRI_VOLUME_TYPE_UNKNOWN};
  int                 forced_in_type{MRI_VOLUME_TYPE_UNKNOWN};
  int                 forced_template_type{MRI_VOLUME_TYPE_UNKNOWN};
  int                 smooth_parcellation_count{};
  int                 ascii_flag{};
  bool                debug{};
  bool                conform_width_256_flag{};
  bool                left_right_reverse{}; // where does this come from
  bool                left_right_reverse_pix{};
  bool                left_right_mirror_flag{}; // mirror half of the image
  bool                left_right_keep_flag{};   // keep half of the image
  bool                left_right_swap_label{};
  bool                flip_cols{};
  bool                slice_reverse{};
  bool                in_stats_table_flag{};
  bool                out_stats_table_flag{};
  bool                conform_flag{};
  bool                delete_cmds{};
  bool                DoNewTransformFname{};
  bool                sphinx_flag{};
  bool                nochange_flag{};
  bool                conform_min_flag{}; // conform to the smallest dimension
  bool                parse_only_flag{};
  bool                in_info_flag{};
  bool                out_info_flag{};
  bool                template_info_flag{};
  bool                in_stats_flag{};
  bool                out_stats_flag{};
  bool                read_only_flag{};
  bool                no_write_flag{};
  bool                in_matrix_flag{};
  bool                out_matrix_flag{};
  bool                force_ras_good{};
  bool                split_frames_flag{};
  bool                transform_flag{};
  bool                invert_transform_flag{};
  bool                out_like_flag{};
  bool                crop_flag{};
  bool                slice_crop_flag{};
  bool                upsample_flag{};
  bool                in_i_size_flag{};
  bool                in_j_size_flag{};
  bool                in_k_size_flag{};
  bool                out_i_size_flag{};
  bool                out_j_size_flag{};
  bool                out_k_size_flag{};
  bool                translate_labels_flag{};
  bool                zero_outlines_flag{};
  bool                fill_parcellation_flag{};
  bool                roi_flag{};
  bool                erode_seg_flag{};
  bool                dil_seg_flag{};
  bool                cutends_flag{};
  bool                out_n_i_flag{};
  bool                out_n_j_flag{};
  bool                out_n_k_flag{};
  bool                in_n_i_flag{};
  bool                in_n_j_flag{};
  bool                in_n_k_flag{};
  bool                in_tr_flag{};
  bool                in_ti_flag{};
  bool                in_te_flag{};
  bool                in_flip_angle_flag{};
  bool                downsample2_flag{};
  bool                zero_ge_z_offset_flag{}; // E/
  bool                no_zero_ge_z_offset_flag{};
  bool                subsample_flag{};
  bool                frame_flag{};
  bool                mid_frame_flag{};
  bool                reslice_like_flag{};
  bool                SliceBias{};
  bool                in_like_flag{};
  bool                color_file_flag{};
  bool                no_scale_flag{};
  bool                in_i_direction_flag{};
  bool                in_j_direction_flag{};
  bool                out_j_direction_flag{};
  bool                out_i_direction_flag{};
  bool                out_k_direction_flag{};
  bool                in_k_direction_flag{};
  bool                force_in_type_flag{};
  bool                force_out_type_flag{};
  bool                force_template_type_flag{};
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

#include "mri_convert.help.xml.h"

void usage(FILE * /*stream*/) {
  outputHelpXml(mri_convert_help_xml, mri_convert_help_xml_len);
} /* end usage() */

static auto good_cmdline_args(CMDARGS *cmdargs, ENV *env) noexcept -> bool {

  namespace po = boost::program_options;

  po::options_description desc(
      "\nUSAGE: mri_convert_exec [options] <in volume> <out volume>\n"
      "\n\nAvailable Options");
  po::positional_options_description pos;
  pos.add("in_name", 1).add("out_name", 1);
  po::variables_map vm;

  initArgDesc(&desc, cmdargs, env);
  auto av = cmdargs->raw.data();
  auto ac = static_cast<int>(cmdargs->raw.size());
  if (ac == 1) {
    print_usage(desc, env);
    return false;
  }

  try {
    auto parsed_opts = po::command_line_parser(ac, av)
                           .options(desc)
                           .positional(pos)
                           .style(fs::util::cli::po_style)
                           .run();

    po::store(parsed_opts, vm);
    fs::util::cli::print_parsed_tokens(parsed_opts);

    po::notify(vm);

    cmdargs->check_conflicts(vm);
    cmdargs->check_dependencies(vm);
  } catch (std::exception const &e) {
    spdlog::get("mri-convert")->critical(e.what());
    return false;
  }

  return true;
}

void initArgDesc(boost::program_options::options_description *desc,
                 CMDARGS *cmdargs, ENV *env) {

  namespace po  = boost::program_options;
  namespace cli = fs::util::cli;
  namespace io  = fs::util::io;
  namespace mri = fs::util::mri;

  desc->add_options()

      ("help,h",

       po::bool_switch() //
           ->notifier([env, desc](auto v) {
             if (v) {
               usage(nullptr);
               print_help(*desc, env);
               exit(0);
             }
           }),

       "print out information on how to use this program and exit")

      //

      ("version,v",

       po::bool_switch() //
           ->notifier([cmdargs, env](auto v) {
             if (v) {
               // handle_version_option(false, cmdargs->raw,
               // env->vcid,
               //                       "$Name:  $");
               exit(0);
             }
           }),

       "print out version and exit")

      //

      ("all-info",

       po::bool_switch() //
           ->notifier([cmdargs, env](auto v) {
             if (v) {
               // handle_version_option(true, cmdargs->raw,
               // env->vcid,
               //                       "$Name:  $");
               exit(0);
             }
           }),

       "print out all info and exit")

      //

      ("usage,u",

       po::bool_switch() //
           ->notifier([env, desc](auto v) {
             if (v) {
               usage(nullptr);
               print_help(*desc, env);
               exit(0);
             }
           }),

       "print usage and exit")

      //

      ("version2",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               exit(97);
             }
           }),

       "just exits")

      //

      ("debug",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               spdlog::set_level(
                   spdlog::level::debug); // Set global log level to debug
               fs::dbg::create_gdb_file(cmdargs->raw);
             }
           }),

       "turn on debugging")

      //

      ("reorder,r",

       po::value(&cmdargs->reorder_vals) //
           ->multitoken()
           ->notifier(cli::checkSize("reorder", 3)),

       "todo")

      //

      ("reorder4,r4",

       po::value(&cmdargs->reorder4_vals) //
           ->multitoken()
           ->notifier(cli::checkSize("reorder4", 4)),

       "Reorder axes such that olddim1 is the new column dimensions, olddim2 "
       "is the new row dimension, olddim3 is the new slice, and olddim4 is the "
       "new frame dimension. Example: 2 1 3 will swap row and colms. If using "
       "-r4, the output geometry will likely be wrong. It is best to re-run "
       "mri_convert and specify a correctly orientated volume through the "
       "--in_like option")

      //

      ("outside_val,oval",

       po::value(&cmdargs->outside_val) //
           ->notifier([cmdargs](auto /*unused*/) {
             fmt::printf("setting outside val to %d\n", cmdargs->outside_val);
           }),

       "Set the values outside of the image that may rotate in if a transform "
       "is applied to val")

      //

      ("no-dwi",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_LOAD_DWI", "0", 1);
             }
           }),

       "set FS_LOAD_DWI to 0")

      //

      ("left-right-reverse",

       po::bool_switch(&cmdargs->left_right_reverse),

       "left right reverse")

      //

      ("left-right-reverse-pix",

       po::bool_switch(&cmdargs->left_right_reverse_pix),

       "left-right-reverse-pix")

      //

      ("left-right-keep",

       po::value(&cmdargs->left_right_keep_hemi) //
           ->notifier(cli::checkString({"lh", "rh"}, "left-right-keep")),

       "left-right-keep")

      //

      ("left-right-mirror",

       po::value(&cmdargs->left_right_mirror_hemi) //
           ->notifier(cli::checkString({"lh", "rh"}, "left-right-mirror")),

       "left-right-mirror")

      //

      ("left-right-swap-label",

       po::bool_switch(&cmdargs->left_right_swap_label),

       "left-right-swap-label")

      //

      ("flip-cols",

       po::bool_switch(&cmdargs->flip_cols),

       "flip-cols")

      //

      ("slice-reverse",

       po::bool_switch(&cmdargs->slice_reverse),

       "Reverse order of slices, update vox2ras")

      //

      ("in_stats_table",

       po::bool_switch(&cmdargs->in_stats_table_flag),

       "Input data is a stats table as produced by asegstats2table or "
       "aparcstats2table")

      //

      ("out_stats_table",
       po::bool_switch(&cmdargs->out_stats_table_flag) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               auto adder =
                   cli::addDependencies({"like"}, "out_stats_table", cmdargs);
               adder(v);
             }
           }),

       "Output data is a stats table (use --like to pass template table for "
       "measure, columns, and rows heads)")

      //

      ("invert_contrast",

       po::value(&cmdargs->invert_contrast),

       "All voxels in volume greater than threshold are replaced with "
       "255-value. Only makes sense for 8 bit images. Only operates on first "
       "frame.")

      //

      ("input_volume,i",

       po::value(&cmdargs->input_volume),

       "input_volume")

      //

      ("output_volume,o",

       po::value(&cmdargs->output_volume),

       "output_volume")

      //

      ("conform,c",

       po::bool_switch(&cmdargs->conform_flag),

       "Conform to 1mm voxel size in coronal slice direction with 256^3 or "
       "more.")

      //

      ("conform-dc",

       po::value(&cmdargs->conf_keep_dc) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->conform_flag = true; }),

       "conform-dc")

      //

      ("cw256",

       po::bool_switch(&cmdargs->conform_width_256_flag) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->conform_flag = true;
             }
           }),

       "cw256")

      //

      ("delete-cmds",

       po::bool_switch(&cmdargs->delete_cmds),

       "delete-cmds")

      //

      ("new-transform-fname",

       po::value(&cmdargs->new_transform_fname),

       "new-transform-fname")
      //

      ("sphinx",

       po::bool_switch(&cmdargs->sphinx_flag),

       "Change orientation info to sphinx. Reorient to sphinx the position. "
       "This function is applicable when the input geometry information is "
       "correct but the subject was in the scanner in the 'sphinx' position "
       "(ie AP in line with the bore) instead of head-first-supine (HFS). This "
       "is often the case with monkeys. Note that the assumption is that the "
       "geometry information in the input file is otherwise accurate.")

      //

      ("rescale-dicom",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               setenv("FS_RESCALE_DICOM", "1", 1);
               auto tmp = cli::addConflicts({"no-rescale-dicom"},
                                            "rescale-dicom", cmdargs);
               tmp(v);
             }
           }),

       "DO  apply rescale intercept and slope based on (0028,1052) "
       "(0028,1053).")

      //
      ("no-rescale-dicom",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_RESCALE_DICOM", "0", 1);
             }
           }),

       "Do NOT apply rescale intercept and slope based on (0028,1052) "
       "(0028,1053).")

      //

      ("bvec-scanner",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_DESIRED_BVEC_SPACE", "1", 1);
             }
           }),

       "force bvecs to be in scanner space. only applies when reading "
       "dicoms")

      //

      ("bvec-voxel",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_DESIRED_BVEC_SPACE", "2", 1);
             }
           }),

       "force bvecs to be in voxel space. only applies when reading"
       " dicoms.")

      //

      ("no-analyze-rescale",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_ANALYZE_NO_RESCALE", "1", 1);
             }
           }),

       "Turns off rescaling of analyze files")

      //

      ("autoalign",

       po::value(&cmdargs->autoalign_file) //
           ->notifier([cmdargs](auto v) {
             io::checkFileReadable(v);
             cmdargs->AutoAlign = MatrixReadTxt(v.c_str(), nullptr);
             fmt::printf("Auto Align Matrix\n");
             MatrixPrint(stdout, cmdargs->AutoAlign);
           }),

       "Text file with autoalign matrix")

      //

      ("nochange,nc",

       po::bool_switch(&cmdargs->nochange_flag),

       "Don't change type of input to that of template")

      //

      ("conform_min,cm",

       po::bool_switch(&cmdargs->conform_min_flag) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->conform_flag = true;
             }
           }),

       "Conform to the src min direction size")

      //

      ("conform_size,cs",

       po::value(&cmdargs->conform_size) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->conform_flag = true; }),

       "conform to the size given in mm")

      //

      ("parse_only,po",

       po::bool_switch(&cmdargs->parse_only_flag),

       "parse_only")

      //

      ("in_info,ii",

       po::bool_switch(&cmdargs->in_info_flag),

       "in_info")

      //

      ("out_info,oi",

       po::bool_switch(&cmdargs->out_info_flag),

       "out_info")

      //

      ("template_info,ti",

       po::bool_switch(&cmdargs->template_info_flag),

       "Dump info about template")

      //

      ("in_stats,is",

       po::bool_switch(&cmdargs->in_stats_flag),

       "Print statistics on input volume")

      //

      ("out_stats,os",

       po::bool_switch(&cmdargs->out_stats_flag),

       "Print statistics on output volume")

      //

      ("read_only,ro",

       po::bool_switch(&cmdargs->read_only_flag),

       "read_only")

      //

      ("no_write,nw",

       po::bool_switch(&cmdargs->no_write_flag),

       "no_write")

      //

      ("in_matrix,im",

       po::bool_switch(&cmdargs->in_matrix_flag),

       "in_matrix")

      //

      ("out_matrix,om",

       po::bool_switch(&cmdargs->out_matrix_flag),

       "out_matrix")

      //

      ("force_ras_good",

       po::bool_switch(&cmdargs->force_ras_good) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               auto adder = cli::addConflicts(
                   {"in_i_direction", "in_j_direction", "in_k_direction"},
                   "force_ras_good", cmdargs);
               adder(v);
             }
           }),

       "force_ras_good")

      //

      ("split",

       po::bool_switch(&cmdargs->split_frames_flag),

       "Split output frames into separate output files. Example: mri_convert "
       "a.nii b.nii --split will create b0000.nii b0001.nii b0002.nii ...")

      //

      /* transform related things here */

      ("apply_transform,T,at",

       po::value(&cmdargs->transform_fname) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->transform_flag        = true;
             cmdargs->invert_transform_flag = false;
           }),

       "Apply transform given by xfm or m3z files. The volume can be resampled "
       "into another space by supplying a transform using the -apply_transform "
       "flag. This reads the transform file and applies the transform (when "
       "--apply_inverse_transform is used, the transform is inverted an then "
       "applied). An example of a transform file is talairach.xfm as found in "
       "subjectid/mri/transforms. To convert a subject's orig volume to "
       "talairach space, execute the following lines: cd subjectid/mri , mkdir "
       "talairach, mri_convert orig.mgz --apply_transform "
       "transforms/talairach.xfm -oc 0 0 0 orig.talairach.mgz. This properly "
       "accounts for the case where the input volume does not have it's "
       "coordinate center at 0. To evaluate the result, run: tkmedit -f "
       "$SUBJECTS_DIR/talairach/mri/orig.mgz -aux orig.talairach.mgz . The "
       "main and aux volumes should overlap very closely. If they do not, use "
       "tkregister2 to fix it.")

      //

      ("like",

       po::value(&cmdargs->out_like_name) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->out_like_flag = true;
             // creates confusion when this is printed:
             // fmt::printf("WARNING: --like does not work on multi-frame
             // data\n"); but we'll leave it here for the interested coder
           }),

       "Output is embedded in a volume like name, or in stats-table like name "
       "(measure, columns, rows)")

      //

      ("crop",

       po::value(&cmdargs->crop_center) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto checker = cli::checkSize("crop", 3);
             checker(v);
             cmdargs->crop_flag = true;
           }),

       "Crop to 256 around center (x,y,z)")

      //

      ("slice-crop",

       po::value(&cmdargs->slice_crop) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto checker = cli::checkSize("slice-crop", 2);
             checker(v);
             cmdargs->slice_crop_flag  = true;
             cmdargs->slice_crop_start = cmdargs->slice_crop[0];
             cmdargs->slice_crop_stop  = cmdargs->slice_crop[1];
             if (cmdargs->slice_crop_start > cmdargs->slice_crop_stop) {
               fmt::fprintf(stderr, "ERROR: s_start > s_end\n");
               exit(1);
             }
           }),

       "Keep slices s_start to s_end")

      //

      ("cropsize",

       po::value(&cmdargs->cropsize) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto checker = cli::checkSize("cropsize", 3);
             checker(v);
             cmdargs->crop_flag = true;
           }),

       "Crop to size <dx, dy, dz>")

      //

      ("devolvexfm",

       po::value(&cmdargs->devolvexfm_subject) //
           ->notifier([cmdargs](auto /*unused*/) {
             /* devolve xfm to account for cras != 0 */
             cmdargs->DevXFM = 0;
           }),

       "devolvexfm")

      //

      ("apply_inverse_transform,ait",

       po::value(&cmdargs->transform_fname) //
           ->notifier([cmdargs](auto v) {
             io::checkFileReadable(v);
             cmdargs->transform_flag        = true;
             cmdargs->invert_transform_flag = true;
           }),

       "Apply inverse transform given by xfm or m3z files.")

      //

      ("upsample",

       po::value(&cmdargs->upsample_factor) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->upsample_flag = true; }),

       "Reduce voxel size by a factor of N in all dimensions")

      //

      ("in_i_size,iis",

       po::value(&cmdargs->in_i_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "in_i_size", std::greater<>());
             checker(v);
             cmdargs->in_i_size_flag = true;
           }),

       "in_i_size")

      //

      ("in_j_size,ijs",

       po::value(&cmdargs->in_j_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "in_j_size", std::greater<>());
             checker(v);
             cmdargs->in_j_size_flag = true;
           }),

       "in_j_size")

      //

      ("in_k_size,iks",

       po::value(&cmdargs->in_k_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "in_k_size", std::greater<>());
             checker(v);
             cmdargs->in_k_size_flag = true;
           }),

       "in_k_size")

      //

      ("out_i_size,ois",

       po::value(&cmdargs->out_i_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "out_i_size", std::greater<>());
             checker(v);
             cmdargs->out_i_size_flag = true;
           }),

       "out_i_size")

      //

      ("out_j_size,ojs",

       po::value(&cmdargs->out_j_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "out_j_size", std::greater<>());
             checker(v);
             cmdargs->out_j_size_flag = true;
           }),

       "out_j_size")

      //

      ("out_k_size,oks",

       po::value(&cmdargs->out_k_size) //
           ->notifier([cmdargs](auto v) {
             auto checker =
                 cli::checkValue(0.0F, "out_k_size", std::greater<>());
             checker(v);
             cmdargs->out_k_size_flag = true;
           }),

       "out_k_size")

      //

      ("ctab",

       po::value(&cmdargs->colortablefile),

       "ctab")

      //

      ("nth_frame,nth",

       po::value(&cmdargs->nthframe),

       "Specify frame number")

      //

      ("no_translate,nt",

       po::bool_switch(&cmdargs->translate_labels_flag),

       "no_translate")

      //

      ("zero_outlines,zo",

       po::bool_switch(&cmdargs->zero_outlines_flag),

       "zero_outlines")

      //

      ("fill_parcellation,fp",

       po::bool_switch(&cmdargs->fill_parcellation_flag),

       "fill_parcellation")
      //

      ("roi",

       po::bool_switch(&cmdargs->roi_flag),

       "roi flag")

      //

      ("dil-seg-mask",

       po::value(&cmdargs->dil_seg_mask),

       "Dilate segmentation boundaries to fill mask")

      //

      ("erode-seg",

       po::value(&cmdargs->n_erode_seg) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->erode_seg_flag = true; }),

       "Erode segmentation boundaries Nerode times (based on 6 nearest "
       "neighbors)")

      //

      ("dil-seg",

       po::value(&cmdargs->n_dil_seg) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->dil_seg_flag = true; }),

       "Dilate segmentation boundaries Ndilate times (based on 6 nearest "
       "neighbors) to fill seg=0 voxels")

      //

      ("cutends",

       po::value(&cmdargs->ncutends) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->cutends_flag = true; }),

       "Remove ncut slices from the ends")

      //

      ("out_i_count,oni,oic",

       po::value(&cmdargs->out_n_i) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->out_n_i_flag = true; }),

       "out_i_count")

      //

      ("out_j_count,onj,ojc",

       po::value(&cmdargs->out_n_j) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->out_n_j_flag = true; }),

       "out_i_count")

      //

      ("out_k_count,onk,okc",

       po::value(&cmdargs->out_n_k) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->out_n_k_flag = true; }),

       "out_i_count")

      //

      ("downsample2,ds2",

       po::bool_switch(&cmdargs->downsample2_flag),

       "downsample2")

      //

      ("in_i_count,ini,iic",

       po::value(&cmdargs->in_n_i) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_n_i_flag = true; }),

       "in_i_count")

      //

      ("in_j_count,inj,ijc",

       po::value(&cmdargs->in_n_j) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_n_j_flag = true; }),

       "in_j_count")

      //

      ("in_k_count,ink,ikc",

       po::value(&cmdargs->in_n_k) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_n_k_flag = true; }),

       "in_k_count")

      //

      ("tr",

       po::value(&cmdargs->in_tr) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_tr_flag = true; }),

       "TR in msec")

      //

      ("TI",

       po::value(&cmdargs->in_ti) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_ti_flag = true; }),

       "TI in msec (note uppercase flag)")

      //

      ("te",

       po::value(&cmdargs->in_te) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_te_flag = true; }),

       "TE in msec")

      //

      ("flip_angle",

       po::value(&cmdargs->in_flip_angle) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_flip_angle_flag = true;
           }),

       "Angle in radians")

      //

      ("in_name",

       po::value(&cmdargs->in_name)->required(), "in_name")

      //

      ("out_name",

       po::value(&cmdargs->out_name) //
           ->notifier([cmdargs](auto v) {
             auto adder = cli::addConflicts({"read_only", "no_write"},
                                            "out_name", cmdargs);
             adder(v);
           }),

       "out_name")

      //

      ("zero_ge_z_offset,zgez",

       po::bool_switch(&cmdargs->zero_ge_z_offset_flag) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               auto adder = cli::addConflicts({"no_zero_ge_z_offset"},
                                              "zero_ge_z_offset", cmdargs);
               adder(v);
             }
           }),

       "Set c_s=0 (appropriate for dicom files from GE machines with isocenter "
       "scanning)")

      //

      ("no_zero_ge_z_offset,nozgez",

       po::bool_switch(&cmdargs->no_zero_ge_z_offset_flag),

       "no_zero_ge_z_offset")

      //

      ("nskip",

       po::value(&cmdargs->nskip) //
           ->notifier(cli::checkValue(0, "nskip", std::greater_equal<>())),

       "Skip the first n frames")

      //

      ("ndrop",

       po::value(&cmdargs->ndrop) //
           ->notifier(cli::checkValue(0, "ndrop", std::greater_equal<>())),

       "Drop the last n frames")

      //

      ("diag",

       po::value(&Gdiag_no),

       "diag")

      //

      ("mra",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               SliceResElTag1 = 0x50;
               SliceResElTag2 = 0x88;
             }
           }),

       "This flag forces DICOMread to first use 18,50 to get the slice "
       "thickness instead of 18,88. This is needed with siemens mag res "
       "angiogram (MRAs)")

      //

      ("auto-slice-res",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               AutoSliceResElTag = 1;
             }
           }),

       "Automatically determine whether to get slice thickness from 18,50 or "
       "18,88 depending upon the value of 18,23")

      //

      ("no-strip-pound",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               MRIIO_Strip_Pound = 0;
             }
           }),

       "no-strip-pound")

      //

      ("in_nspmzeropad",

       po::value(&N_Zero_Pad_Input),

       "in_nspmzeropad")

      //

      ("nspmzeropad",

       po::value(&N_Zero_Pad_Output),

       "out_nspmzeropad")

      //

      ("out_nspmzeropad",

       po::value(&N_Zero_Pad_Output),

       "out_nspmzeropad")

      //

      ("mosaic-fix-noascii",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("FS_MOSAIC_FIX_NOASCII", "1", 1);
             }
           }),

       "mosaic-fix-noascii")

      //

      ("nslices-override",

       po::value(&cmdargs->nslices_override) //
           ->notifier([cmdargs](auto /*unused*/) {
             fmt::printf("NSlicesOverride %d\n", cmdargs->nslices_override);
             std::string tmpstr = std::to_string(cmdargs->nslices_override);
             setenv("NSLICES_OVERRIDE", tmpstr.data(), 1);
           }),

       "Use this number of slices when converting DICOM mosaics")

      //

      ("ncols-override",

       po::value(&cmdargs->ncols_override) //
           ->notifier([cmdargs](auto /*unused*/) {
             fmt::printf("NColsOverride %d\n", cmdargs->ncols_override);
             std::string tmpstr = std::to_string(cmdargs->ncols_override);
             setenv("NCOLS_OVERRIDE", tmpstr.data(), 1);
           }),

       "ncols-override")

      //

      ("nrows-override",

       po::value(&cmdargs->nrows_override) //
           ->notifier([cmdargs](auto /*unused*/) {
             fmt::printf("NRowsOverride %d\n", cmdargs->nrows_override);
             std::string tmpstr = std::to_string(cmdargs->nrows_override);
             setenv("NROWS_OVERRIDE", tmpstr.data(), 1);
           }),

       "nrows-override")

      //

      ("statusfile,status",

       po::value(&cmdargs->statusfile) //
           ->notifier([](auto v) {
             auto checker = io::checkFileWriteable;
             checker(v);
             auto allocator = cli::allocateExternalString(&SDCMStatusFile);
             allocator(v);
           }),

       "File name to write percent complete for Siemens DICOM. Status file for "
       "DICOM conversion")

      //

      ("sdcmlist",

       po::value(&cmdargs->sdcmlist) //
           ->notifier([](auto v) {
             auto checker = io::checkFileWriteable;
             checker(v);
             auto allocator = cli::allocateExternalString(&SDCMListFile);
             allocator(v);
           }),

       "File name that contains a list of Siemens DICOM files that are in the "
       "same run as the one listed on the command-line. If not present, the "
       "directory will be scanned, but this can take a while. List of DICOM "
       "files for conversion")

      //

      ("fsubsample",

       po::value(&cmdargs->fsubsample) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto checker = cli::checkSize("fsubsample", 3);
             checker(v);
             cmdargs->SubSampStart = cmdargs->fsubsample[0];
             cmdargs->SubSampDelta = cmdargs->fsubsample[1];
             cmdargs->SubSampEnd   = cmdargs->fsubsample[2];

             if (cmdargs->SubSampDelta == 0) {
               fmt::printf("ERROR: don't use subsample delta = 0\n");
               exit(1);
             }
             cmdargs->subsample_flag = true;
           }),

       "Frame subsampling (end = -1 for end)")

      //

      ("mid-frame",

       po::bool_switch(&cmdargs->mid_frame_flag) //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->frame_flag = true;
             }
           }),

       "Keep only the middle frame")

      //

      ("in_center,ic",

       po::value(&cmdargs->in_center) //
           ->multitoken()
           ->notifier(cli::checkSize("in_center", 3)),

       "in_center")

      //

      ("delta_in_center,dic",

       po::value(&cmdargs->delta_in_center) //
           ->multitoken()
           ->notifier(cli::checkSize("delta_in_center", 3)),

       "delta_in_center")

      //

      ("out_center,oc",

       po::value(&cmdargs->out_center)
           ->multitoken()
           ->notifier(cli::checkSize("out_center", 3)),

       "out_center")

      //

      ("voxsize,vs",

       po::value(&cmdargs->voxel_size) //
           ->multitoken()
           ->notifier(cli::checkSize("voxsize", 3)),

       "voxel_size")

      //

      ("downsample,ds",

       po::value(&cmdargs->downsample_factor) //
           ->multitoken()
           ->notifier(cli::checkSize("downsample", 3)),

       "downsample")

      //

      ("reduce",

       po::value(&cmdargs->reduce) //
           ->notifier([cmdargs](auto /*unused*/) {
             fmt::printf("reducing input image %d times\n", cmdargs->reduce);
           }),

       "reduce")

      //

      ("bfile-little-endian",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               setenv("BFILE_LITTLE_ENDIAN", "1", 1);
             }
           }),

       "Write out bshort/bfloat files in little endian")

      //

      ("rescale",

       po::value(&cmdargs->rescale_factor),

       "Rescale so that the global mean of input is rescale_factor")

      //

      ("scale,sc",

       po::value(&cmdargs->scale_factor),

       "Input intensity scale factor")

      //

      ("out-scale,osc",

       po::value(&cmdargs->out_scale_factor),

       "Output intensity scale factor")

      //

      ("dicomread2",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               UseDICOMRead2 = 1;
               auto checker =
                   cli::addConflicts({"dicomread0"}, "dicomread2", cmdargs);
               checker(v);
             }
           }),

       "dicomread2")

      //

      ("dicomread0",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               UseDICOMRead2 = 0;
             }
           }),

       "dicomread0")

      //

      ("subject_name,sn",

       po::value(&cmdargs->subject_name),

       "subject_name")

      //

      ("gdf_image_stem,gis",

       po::value(&cmdargs->gdf_image_stem),

       "gdf_image_stem")

      //

      ("reslice_like,rl",

       po::value(&cmdargs->reslice_like_name) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->reslice_like_flag = true;
           }),

       "reslice_like")

      //

      ("slice-bias",

       po::value(&cmdargs->SliceBiasAlpha) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->SliceBias = true; }),

       "Apply half-cosine bias field")

      //

      ("in_like,il",

       po::value(&cmdargs->in_like_name) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->in_like_flag = true; }),

       "in_like")

      //

      ("color_file,cf",

       po::value(&cmdargs->color_file_name) //
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->color_file_flag = true; }),

       "color_file")

      //

      ("no_scale,ns",

       po::bool_switch(&cmdargs->no_scale_flag),

       "1 means don't rescale values for COR")

      //

      ("crop_gdf,cg",

       po::bool_switch() //
           ->notifier([](auto v) {
             if (v) {
               mriio_set_gdf_crop_flag(TRUE);
             }
           }),

       "Apply GDF cropping")

      //

      ("in_orientation",

       po::value(&cmdargs->in_orientation_string) //
           ->notifier(mri::checkOrientationString),

       "see SPECIFYING THE ORIENTATION")

      //

      ("out_orientation",

       po::value(&cmdargs->out_orientation_string) //
           ->notifier(mri::checkOrientationString),

       "See SETTING ORIENTATION")

      //

      ("fwhm",

       po::value(&cmdargs->fwhm) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->gstd = cmdargs->fwhm / sqrt(log(256.0));
             fmt::printf("fwhm = %g, gstd = %g\n", cmdargs->fwhm,
                         cmdargs->gstd);
           }),

       "Smooth input volume by fwhm mm")

      //

      ("out_data_type,odt",

       po::value(&cmdargs->out_data_type_string) //
           ->notifier([cmdargs](auto /*unused*/) {
             if (boost::iequals(cmdargs->out_data_type_string, "uchar")) {
               cmdargs->out_data_type = MRI_UCHAR;
             } else if (boost::iequals(cmdargs->out_data_type_string,
                                       "short")) {
               cmdargs->out_data_type = MRI_SHORT;
             } else if (boost::iequals(cmdargs->out_data_type_string, "int")) {
               cmdargs->out_data_type = MRI_INT;
             } else if (boost::iequals(cmdargs->out_data_type_string,
                                       "float")) {
               cmdargs->out_data_type = MRI_FLOAT;
             } else if (boost::iequals(cmdargs->out_data_type_string, "rgb")) {
               cmdargs->out_data_type = MRI_RGB;
             } else {
               fmt::fprintf(stderr, "\n%s: unknown data type \"%s\"\n",
                            Progname, cmdargs->out_data_type_string);
               fs::util::cli::usage_message(stdout);
               exit(1);
             }
           }),

       "out_data_type")

      //

      ("resample_type,rt",

       po::value(&cmdargs->resample_type) //
           ->notifier([cmdargs](auto /*unused*/) {
             if (boost::iequals(cmdargs->resample_type, "interpolate")) {
               cmdargs->resample_type_val = SAMPLE_TRILINEAR;
             } else if (boost::iequals(cmdargs->resample_type, "nearest")) {
               cmdargs->resample_type_val = SAMPLE_NEAREST;
             } else if (boost::iequals(cmdargs->resample_type, "vote")) {
               cmdargs->resample_type_val = SAMPLE_VOTE;
             } else if (boost::iequals(cmdargs->resample_type, "weighted")) {
               cmdargs->resample_type_val = SAMPLE_WEIGHTED;
             } /*else if (boost::iequals(cmdargs->resample_type, "sinc")) {
          cmdargs->resample_type_val = SAMPLE_SINC;
        }*/
             else if (boost::iequals(cmdargs->resample_type, "cubic")) {
               cmdargs->resample_type_val = SAMPLE_CUBIC_BSPLINE;
             } else {
               fmt::fprintf(stderr, "\n%s: unknown resample type \"%s\"\n",
                            Progname, cmdargs->resample_type);
               fs::util::cli::usage_message(stdout);
               exit(1);
             }
           }),

       "Default is interpolate")

      //

      ("in_i_direction,iid",

       po::value(&cmdargs->in_i_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker =
                 mri::checkNorm("in_i_direction", cmdargs->in_i_direction_flag);
             auto sizeChecker = cli::checkSize("in_i_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "in_i_direction")

      //

      ("in_j_direction,ijd",

       po::value(&cmdargs->in_j_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker =
                 mri::checkNorm("in_j_direction", cmdargs->in_j_direction_flag);
             auto sizeChecker = cli::checkSize("in_j_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "in_j_direction")

      //

      ("in_k_direction,ikd",

       po::value(&cmdargs->in_k_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker =
                 mri::checkNorm("in_k_direction", cmdargs->in_k_direction_flag);
             auto sizeChecker = cli::checkSize("in_k_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "in_k_direction")

      //

      ("out_i_direction,oid",

       po::value(&cmdargs->out_i_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker = mri::checkNorm("out_i_direction",
                                               cmdargs->out_i_direction_flag);
             auto sizeChecker = cli::checkSize("out_i_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "out_i_direction")

      //

      ("out_j_direction,ojd",

       po::value(&cmdargs->out_j_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker = mri::checkNorm("out_j_direction",
                                               cmdargs->out_j_direction_flag);
             auto sizeChecker = cli::checkSize("out_j_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "out_j_direction")

      //

      ("out_k_direction,okd",

       po::value(&cmdargs->out_k_directions) //
           ->multitoken()
           ->notifier([cmdargs](auto v) {
             auto normChecker = mri::checkNorm("out_k_direction",
                                               cmdargs->out_k_direction_flag);
             auto sizeChecker = cli::checkSize("out_k_direction", 3);
             sizeChecker(v);
             normChecker(v);
           }),

       "out_k_direction")

      //

      ("in_type,it",

       po::value(&cmdargs->in_type_string) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->force_in_type_flag = true;
             cmdargs->forced_in_type =
                 string_to_type(cmdargs->in_type_string.data());
           }),

       "in_type")

      //

      ("out_type,ot",

       po::value(&cmdargs->out_type_string) //
           ->notifier([cmdargs](auto v) {
             auto adder =
                 cli::addDependencies({"out_name"}, "out_type", cmdargs);
             adder(v);
             cmdargs->force_out_type_flag = true;
             cmdargs->forced_out_type =
                 string_to_type(cmdargs->out_type_string.data());
           }),

       "out_type")

      //

      ("template_type,tt",

       po::value(&cmdargs->template_type_string) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->force_template_type_flag = true;
             cmdargs->forced_template_type =
                 string_to_type(cmdargs->template_type_string.data());
           }),

       "template_type")

      //

      ("frame,f",

       po::value(&cmdargs->frames) //
           ->multitoken()
           ->notifier(
               [cmdargs](auto /*unused*/) { cmdargs->frame_flag = true; }),

       "Keep only 0-based frame number(s)")

      //

      ("smooth_parcellation,sp",

       po::value(&cmdargs->smooth_parcellation_count) //
           ->notifier(cli::checkRange(14, 26, "smooth_parcellation")),

       "smooth_parcellation")

      //

      ("ascii",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->ascii_flag         = 1;
               cmdargs->force_in_type_flag = true;
             }
           }),

       "Save output as ascii. This will be a data file with a single column of "
       "data. The fastest dimension will be col, then row, then slice, then "
       "frame")

      //

      ("ascii+crsf",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->ascii_flag         = 2;
               cmdargs->force_in_type_flag = true;
             }
           }),

       "Same as --ascii but includes col, row, slice, and frame")

      //

      ("ascii-fcol",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->ascii_flag         = 3;
               cmdargs->force_in_type_flag = true;
               fmt::printf("blowme please");
             }
           }),

       "ascii-fcol");
}

#endif // MRI_TOOLS_MRI_CONVERT_HPP
