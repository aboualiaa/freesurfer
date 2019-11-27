/**
 * @file  mri_convert.c
 * @brief performs all kinds of conversion and reformatting of MRI volume files
 *
 */
/*
 * Original Author: Bruce Fischl (Apr 16, 1997)
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2017/02/16 19:15:42 $
 *    $Revision: 1.227 $
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "fmt/printf.h"

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

struct CMDARGS {
  CMDARGS(int argc, char *argv[]) : raw(argv, argc){}; // NOLINT

public:
  gsl::multi_span<char *> raw;
  std::string fvol;
  std::string volmask;
  std::string flh;
  std::string lhsurface;
  std::string lhsurface2;
  std::string lhlabel;
  std::string lhmask;
  std::string frh;
  std::string rhsurface;
  std::string rhsurface2;
  std::string rhlabel;
  std::string rhmask;
  std::string outdir;
  std::string volcon;
  std::string lhcon;
  std::string rhcon;
  std::string volconS;
  std::string lhconS;
  std::string rhconS;
  std::string volconL;
  std::string lhconL;
  std::string rhconL;
  std::string volrhomean;
  std::string lhrhomean;
  std::string rhrhomean;
  std::string matfile; // read: matrix file
  std::vector<double> rholist;
  double distthresh;
  int nthreads;
  bool DoMat; // do matrix??
  bool ForceFail;
  bool SaveTest;
  bool DoTest;
  bool debug = false;
  bool DoDist;
};

struct ENV {
  std::string vcid = "$Id: mri_wbc.c,v 1.8 2015/07/22 21:51:35 zkaufman Exp $";
};

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace po = boost::program_options;
namespace fsys = std::filesystem; // sadly fs is already defined: freesurfer
using podesc = po::options_description;
using povm = po::variables_map;
namespace pocl = boost::program_options::command_line_style;

auto cl_style =
    pocl::allow_short | pocl::short_allow_adjacent | pocl::short_allow_next |
    pocl::allow_long | pocl::long_allow_adjacent | pocl::long_allow_next |
    pocl::allow_sticky | pocl::allow_dash_for_short | pocl::allow_long_disguise;
namespace boost::program_options {

///
/// \param vm
/// \param for_what
/// \param required_option
inline void option_dependency(povm const &vm, char const *for_what,
                              char const *required_option) {
  if ((vm.count(for_what) != 0U) && !vm[for_what].defaulted()) {
    if (vm.count(required_option) == 0 || vm[required_option].defaulted()) {
      throw std::logic_error(std::string("Option '") + for_what +
                             "' requires option '" + required_option + "'.");
    }
  }
}

///
/// \param vm
/// \param opt1
/// \param opt2
inline void conflicting_options(povm const &vm, std::string const &opt1,
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
/// \param env
/// \return true if all logic is ok, false otherwise
static bool good_cmdline_args(CMDARGS *cmdargs, ENV *env);

/// \brief initialize options description and save values in cmdargs
/// \param desc
/// \param cmdargs
static void initArgDesc(podesc *desc, CMDARGS *cmdargs);

///
/// \param desc
/// \param env
inline static void print_usage(podesc const &desc, ENV *env) {
  std::cout << desc << "\n" << env->vcid << std::endl;
}

///
/// \param desc
/// \param env
inline static void print_help(podesc const &desc, ENV *env) {
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

int debug = 0;

extern int errno;

const char *Progname;

int ncutends = 0, cutends_flag = 0;

int slice_crop_flag = FALSE;
int slice_crop_start, slice_crop_stop;
int SplitFrames = 0;
int DeleteCMDs = 0;
char NewTransformFname[2000];
int DoNewTransformFname = 0;

/*-------------------------------------------------------------*/
int main(int argc, char *argv[]) {

  auto env = ENV();
  auto cmdargs = CMDARGS(argc, argv);
  if (!good_cmdline_args(&cmdargs, &env)) {
    return 1;
  }

  int outside_val = 0;
  int nargs = 0;
  MRI *mri{nullptr};
  MRI *mri2{nullptr};
  MRI *mri_template;
  MRI *mri_in_like;
  int i;
  int err = 0;
  int reorder_vals[3];
  float invert_val{1.0};
  bool in_info_flag{false};
  bool out_info_flag{false};
  bool template_info_flag{false};
  bool voxel_size_flag{false};
  bool nochange_flag{false};
  bool conform_flag{false};
  bool conform_min{false}; // conform to the smallest dimension
  int conform_width;
  bool conform_width_256_flag{false};
  int ConfKeepDC = 0;
  bool parse_only_flag{false};
  bool reorder_flag{false};
  int reorder4_vals[4];
  int reorder4_flag;
  bool in_stats_flag{false};
  bool out_stats_flag{false};
  bool read_only_flag{false};
  bool no_write_flag{false};
  std::array<char, STRLEN> in_name{};
  std::array<char, STRLEN> out_name{};
  int in_volume_type;
  int out_volume_type{MRI_VOLUME_TYPE_UNKNOWN};
  char resample_type[STRLEN];
  int resample_type_val{SAMPLE_TRILINEAR};
  bool in_i_size_flag{false};
  bool in_j_size_flag{false};
  bool in_k_size_flag{false};
  int crop_flag = FALSE;
  bool out_i_size_flag{false};
  bool out_j_size_flag{false};
  bool out_k_size_flag{false};
  float in_i_size;
  float in_j_size;
  float in_k_size;
  float out_i_size;
  float out_j_size;
  float out_k_size;
  int crop_center[3];
  int sizes_good_flag;
  int crop_size[3];
  float in_i_directions[3];
  float in_j_directions[3];
  float in_k_directions[3];
  float out_i_directions[3];
  float out_j_directions[3];
  float out_k_directions[3];
  float voxel_size[3];
  bool in_i_direction_flag{false};
  bool in_j_direction_flag{false};
  bool in_k_direction_flag{false};
  bool out_i_direction_flag{false};
  bool out_j_direction_flag{false};
  bool out_k_direction_flag{false};
  int in_orientation_flag = FALSE;
  char in_orientation_string[STRLEN];
  int out_orientation_flag = FALSE;
  char out_orientation_string[STRLEN];
  char colortablefile[STRLEN] = "";
  char tmpstr[STRLEN];
  char *stem;
  char *ext;
  char ostr[4] = {'\0', '\0', '\0', '\0'};
  char *errmsg = nullptr;
  int in_tr_flag = 0;
  float in_tr = 0;
  int in_ti_flag = 0;
  float in_ti = 0;
  int in_te_flag = 0;
  float in_te = 0;
  int in_flip_angle_flag = 0;
  float in_flip_angle = 0;
  float magnitude;
  float i_dot_j;
  float i_dot_k;
  float j_dot_k;
  float in_center[3];
  float out_center[3];
  float delta_in_center[3];
  bool in_center_flag{false};
  bool out_center_flag{false};
  bool delta_in_center_flag{false};
  int out_data_type{-1};
  char out_data_type_string[STRLEN];
  int out_n_i;
  int out_n_j;
  int out_n_k;
  bool out_n_i_flag{false};
  bool out_n_j_flag{false};
  bool out_n_k_flag{false};
  float fov_x;
  float fov_y;
  float fov_z;
  bool force_in_type_flag{false};
  bool force_out_type_flag{false};
  int forced_in_type{MRI_VOLUME_TYPE_UNKNOWN};
  int forced_out_type{MRI_VOLUME_TYPE_UNKNOWN};
  std::array<char, STRLEN> in_type_string{};
  char out_type_string[STRLEN];
  std::array<char, STRLEN> subject_name{};
  bool force_template_type_flag{false};
  int forced_template_type{MRI_VOLUME_TYPE_UNKNOWN};
  char template_type_string[STRLEN];
  char reslice_like_name[STRLEN];
  bool reslice_like_flag{false};
  int nframes = 0;
  bool frame_flag{false};
  bool mid_frame_flag{false};
  int frames[2000];
  char *errormsg;
  bool subsample_flag{false};
  int SubSampStart;
  int SubSampDelta;
  int SubSampEnd;
  bool downsample2_flag{false};
  bool downsample_flag{false};
  float downsample_factor[3];
  char in_name_only[STRLEN];
  char transform_fname[STRLEN];
  bool transform_flag{false};
  bool invert_transform_flag{false};
  LTA *lta_transform = nullptr;
  MRI *mri_transformed = nullptr;
  MRI *mritmp = nullptr;
  int transform_type = -1;
  MATRIX *inverse_transform_matrix;
  bool smooth_parcellation_flag{false};
  int smooth_parcellation_count;
  bool in_like_flag{false};
  char in_like_name[STRLEN];
  char out_like_name[STRLEN];
  int out_like_flag = FALSE;
  int in_n_i;
  int in_n_j;
  int in_n_k;
  bool in_n_i_flag{false};
  bool in_n_j_flag{false};
  bool in_n_k_flag{false};
  bool fill_parcellation_flag{false};
  int read_parcellation_volume_flag;
  bool zero_outlines_flag{false};
  int erode_seg_flag = FALSE;
  int n_erode_seg;
  int dil_seg_flag = FALSE;
  int n_dil_seg;
  std::array<char, STRLEN> dil_seg_mask{};
  int read_otl_flags;
  bool color_file_flag{false};
  char color_file_name[STRLEN];
  int no_scale_flag{false}; // TODO: convert to bool
  int temp_type;
  bool roi_flag{false};
  FILE *fptmp;
  int j;
  bool translate_labels_flag{false};
  int force_ras_good = FALSE;
  std::array<char, STRLEN> gdf_image_stem{};
  bool in_matrix_flag{false};
  bool out_matrix_flag{false};
  float conform_size{1.0};
  int zero_ge_z_offset_flag = FALSE; // E/
  int nskip = 0;                     // number of frames to skip from start
  int ndrop = 0;                     // number of frames to skip from end
  VOL_GEOM vgtmp;
  LT *lt = nullptr;
  int DevXFM = 0;
  char devxfm_subject[STRLEN];
  MATRIX *T;
  float scale_factor{1};
  float out_scale_factor{1};
  float rescale_factor{1};
  int nthframe = -1;
  int reduce = 0;
  float fwhm = -1;
  float gstd = -1;
  char cmdline[STRLEN];
  int sphinx_flag = FALSE;
  int LeftRightReverse = FALSE;
  int LeftRightReversePix = FALSE;
  int LeftRightMirrorFlag = FALSE;  // mirror half of the image
  char LeftRightMirrorHemi[STRLEN]; // which half to mirror (lh, rh)
  int LeftRightKeepFlag = FALSE;    // keep half of the image
  int LeftRightSwapLabel = FALSE;
  int FlipCols = FALSE;
  int SliceReverse = FALSE;
  int SliceBias = FALSE;
  float SliceBiasAlpha = 1.0;
  float v;
  char AutoAlignFile[STRLEN];
  MATRIX *AutoAlign = nullptr;
  MATRIX *cras = nullptr;
  MATRIX *vmid = nullptr;
  int ascii_flag = FALSE;
  int c = 0;
  int r = 0;
  int s = 0;
  int f = 0;
  int c1 = 0;
  int c2 = 0;
  int r1 = 0;
  int r2 = 0;
  int s1 = 0;
  int s2 = 0;
  bool InStatTableFlag{false};
  bool OutStatTableFlag{false};
  int UpsampleFlag = 0;
  int UpsampleFactor = 0;

  FSinit();
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  make_cmd_version_string(
      argc, argv, "$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $",
      "$Name:  $", cmdline);

  for (i = 0; i < argc; i++) {
    fmt::printf("%s ", argv[i]);
  }
  fmt::printf("\n");
  fflush(stdout);

  crop_size[0] = crop_size[1] = crop_size[2] = 256;
  crop_center[0] = crop_center[1] = crop_center[2] = 128;
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) {
      fptmp = fopen("debug.gdb", "w");
      fmt::fprintf(fptmp, "# source this file in gdb to debug\n");
      fmt::fprintf(fptmp, "file %s \n", argv[0]);
      fmt::fprintf(fptmp, "run ");
      for (j = 1; j < argc; j++) {
        if (strcmp(argv[j], "--debug") != 0) {
          fmt::fprintf(fptmp, "%s ", argv[j]);
        }
      }
      fmt::fprintf(fptmp, "\n");
      fclose(fptmp);
      break;
    }
  }

  /* ----- get the program name ----- */
  Progname = strrchr(argv[0], '/');
  Progname = (Progname == nullptr ? argv[0] : Progname + 1);

  /* ----- pass the command line to mriio ----- */
  mriio_command_line(argc, argv);

  /* ----- catch no arguments here ----- */
  if (argc == 1) {
    usage(stdout);
    exit(1);
  }

  /* ----- initialize values ----- */
  STAT_TABLE *StatTable = nullptr;
  STAT_TABLE *OutStatTable = nullptr;

  /* rkt: check for and handle version tag */
  nargs = handle_version_option(
      argc, argv, "$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $",
      "$Name:  $");
  if ((nargs != 0) && argc - nargs == 1) {
    exit(0);
  }
  argc -= nargs;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-version2") == 0) {
      exit(97);
    } else if (strcmp(argv[i], "-r") == 0 ||
               strcmp(argv[i], "--reorder") == 0) {
      get_ints(argc, argv, &i, reorder_vals, 3);
      reorder_flag = TRUE;
    } else if (strcmp(argv[i], "-r4") == 0 ||
               strcmp(argv[i], "--reorder4") == 0) {
      get_ints(argc, argv, &i, reorder4_vals, 4);
      reorder4_flag = TRUE;
    } else if (strcmp(argv[i], "-oval") == 0 ||
               strcmp(argv[i], "--outside_val") == 0) {
      get_ints(argc, argv, &i, &outside_val, 1);
      fmt::printf("setting outside val to %d\n", outside_val);
    } else if (strcmp(argv[i], "--no-dwi") == 0) {
      setenv("FS_LOAD_DWI", "0", 1);
    } else if (strcmp(argv[i], "--debug") == 0) {
      debug = 1;
    } else if (strcmp(argv[i], "--left-right-reverse") == 0) {
      LeftRightReverse = 1;
    } else if (strcmp(argv[i], "--left-right-reverse-pix") == 0) {
      LeftRightReversePix = 1;
    } else if (strcmp(argv[i], "--left-right-mirror") == 0) {
      LeftRightMirrorFlag = 1;
      get_string(argc, argv, &i, LeftRightMirrorHemi);
      if ((strcmp(LeftRightMirrorHemi, "lh") != 0) &&
          (strcmp(LeftRightMirrorHemi, "rh") != 0)) {
        fmt::printf(
            "ERROR: pass either 'lh' or 'rh' with --left-right-mirror!\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "--left-right-keep") == 0) {
      LeftRightKeepFlag = 1;
      get_string(argc, argv, &i, LeftRightMirrorHemi);
      if ((strcmp(LeftRightMirrorHemi, "lh") != 0) &&
          (strcmp(LeftRightMirrorHemi, "rh") != 0)) {
        fmt::printf(
            "ERROR: pass either 'lh' or 'rh' with --left-right-keep!\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "--left-right-swap-label") == 0) {
      LeftRightSwapLabel = 1;
    } else if (strcmp(argv[i], "--flip-cols") == 0) {
      FlipCols = 1;
    } else if (strcmp(argv[i], "--slice-reverse") == 0) {
      SliceReverse = 1;
    } else if (strcmp(argv[i], "--ascii") == 0) {
      ascii_flag = 1;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "--ascii+crsf") == 0) {
      ascii_flag = 2;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "--ascii-fcol") == 0) {
      ascii_flag = 3;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "--in_stats_table") == 0) {
      // Input is a stat table
      InStatTableFlag = 1;
    } else if (strcmp(argv[i], "--out_stats_table") == 0) {
      // Input is a stat table
      OutStatTableFlag = 1;
    } else if (strcmp(argv[i], "--invert_contrast") == 0) {
      get_floats(argc, argv, &i, &invert_val, 1);
    } else if (strcmp(argv[i], "-i") == 0 ||
               strcmp(argv[i], "--input_volume") == 0) {
      get_string(argc, argv, &i, in_name.data());
    } else if (strcmp(argv[i], "-o") == 0 ||
               strcmp(argv[i], "--output_volume") == 0) {
      get_string(argc, argv, &i, out_name.data());
    } else if (strcmp(argv[i], "-c") == 0 ||
               strcmp(argv[i], "--conform") == 0) {
      conform_flag = TRUE;
    } else if (strcmp(argv[i], "--conform-dc") == 0) {
      conform_flag = TRUE;
      ConfKeepDC = 1;
    } else if (strcmp(argv[i], "--cw256") == 0) {
      conform_flag = TRUE;
      conform_width_256_flag = TRUE;
    } else if (strcmp(argv[i], "--delete-cmds") == 0) {
      DeleteCMDs = 1;
    } else if (strcmp(argv[i], "--new-transform-fname") == 0) {
      get_string(argc, argv, &i, NewTransformFname);
      DoNewTransformFname = 1;
    } else if (strcmp(argv[i], "--sphinx") == 0) {
      sphinx_flag = TRUE;
    } else if (strcmp(argv[i], "--rescale-dicom") == 0) {
      // DO  apply rescale intercept and slope based on (0028,1052) (0028,1053).
      setenv("FS_RESCALE_DICOM", "1", 1);
    } else if (strcmp(argv[i], "--no-rescale-dicom") == 0) {
      // Do NOT apply rescale intercept and slope based on (0028,1052)
      // (0028,1053).
      setenv("FS_RESCALE_DICOM", "0", 1);
    } else if (strcmp(argv[i], "--bvec-scanner") == 0) {
      // force bvecs to be in scanner space. only applies when
      // reading dicoms
      setenv("FS_DESIRED_BVEC_SPACE", "1", 1);
    } else if (strcmp(argv[i], "--bvec-voxel") == 0) {
      // force bvecs to be in voxel space. only applies when
      // reading dicoms.
      setenv("FS_DESIRED_BVEC_SPACE", "2", 1);
    } else if (strcmp(argv[i], "--no-analyze-rescale") == 0) {
      // Turns off rescaling of analyze files
      setenv("FS_ANALYZE_NO_RESCALE", "1", 1);
    } else if (strcmp(argv[i], "--autoalign") == 0) {
      get_string(argc, argv, &i, AutoAlignFile);
      AutoAlign = MatrixReadTxt(AutoAlignFile, nullptr);
      fmt::printf("Auto Align Matrix\n");
      MatrixPrint(stdout, AutoAlign);
    } else if (strcmp(argv[i], "-nc") == 0 ||
               strcmp(argv[i], "--nochange") == 0) {
      nochange_flag = TRUE;
    } else if (strcmp(argv[i], "-cm") == 0 ||
               strcmp(argv[i], "--conform_min") == 0) {
      conform_min = TRUE;
      conform_flag = TRUE;
    } else if (strcmp(argv[i], "-cs") == 0 ||
               strcmp(argv[i], "--conform_size") == 0) {
      get_floats(argc, argv, &i, &conform_size, 1);
      conform_flag = TRUE;
    } else if (strcmp(argv[i], "-po") == 0 ||
               strcmp(argv[i], "--parse_only") == 0) {
      parse_only_flag = TRUE;
    } else if (strcmp(argv[i], "-ii") == 0 ||
               strcmp(argv[i], "--in_info") == 0) {
      in_info_flag = TRUE;
    } else if (strcmp(argv[i], "-oi") == 0 ||
               strcmp(argv[i], "--out_info") == 0) {
      out_info_flag = TRUE;
    } else if (strcmp(argv[i], "-ti") == 0 ||
               strcmp(argv[i], "--template_info") == 0) {
      template_info_flag = TRUE;
    } else if (strcmp(argv[i], "-is") == 0 ||
               strcmp(argv[i], "--in_stats") == 0) {
      in_stats_flag = TRUE;
    } else if (strcmp(argv[i], "-os") == 0 ||
               strcmp(argv[i], "--out_stats") == 0) {
      out_stats_flag = TRUE;
    } else if (strcmp(argv[i], "-ro") == 0 ||
               strcmp(argv[i], "--read_only") == 0) {
      read_only_flag = TRUE;
    } else if (strcmp(argv[i], "-nw") == 0 ||
               strcmp(argv[i], "--no_write") == 0) {
      no_write_flag = TRUE;
    } else if (strcmp(argv[i], "-im") == 0 ||
               strcmp(argv[i], "--in_matrix") == 0) {
      in_matrix_flag = TRUE;
    } else if (strcmp(argv[i], "-om") == 0 ||
               strcmp(argv[i], "--out_matrix") == 0) {
      out_matrix_flag = TRUE;
    } else if (strcmp(argv[i], "--force_ras_good") == 0) {
      force_ras_good = TRUE;
    } else if (strcmp(argv[i], "--split") == 0) {
      SplitFrames = TRUE;
    }
    // transform related things here /////////////////////
    else if (strcmp(argv[i], "-at") == 0 ||
             strcmp(argv[i], "--apply_transform") == 0 ||
             strcmp(argv[i], "-T") == 0) {
      get_string(argc, argv, &i, transform_fname);
      transform_flag = TRUE;
      invert_transform_flag = FALSE;
    } else if (strcmp(argv[i], "--like") == 0) {
      get_string(argc, argv, &i, out_like_name);
      out_like_flag = TRUE;
      // creates confusion when this is printed:
      // fmt::printf("WARNING: --like does not work on multi-frame data\n");
      // but we'll leave it here for the interested coder
    } else if (strcmp(argv[i], "--crop") == 0) {
      crop_flag = TRUE;
      get_ints(argc, argv, &i, crop_center, 3);
    } else if (strcmp(argv[i], "--slice-crop") == 0) {
      slice_crop_flag = TRUE;
      get_ints(argc, argv, &i, &slice_crop_start, 1);
      get_ints(argc, argv, &i, &slice_crop_stop, 1);
      if (slice_crop_start > slice_crop_stop) {
        fmt::fprintf(stderr, "ERROR: s_start > s_end\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "--cropsize") == 0) {
      crop_flag = TRUE;
      get_ints(argc, argv, &i, crop_size, 3);
    } else if (strcmp(argv[i], "--devolvexfm") == 0) {
      /* devolve xfm to account for cras != 0 */
      get_string(argc, argv, &i, devxfm_subject);
      DevXFM = 1;
    } else if (strcmp(argv[i], "-ait") == 0 ||
               strcmp(argv[i], "--apply_inverse_transform") == 0) {
      get_string(argc, argv, &i, transform_fname);
      if (FileExists(transform_fname) == 0) {
        fmt::fprintf(stderr, "ERROR: cannot find transform file %s\n",
                     transform_fname);
        exit(1);
      }
      transform_flag = TRUE;
      invert_transform_flag = TRUE;
    }
    ///////////////////////////////////////////////////////////
    else if (strcmp(argv[i], "--upsample") == 0) {
      get_ints(argc, argv, &i, &UpsampleFactor, 1);
      UpsampleFlag = 1;
    } else if (strcmp(argv[i], "-iis") == 0 ||
               strcmp(argv[i], "--in_i_size") == 0) {
      get_floats(argc, argv, &i, &in_i_size, 1);
      in_i_size_flag = TRUE;
    } else if (strcmp(argv[i], "-ijs") == 0 ||
               strcmp(argv[i], "--in_j_size") == 0) {
      get_floats(argc, argv, &i, &in_j_size, 1);
      in_j_size_flag = TRUE;
    } else if (strcmp(argv[i], "-iks") == 0 ||
               strcmp(argv[i], "--in_k_size") == 0) {
      get_floats(argc, argv, &i, &in_k_size, 1);
      in_k_size_flag = TRUE;
    } else if (strcmp(argv[i], "-ois") == 0 ||
               strcmp(argv[i], "--out_i_size") == 0) {
      get_floats(argc, argv, &i, &out_i_size, 1);
      out_i_size_flag = TRUE;
    } else if (strcmp(argv[i], "-ojs") == 0 ||
               strcmp(argv[i], "--out_j_size") == 0) {
      get_floats(argc, argv, &i, &out_j_size, 1);
      out_j_size_flag = TRUE;
    } else if (strcmp(argv[i], "-oks") == 0 ||
               strcmp(argv[i], "--out_k_size") == 0) {
      get_floats(argc, argv, &i, &out_k_size, 1);
      out_k_size_flag = TRUE;
    } else if (strcmp(argv[i], "-iid") == 0 ||
               strcmp(argv[i], "--in_i_direction") == 0) {
      get_floats(argc, argv, &i, in_i_directions, 3);
      magnitude = sqrt(in_i_directions[0] * in_i_directions[0] +
                       in_i_directions[1] * in_i_directions[1] +
                       in_i_directions[2] * in_i_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_i_direction = (%g, %g, %g)\n",
                     Progname, in_i_directions[0], in_i_directions[1],
                     in_i_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_i_direction: (%g, %g, %g) -> ",
                    in_i_directions[0], in_i_directions[1], in_i_directions[2]);
        in_i_directions[0] = in_i_directions[0] / magnitude;
        in_i_directions[1] = in_i_directions[1] / magnitude;
        in_i_directions[2] = in_i_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", in_i_directions[0], in_i_directions[1],
                    in_i_directions[2]);
      }
      in_i_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ijd") == 0 ||
               strcmp(argv[i], "--in_j_direction") == 0) {
      get_floats(argc, argv, &i, in_j_directions, 3);
      magnitude = sqrt(in_j_directions[0] * in_j_directions[0] +
                       in_j_directions[1] * in_j_directions[1] +
                       in_j_directions[2] * in_j_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_j_direction = (%g, %g, %g)\n",
                     Progname, in_j_directions[0], in_j_directions[1],
                     in_j_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_j_direction: (%g, %g, %g) -> ",
                    in_j_directions[0], in_j_directions[1], in_j_directions[2]);
        in_j_directions[0] = in_j_directions[0] / magnitude;
        in_j_directions[1] = in_j_directions[1] / magnitude;
        in_j_directions[2] = in_j_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", in_j_directions[0], in_j_directions[1],
                    in_j_directions[2]);
      }
      in_j_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ikd") == 0 ||
               strcmp(argv[i], "--in_k_direction") == 0) {
      get_floats(argc, argv, &i, in_k_directions, 3);
      magnitude = sqrt(in_k_directions[0] * in_k_directions[0] +
                       in_k_directions[1] * in_k_directions[1] +
                       in_k_directions[2] * in_k_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_k_direction = (%g, %g, %g)\n",
                     Progname, in_k_directions[0], in_k_directions[1],
                     in_k_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_k_direction: (%g, %g, %g) -> ",
                    in_k_directions[0], in_k_directions[1], in_k_directions[2]);
        in_k_directions[0] = in_k_directions[0] / magnitude;
        in_k_directions[1] = in_k_directions[1] / magnitude;
        in_k_directions[2] = in_k_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", in_k_directions[0], in_k_directions[1],
                    in_k_directions[2]);
      }
      in_k_direction_flag = TRUE;
    } else if (strcmp(argv[i], "--ctab") == 0) {
      get_string(argc, argv, &i, colortablefile);
    } else if (strcmp(argv[i], "--in_orientation") == 0) {
      get_string(argc, argv, &i, in_orientation_string);
      errmsg = MRIcheckOrientationString(in_orientation_string);
      if (errmsg != nullptr) {
        fmt::printf("ERROR: with in orientation string %s\n",
                    in_orientation_string);
        fmt::printf("%s\n", errmsg);
        exit(1);
      }
      in_orientation_flag = TRUE;
    }

    else if (strcmp(argv[i], "--out_orientation") == 0) {
      get_string(argc, argv, &i, out_orientation_string);
      errmsg = MRIcheckOrientationString(out_orientation_string);
      if (errmsg != nullptr) {
        fmt::printf("ERROR: with out_orientation string %s\n",
                    out_orientation_string);
        fmt::printf("%s\n", errmsg);
        exit(1);
      }
      out_orientation_flag = TRUE;
    }

    else if (strcmp(argv[i], "-oid") == 0 ||
             strcmp(argv[i], "--out_i_direction") == 0) {
      get_floats(argc, argv, &i, out_i_directions, 3);
      magnitude = sqrt(out_i_directions[0] * out_i_directions[0] +
                       out_i_directions[1] * out_i_directions[1] +
                       out_i_directions[2] * out_i_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_i_direction = (%g, %g, %g)\n",
                     Progname, out_i_directions[0], out_i_directions[1],
                     out_i_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_i_direction: (%g, %g, %g) -> ",
                    out_i_directions[0], out_i_directions[1],
                    out_i_directions[2]);
        out_i_directions[0] = out_i_directions[0] / magnitude;
        out_i_directions[1] = out_i_directions[1] / magnitude;
        out_i_directions[2] = out_i_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", out_i_directions[0], out_i_directions[1],
                    out_i_directions[2]);
      }
      out_i_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ojd") == 0 ||
               strcmp(argv[i], "--out_j_direction") == 0) {
      get_floats(argc, argv, &i, out_j_directions, 3);
      magnitude = sqrt(out_j_directions[0] * out_j_directions[0] +
                       out_j_directions[1] * out_j_directions[1] +
                       out_j_directions[2] * out_j_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_j_direction = (%g, %g, %g)\n",
                     Progname, out_j_directions[0], out_j_directions[1],
                     out_j_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_j_direction: (%g, %g, %g) -> ",
                    out_j_directions[0], out_j_directions[1],
                    out_j_directions[2]);
        out_j_directions[0] = out_j_directions[0] / magnitude;
        out_j_directions[1] = out_j_directions[1] / magnitude;
        out_j_directions[2] = out_j_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", out_j_directions[0], out_j_directions[1],
                    out_j_directions[2]);
      }
      out_j_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-okd") == 0 ||
               strcmp(argv[i], "--out_k_direction") == 0) {
      get_floats(argc, argv, &i, out_k_directions, 3);
      magnitude = sqrt(out_k_directions[0] * out_k_directions[0] +
                       out_k_directions[1] * out_k_directions[1] +
                       out_k_directions[2] * out_k_directions[2]);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_k_direction = (%g, %g, %g)\n",
                     Progname, out_k_directions[0], out_k_directions[1],
                     out_k_directions[2]);
        usage_message(stdout);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_k_direction: (%g, %g, %g) -> ",
                    out_k_directions[0], out_k_directions[1],
                    out_k_directions[2]);
        out_k_directions[0] = out_k_directions[0] / magnitude;
        out_k_directions[1] = out_k_directions[1] / magnitude;
        out_k_directions[2] = out_k_directions[2] / magnitude;
        fmt::printf("(%g, %g, %g)\n", out_k_directions[0], out_k_directions[1],
                    out_k_directions[2]);
      }
      out_k_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ic") == 0 ||
               strcmp(argv[i], "--in_center") == 0) {
      get_floats(argc, argv, &i, in_center, 3);
      in_center_flag = TRUE;
    } else if (strcmp(argv[i], "-dic") == 0 ||
               strcmp(argv[i], "--delta_in_center") == 0) {
      get_floats(argc, argv, &i, delta_in_center, 3);
      delta_in_center_flag = TRUE;
    } else if (strcmp(argv[i], "-oc") == 0 ||
               strcmp(argv[i], "--out_center") == 0) {
      get_floats(argc, argv, &i, out_center, 3);
      out_center_flag = TRUE;
    } else if (strcmp(argv[i], "-oni") == 0 || strcmp(argv[i], "-oic") == 0 ||
               strcmp(argv[i], "--out_i_count") == 0) {
      get_ints(argc, argv, &i, &out_n_i, 1);
      out_n_i_flag = TRUE;
    } else if (strcmp(argv[i], "-onj") == 0 || strcmp(argv[i], "-ojc") == 0 ||
               strcmp(argv[i], "--out_j_count") == 0) {
      get_ints(argc, argv, &i, &out_n_j, 1);
      out_n_j_flag = TRUE;
    } else if (strcmp(argv[i], "-onk") == 0 || strcmp(argv[i], "-okc") == 0 ||
               strcmp(argv[i], "--out_k_count") == 0) {
      get_ints(argc, argv, &i, &out_n_k, 1);
      out_n_k_flag = TRUE;
    } else if (strcmp(argv[i], "-vs") == 0 ||
               strcmp(argv[i], "--voxsize") == 0 ||
               strcmp(argv[i], "-voxsize") == 0) {
      get_floats(argc, argv, &i, voxel_size, 3);
      voxel_size_flag = TRUE;
    } else if (strcmp(argv[i], "-ds") == 0 ||
               strcmp(argv[i], "--downsample") == 0 ||
               strcmp(argv[i], "-downsample") == 0) {
      get_floats(argc, argv, &i, downsample_factor, 3);
      downsample_flag = TRUE;
    } else if (strcmp(argv[i], "-ds2") == 0 ||
               strcmp(argv[i], "--downsample2") == 0) {
      downsample2_flag = TRUE;
    } else if (strcmp(argv[i], "-ini") == 0 || strcmp(argv[i], "-iic") == 0 ||
               strcmp(argv[i], "--in_i_count") == 0) {
      get_ints(argc, argv, &i, &in_n_i, 1);
      in_n_i_flag = TRUE;
    } else if (strcmp(argv[i], "-inj") == 0 || strcmp(argv[i], "-ijc") == 0 ||
               strcmp(argv[i], "--in_j_count") == 0) {
      get_ints(argc, argv, &i, &in_n_j, 1);
      in_n_j_flag = TRUE;
    } else if (strcmp(argv[i], "-ink") == 0 || strcmp(argv[i], "-ikc") == 0 ||
               strcmp(argv[i], "--in_k_count") == 0) {
      get_ints(argc, argv, &i, &in_n_k, 1);
      in_n_k_flag = TRUE;
    } else if (strcmp(argv[i], "--fwhm") == 0) {
      get_floats(argc, argv, &i, &fwhm, 1);
      gstd = fwhm / sqrt(log(256.0));
      fmt::printf("fwhm = %g, gstd = %g\n", fwhm, gstd);
    } else if (strcmp(argv[i], "--reduce") == 0) {
      get_ints(argc, argv, &i, &reduce, 1);
      fmt::printf("reducing input image %d times\n", reduce);
    } else if (strcmp(argv[i], "-tr") == 0) {
      get_floats(argc, argv, &i, &in_tr, 1);
      in_tr_flag = TRUE;
    } else if (strcmp(argv[i], "-TI") == 0) {
      get_floats(argc, argv, &i, &in_ti, 1);
      in_ti_flag = TRUE;
    } else if (strcmp(argv[i], "-te") == 0) {
      get_floats(argc, argv, &i, &in_te, 1);
      in_te_flag = TRUE;
    } else if (strcmp(argv[i], "-flip_angle") == 0) {
      get_floats(argc, argv, &i, &in_flip_angle, 1);
      in_flip_angle_flag = TRUE;
    }

    else if (strcmp(argv[i], "-odt") == 0 ||
             strcmp(argv[i], "--out_data_type") == 0) {
      get_string(argc, argv, &i, out_data_type_string);
      if (strcmp(StrLower(out_data_type_string), "uchar") == 0) {
        out_data_type = MRI_UCHAR;
      } else if (strcmp(StrLower(out_data_type_string), "short") == 0) {
        out_data_type = MRI_SHORT;
      } else if (strcmp(StrLower(out_data_type_string), "int") == 0) {
        out_data_type = MRI_INT;
      } else if (strcmp(StrLower(out_data_type_string), "float") == 0) {
        out_data_type = MRI_FLOAT;
      } else if (strcmp(StrLower(out_data_type_string), "rgb") == 0) {
        out_data_type = MRI_RGB;
      } else {
        fmt::fprintf(stderr, "\n%s: unknown data type \"%s\"\n", Progname,
                     argv[i]);
        usage_message(stdout);
        exit(1);
      }
    } else if (strcmp(argv[i], "--bfile-little-endian") == 0) {
      setenv("BFILE_LITTLE_ENDIAN", "1", 1);
    } else if (strcmp(argv[i], "--rescale") == 0) {
      // Rescale so that the global mean of input is rescale_factor
      rescale_factor = atof(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-sc") == 0 || strcmp(argv[i], "--scale") == 0) {
      scale_factor = atof(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-osc") == 0 ||
               strcmp(argv[i], "--out-scale") == 0) {
      out_scale_factor = atof(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-rt") == 0 ||
               strcmp(argv[i], "--resample_type") == 0) {
      get_string(argc, argv, &i, resample_type);
      if (strcmp(StrLower(resample_type), "interpolate") == 0) {
        resample_type_val = SAMPLE_TRILINEAR;
      } else if (strcmp(StrLower(resample_type), "nearest") == 0) {
        resample_type_val = SAMPLE_NEAREST;
      } else if (strcmp(StrLower(resample_type), "vote") == 0) {
        resample_type_val = SAMPLE_VOTE;
      } else if (strcmp(StrLower(resample_type), "weighted") == 0) {
        resample_type_val = SAMPLE_WEIGHTED;
      }
      /*       else if(strcmp(StrLower(resample_type), "sinc") == 0)
            {
              resample_type_val = SAMPLE_SINC;
            }*/
      else if (strcmp(StrLower(resample_type), "cubic") == 0) {
        resample_type_val = SAMPLE_CUBIC_BSPLINE;
      } else {
        fmt::fprintf(stderr, "\n%s: unknown resample type \"%s\"\n", Progname,
                     argv[i]);
        usage_message(stdout);
        exit(1);
      }
    } else if (strcmp(argv[i], "-it") == 0 ||
               strcmp(argv[i], "--in_type") == 0) {
      get_string(argc, argv, &i, in_type_string.data());
      forced_in_type = string_to_type(in_type_string.data());
      force_in_type_flag = TRUE;
    } else if (strcmp(argv[i], "-dicomread2") == 0) {
      UseDICOMRead2 = 1;
    } else if (strcmp(argv[i], "-dicomread0") == 0) {
      UseDICOMRead2 = 0;
    } else if (strcmp(argv[i], "-ot") == 0 ||
               strcmp(argv[i], "--out_type") == 0) {
      get_string(argc, argv, &i, out_type_string);
      forced_out_type = string_to_type(out_type_string);
      /* see mri_identify.c */
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "-tt") == 0 ||
               strcmp(argv[i], "--template_type") == 0) {
      get_string(argc, argv, &i, template_type_string);
      forced_template_type = string_to_type(template_type_string);
      force_template_type_flag = TRUE;
    } else if (strcmp(argv[i], "-sn") == 0 ||
               strcmp(argv[i], "--subject_name") == 0) {
      get_string(argc, argv, &i, subject_name.data());
    } else if (strcmp(argv[i], "-gis") == 0 ||
               strcmp(argv[i], "--gdf_image_stem") == 0) {
      get_string(argc, argv, &i, gdf_image_stem.data());
      if (strlen(gdf_image_stem.data()) == 0) {
        fmt::fprintf(stderr, "\n%s: zero length GDF image stem given\n",
                     Progname);
        usage_message(stdout);
        exit(1);
      }
    } else if (strcmp(argv[i], "-rl") == 0 ||
               strcmp(argv[i], "--reslice_like") == 0) {
      get_string(argc, argv, &i, reslice_like_name);
      reslice_like_flag = TRUE;
    } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--frame") == 0) {
      nframes = 0;
      do {
        nframes++;
        if (i + 1 + nframes < argc &&
            (strncmp(argv[i + 1 + nframes], "-", 1) != 0)) {
          strtol(argv[i + 1 + nframes], &errormsg, 10);
        }
      } while (i + 1 + nframes < argc &&
               (strncmp(argv[i + 1 + nframes], "-", 1) != 0) &&
               *errormsg == '\0');

      get_ints(argc, argv, &i, frames, nframes);
      frame_flag = TRUE;
    } else if (strcmp(argv[i], "--erode-seg") == 0) {
      get_ints(argc, argv, &i, &n_erode_seg, 1);
      erode_seg_flag = TRUE;
    } else if (strcmp(argv[i], "--dil-seg") == 0) {
      get_ints(argc, argv, &i, &n_dil_seg, 1);
      dil_seg_flag = TRUE;
    } else if (strcmp(argv[i], "--dil-seg-mask") == 0) {
      get_string(argc, argv, &i, dil_seg_mask.data());
    } else if (strcmp(argv[i], "--cutends") == 0) {
      get_ints(argc, argv, &i, &ncutends, 1);
      cutends_flag = TRUE;
    } else if (strcmp(argv[i], "--slice-bias") == 0) {
      get_floats(argc, argv, &i, &SliceBiasAlpha, 1);
      SliceBias = TRUE;
    } else if (strcmp(argv[i], "--mid-frame") == 0) {
      frame_flag = TRUE;
      mid_frame_flag = TRUE;
    } else if (strcmp(argv[i], "--fsubsample") == 0) {
      get_ints(argc, argv, &i, &SubSampStart, 1);
      get_ints(argc, argv, &i, &SubSampDelta, 1);
      get_ints(argc, argv, &i, &SubSampEnd, 1);
      if (SubSampDelta == 0) {
        fmt::printf("ERROR: don't use subsample delta = 0\n");
        exit(1);
      }
      subsample_flag = TRUE;
    }

    else if (strcmp(argv[i], "-il") == 0 || strcmp(argv[i], "--in_like") == 0) {
      get_string(argc, argv, &i, in_like_name);
      in_like_flag = TRUE;
    } else if (strcmp(argv[i], "-roi") == 0 || strcmp(argv[i], "--roi") == 0) {
      roi_flag = TRUE;
    } else if (strcmp(argv[i], "-fp") == 0 ||
               strcmp(argv[i], "--fill_parcellation") == 0) {
      fill_parcellation_flag = TRUE;
    } else if (strcmp(argv[i], "-zo") == 0 ||
               strcmp(argv[i], "--zero_outlines") == 0) {
      zero_outlines_flag = TRUE;
    } else if (strcmp(argv[i], "-sp") == 0 ||
               strcmp(argv[i], "--smooth_parcellation") == 0) {
      get_ints(argc, argv, &i, &smooth_parcellation_count, 1);
      if (smooth_parcellation_count < 14 || smooth_parcellation_count > 26) {
        fmt::fprintf(stderr,
                     "\n%s: clean parcellation count must "
                     "be between 14 and 26, inclusive\n",
                     Progname);
        usage_message(stdout);
        exit(1);
      }
      smooth_parcellation_flag = TRUE;
    } else if (strcmp(argv[i], "-cf") == 0 ||
               strcmp(argv[i], "--color_file") == 0) {
      get_string(argc, argv, &i, color_file_name);
      color_file_flag = TRUE;
    } else if (strcmp(argv[i], "-nt") == 0 ||
               strcmp(argv[i], "--no_translate") == 0) {
      translate_labels_flag = FALSE;
    } else if (strcmp(argv[i], "-ns") == 0 ||
               strcmp(argv[i], "--no_scale") == 0) {
      get_ints(argc, argv, &i, &no_scale_flag, 1);
      // no_scale_flag = (no_scale_flag == 0 ? FALSE : TRUE);
    } else if (strcmp(argv[i], "-nth") == 0 ||
               strcmp(argv[i], "--nth_frame") == 0) {
      get_ints(argc, argv, &i, &nthframe, 1);
    } else if (strcmp(argv[i], "-cg") == 0 ||
               strcmp(argv[i], "--crop_gdf") == 0) {
      mriio_set_gdf_crop_flag(TRUE);
    } else if ((strcmp(argv[i], "-u") == 0) ||
               (strcmp(argv[i], "--usage") == 0) ||
               (strcmp(argv[i], "--help") == 0) ||
               (strcmp(argv[i], "-h") == 0)) {
      usage(stdout);
      exit(0);
    }
    /*-------------------------------------------------------------*/
    else if (strcmp(argv[i], "--status") == 0 ||
             strcmp(argv[i], "--statusfile") == 0) {
      /* File name to write percent complete for Siemens DICOM */
      if ((argc - 1) - i < 1) {
        fmt::fprintf(stderr, "ERROR: option --statusfile "
                             "requires one argument\n");
        exit(1);
      }
      i++;
      SDCMStatusFile =
          static_cast<char *>(calloc(strlen(argv[i]) + 1, sizeof(char)));
      memmove(SDCMStatusFile, argv[i], strlen(argv[i]));
      fptmp = fopen(SDCMStatusFile, "w");
      if (fptmp == nullptr) {
        fmt::fprintf(stderr, "ERROR: could not open %s for writing\n",
                     SDCMStatusFile);
        exit(1);
      }
      fmt::fprintf(fptmp, "0\n");
      fclose(fptmp);
    }
    /*-------------------------------------------------------------*/
    else if (strcmp(argv[i], "--sdcmlist") == 0) {
      /* File name that contains a list of Siemens DICOM files
         that are in the same run as the one listed on the
         command-line. If not present, the directory will be scanned,
         but this can take a while.
      */
      if ((argc - 1) - i < 1) {
        fmt::fprintf(stderr,
                     "ERROR: option --sdcmlist requires one argument\n");
        exit(1);
      }
      i++;
      SDCMListFile =
          static_cast<char *>(calloc(strlen(argv[i]) + 1, sizeof(char)));
      memmove(SDCMListFile, argv[i], strlen(argv[i]));
      fptmp = fopen(SDCMListFile, "r");
      if (fptmp == nullptr) {
        fmt::fprintf(stderr, "ERROR: could not open %s for reading\n",
                     SDCMListFile);
        exit(1);
      }
      fclose(fptmp);
    }
    /*-------------------------------------------------------------*/
    else if ((strcmp(argv[i], "--nslices-override") == 0)) {
      int NSlicesOverride;
      get_ints(argc, argv, &i, &NSlicesOverride, 1);
      fmt::printf("NSlicesOverride %d\n", NSlicesOverride);
      fmt::printf(tmpstr, "%d", NSlicesOverride);
      setenv("NSLICES_OVERRIDE", tmpstr, 1);
    }
    /*-------------------------------------------------------------*/
    else if ((strcmp(argv[i], "--ncols-override") == 0)) {
      int NColsOverride;
      get_ints(argc, argv, &i, &NColsOverride, 1);
      fmt::printf("NColsOverride %d\n", NColsOverride);
      fmt::printf(tmpstr, "%d", NColsOverride);
      setenv("NCOLS_OVERRIDE", tmpstr, 1);
    }
    /*-------------------------------------------------------------*/
    else if ((strcmp(argv[i], "--nrows-override") == 0)) {
      int NRowsOverride;
      get_ints(argc, argv, &i, &NRowsOverride, 1);
      fmt::printf("NRowsOverride %d\n", NRowsOverride);
      fmt::printf(tmpstr, "%d", NRowsOverride);
      setenv("NROWS_OVERRIDE", tmpstr, 1);
    } else if (strcmp(argv[i], "--mosaic-fix-noascii") == 0) {
      setenv("FS_MOSAIC_FIX_NOASCII", "1", 1);
    }
    /*-------------------------------------------------------------*/
    else if ((strcmp(argv[i], "--nspmzeropad") == 0) ||
             (strcmp(argv[i], "--out_nspmzeropad") == 0)) {
      /* Choose the amount of zero padding for spm output files */
      if ((argc - 1) - i < 1) {
        fmt::fprintf(stderr, "ERROR: option --out_nspmzeropad "
                             "requires one argument\n");
        exit(1);
      }
      i++;
      sscanf(argv[i], "%d", &N_Zero_Pad_Output);
    }
    /*-------------------------------------------------------------*/
    else if ((strcmp(argv[i], "--in_nspmzeropad") == 0)) {
      /* Choose the amount of zero padding for spm input files */
      if ((argc - 1) - i < 1) {
        fmt::fprintf(stderr, "ERROR: option --in_nspmzeropad "
                             "requires one argument\n");
        exit(1);
      }
      i++;
      sscanf(argv[i], "%d", &N_Zero_Pad_Input);
    } else if ((strcmp(argv[i], "--no-strip-pound") == 0)) {
      MRIIO_Strip_Pound = 0;
      /*-----------------------------------------------------*/ // E/
    } else if (strcmp(argv[i], "-zgez") == 0 ||
               strcmp(argv[i], "--zero_ge_z_offset") == 0) {
      zero_ge_z_offset_flag = TRUE;
    }
    /*-----------------------------------------------------*/ // E/
    else if (strcmp(argv[i], "-nozgez") == 0 ||
             strcmp(argv[i], "--no_zero_ge_z_offset") == 0) {
      zero_ge_z_offset_flag = FALSE;
    } else if (strcmp(argv[i], "--nskip") == 0) {
      get_ints(argc, argv, &i, &nskip, 1);
      fmt::printf("nskip = %d\n", nskip);
      if (nskip < 0) {
        fmt::printf("ERROR: nskip cannot be negative\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "--ndrop") == 0) {
      get_ints(argc, argv, &i, &ndrop, 1);
      fmt::printf("ndrop = %d\n", ndrop);
      if (ndrop < 0) {
        fmt::printf("ERROR: ndrop cannot be negative\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "--diag") == 0) {
      get_ints(argc, argv, &i, &Gdiag_no, 1);
    } else if (strcmp(argv[i], "--mra") == 0) {
      /* This flag forces DICOMread to first use 18,50 to get the slice
         thickness instead of 18,88. This is needed with siemens mag res
         angiogram (MRAs) */
      SliceResElTag1 = 0x50;
      SliceResElTag2 = 0x88;
    } else if (strcmp(argv[i], "--auto-slice-res") == 0) {
      /* Automatically determine whether to get slice thickness from 18,50 or
         18,88 depending upon  the value of 18,23 */
      AutoSliceResElTag = 1;
    }
    /*-------------------------------------------------------------*/
    else {
      if (argv[i][0] == '-') {
        fmt::fprintf(stderr, "\n%s: unknown flag \"%s\"\n", Progname, argv[i]);
        usage_message(stdout);
        exit(1);
      } else {
        if (in_name[0] == '\0') {
          strcpy(in_name.data(), argv[i]);
        } else if (out_name[0] == '\0') {
          strcpy(out_name.data(), argv[i]);
        } else {
          if (i + 1 == argc) {
            fmt::fprintf(stderr, "\n%s: extra argument (\"%s\")\n", Progname,
                         argv[i]);
          } else {
            fmt::fprintf(stderr,
                         "\n%s: extra arguments (\"%s\" and "
                         "following)\n",
                         Progname, argv[i]);
          }
          usage_message(stdout);
          exit(1);
        }
      }
    }
  }
  /**** Finished parsing command line ****/
  /* option inconsistency checks */
  if ((force_ras_good != 0) &&
      ((in_i_direction_flag != 0) || (in_j_direction_flag != 0) ||
       (in_k_direction_flag != 0))) {
    fmt::fprintf(stderr, "ERROR: cannot use --force_ras_good and "
                         "--in_?_direction_flag\n");
    exit(1);
  }
  if (conform_flag == FALSE && conform_min == TRUE) {
    fmt::fprintf(stderr, "In order to use -cm (--conform_min), "
                         "you must set -c (--conform)  at the same time.\n");
    exit(1);
  }

  /* ----- catch zero or negative voxel dimensions ----- */

  sizes_good_flag = TRUE;

  if (((in_i_size_flag != 0) && in_i_size <= 0.0) ||
      ((in_j_size_flag != 0) && in_j_size <= 0.0) ||
      ((in_k_size_flag != 0) && in_k_size <= 0.0) ||
      ((out_i_size_flag != 0) && out_i_size <= 0.0) ||
      ((out_j_size_flag != 0) && out_j_size <= 0.0) ||
      ((out_k_size_flag != 0) && out_k_size <= 0.0)) {
    fmt::fprintf(stderr,
                 "\n%s: voxel sizes must be "
                 "greater than zero\n",
                 Progname);
    sizes_good_flag = FALSE;
  }

  if ((in_i_size_flag != 0) && in_i_size <= 0.0) {
    fmt::fprintf(stderr, "in i size = %g\n", in_i_size);
  }
  if ((in_j_size_flag != 0) && in_j_size <= 0.0) {
    fmt::fprintf(stderr, "in j size = %g\n", in_j_size);
  }
  if ((in_k_size_flag != 0) && in_k_size <= 0.0) {
    fmt::fprintf(stderr, "in k size = %g\n", in_k_size);
  }
  if ((out_i_size_flag != 0) && out_i_size <= 0.0) {
    fmt::fprintf(stderr, "out i size = %g\n", out_i_size);
  }
  if ((out_j_size_flag != 0) && out_j_size <= 0.0) {
    fmt::fprintf(stderr, "out j size = %g\n", out_j_size);
  }
  if ((out_k_size_flag != 0) && out_k_size <= 0.0) {
    fmt::fprintf(stderr, "out k size = %g\n", out_k_size);
  }

  if (sizes_good_flag == 0) {
    usage_message(stdout);
    exit(1);
  }

  /* ----- catch missing input or output volume name ----- */
  if (in_name[0] == '\0') {
    fmt::fprintf(stderr, "\n%s: missing input volume name\n", Progname);
    usage_message(stdout);
    exit(1);
  }

  if (out_name[0] == '\0' && !((read_only_flag != 0) || (no_write_flag != 0))) {
    fmt::fprintf(stderr, "\n%s: missing output volume name\n", Progname);
    usage_message(stdout);
    exit(1);
  }

  /* ---- catch no --like flag for OutStatTableFlag ----- */
  if ((OutStatTableFlag != 0) && out_like_flag == 0) {
    fmt::fprintf(
        stderr,
        "\n%s: pass example (or empty) stats-table with --like to specify "
        "measure, column and row headers\n",
        Progname);
    usage_message(stdout);
    exit(1);
  }

  /* ----- copy file name (only -- strip '@' and '#') ----- */
  MRIgetVolumeName(in_name.data(), in_name_only);

  /* If input type is spm and N_Zero_Pad_Input < 0, set to 3*/
  if ((strcmp(in_type_string.data(), "spm") == 0) && N_Zero_Pad_Input < 0) {
    N_Zero_Pad_Input = 3;
  }

  /* ----- catch unknown volume types ----- */
  if ((force_in_type_flag != 0) && forced_in_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr,
                 "\n%s: unknown input "
                 "volume type %s\n",
                 Progname, in_type_string.data());
    usage_message(stdout);
    exit(1);
  }

  if ((force_template_type_flag != 0) &&
      forced_template_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr, "\n%s: unknown template volume type %s\n", Progname,
                 template_type_string);
    usage_message(stdout);
    exit(1);
  }

  if ((force_out_type_flag != 0) &&
      forced_out_type == MRI_VOLUME_TYPE_UNKNOWN && (ascii_flag == 0)) {
    fmt::fprintf(stderr, "\n%s: unknown output volume type %s\n", Progname,
                 out_type_string);
    usage_message(stdout);
    exit(1);
  }

  /* ----- warn if read only is desired and an
     output volume is specified or the output info flag is set ----- */
  if ((read_only_flag != 0) && out_name[0] != '\0') {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "nothing will be written to %s\n",
                 Progname, out_name.data());
  }
  if ((read_only_flag != 0) &&
      ((out_info_flag != 0) || (out_matrix_flag != 0))) {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "no output information will be printed\n",
                 Progname);
  }

  /* ----- get the type of the output ----- */
  if (read_only_flag == 0) {
    if ((force_out_type_flag == 0) && (OutStatTableFlag == 0)) {
      // if(!read_only_flag && !no_write_flag)
      // because conform_flag value changes depending on type below
      {
        out_volume_type = mri_identify(out_name.data());
        if (out_volume_type == MRI_VOLUME_TYPE_UNKNOWN) {
          fmt::fprintf(stderr, "%s: can't determine type of output volume\n",
                       Progname);
          exit(1);
        }
      }
    } else {
      out_volume_type = forced_out_type;
    }

    // if output type is COR, then it is always conformed
    if (out_volume_type == MRI_CORONAL_SLICE_DIRECTORY) {
      conform_flag = TRUE;
    }
  }

  /* ----- catch the parse-only flag ----- */
  if (parse_only_flag != 0) {
    fmt::printf("input volume name: %s\n", in_name.data());
    fmt::printf("input name only: %s\n", in_name_only);
    fmt::printf("output volume name: %s\n", out_name.data());
    fmt::printf("parse_only_flag = %d\n", parse_only_flag);
    fmt::printf("conform_flag = %d\n", conform_flag);
    fmt::printf("conform_size = %f\n", conform_size);
    fmt::printf("in_info_flag = %d\n", in_info_flag);
    fmt::printf("out_info_flag = %d\n", out_info_flag);
    fmt::printf("in_matrix_flag = %d\n", in_matrix_flag);
    fmt::printf("out_matrix_flag = %d\n", out_matrix_flag);

    if (force_in_type_flag != 0) {
      fmt::printf("input type is %d\n", forced_in_type);
    }
    if (force_out_type_flag != 0) {
      fmt::printf("output type is %d\n", forced_out_type);
    }

    if (subject_name[0] != '\0') {
      fmt::printf("subject name is %s\n", subject_name.data());
    }

    if (invert_val >= 0) {
      fmt::printf("inversion, value is %g\n", invert_val);
    }

    if (reorder_flag != 0) {
      fmt::printf("reordering, values are %d %d %d\n", reorder_vals[0],
                  reorder_vals[1], reorder_vals[2]);
    }

    if (reorder4_flag != 0) {
      fmt::printf("reordering, values are %d %d %d %d\n", reorder4_vals[0],
                  reorder4_vals[1], reorder4_vals[2], reorder4_vals[3]);
    }

    fmt::printf("translation of otl labels is %s\n",
                translate_labels_flag != 0 ? "on" : "off");

    exit(0);
  }

  /* ----- check for a gdf image stem if the output type is gdf ----- */
  if (out_volume_type == GDF_FILE && strlen(gdf_image_stem.data()) == 0) {
    fmt::fprintf(stderr,
                 "%s: GDF output type, "
                 "but no GDF image file stem\n",
                 Progname);
    exit(1);
  }

  /* ----- read the in_like volume ----- */
  if (in_like_flag != 0) {
    fmt::printf("reading info from %s...\n", in_like_name);
    mri_in_like = MRIreadInfo(in_like_name);
    if (mri_in_like == nullptr) {
      exit(1);
    }
  }

  /* ----- read the volume ----- */
  in_volume_type = MRI_VOLUME_TYPE_UNKNOWN;
  if (InStatTableFlag == 0) {
    if (force_in_type_flag != 0) {
      in_volume_type = forced_in_type;
    } else {
      in_volume_type = mri_identify(in_name_only);
    }
    if (in_volume_type == MRI_VOLUME_TYPE_UNKNOWN) {
      errno = 0;
      if (fio_FileExistsReadable(in_name_only) == 0) {
        fmt::printf("ERROR: file %s does not exist\n", in_name_only);
      } else {
        fmt::printf("ERROR: cannot determine file type for %s \n",
                    in_name_only);
      }
      if (in_like_flag != 0) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }
  }

  if ((roi_flag != 0) && in_volume_type != GENESIS_FILE) {
    errno = 0;
    ErrorPrintf(ERROR_BADPARM, "rois must be in GE format");
    if (in_like_flag != 0) {
      MRIfree(&mri_in_like);
    }
    exit(1);
  }

  if ((zero_ge_z_offset_flag != 0) && in_volume_type != DICOM_FILE) // E/
  {
    zero_ge_z_offset_flag = FALSE;
    fmt::fprintf(stderr, "Not a GE dicom volume: -zgez "
                         "= --zero_ge_z_offset option ignored.\n");
  }

  fmt::printf("$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $\n");
  fmt::printf("reading from %s...\n", in_name_only);

  if (in_volume_type == MGH_MORPH) {
    GCA_MORPH *gcam;
    GCA_MORPH *gcam_out;
    gcam = GCAMread(in_name_only);
    if (gcam == nullptr) {
      ErrorExit(ERROR_NOFILE, "%s: could not read input morph from %s",
                Progname, in_name_only);
    }
    if (downsample2_flag != 0) {
      gcam_out = GCAMdownsample2(gcam);
    } else {
      gcam_out = gcam;
    }

    GCAMwrite(gcam_out, out_name.data());
    exit(0);
  }

  if (in_volume_type == OTL_FILE) {

    if ((in_like_flag == 0) && (in_n_k_flag == 0)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM,
                  "parcellation read: must specify"
                  "a volume depth with either in_like or in_k_count");
      exit(1);
    }

    if (color_file_flag == 0) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "parcellation read: must specify a"
                                 "color file name");
      if (in_like_flag != 0) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    read_parcellation_volume_flag = TRUE;
    if ((read_only_flag != 0) &&
        ((in_info_flag != 0) || (in_matrix_flag != 0)) &&
        (in_stats_flag == 0)) {
      read_parcellation_volume_flag = FALSE;
    }

    read_otl_flags = 0x00;

    if (read_parcellation_volume_flag != 0) {
      read_otl_flags |= READ_OTL_READ_VOLUME_FLAG;
    }

    if (fill_parcellation_flag != 0) {
      read_otl_flags |= READ_OTL_FILL_FLAG;
    } else {
      fmt::printf("notice: unfilled parcellations "
                  "currently unimplemented\n");
      fmt::printf("notice: filling outlines\n");
      read_otl_flags |= READ_OTL_FILL_FLAG;
    }

    if (translate_labels_flag != 0) {
      read_otl_flags |= READ_OTL_TRANSLATE_LABELS_FLAG;
    }

    if (zero_outlines_flag != 0) {
      read_otl_flags |= READ_OTL_ZERO_OUTLINES_FLAG;
    }

    if (in_like_flag != 0) {
      mri = MRIreadOtl(in_name.data(), mri_in_like->width, mri_in_like->height,
                       mri_in_like->depth, color_file_name, read_otl_flags);
    } else {
      mri = MRIreadOtl(in_name.data(), 0, 0, in_n_k, color_file_name,
                       read_otl_flags);
    }

    if (mri == nullptr) {
      if (in_like_flag != 0) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

#if 0
    if (out_volume_type == MGH_MORPH)
    {
      GCA_MORPH *gcam ;

      if (mri->nframes != 3)
	ErrorExit(ERROR_UNSUPPORTED, "%s: input volume must have 3 frames not %d to write to warp", Progname, mri->nframes) ;
      gcam = GCAMalloc(mri->width, mri->height, mri->depth) ;
      GCAMinit(gcam, mri, NULL, NULL, 0) ;
      GCAMreadWarpFromMRI( gcam, mri );
      GCAMwrite(gcam, out_name) ;
      exit(0) ;
    }
#endif

    /* ----- smooth the parcellation if requested ----- */
    if (smooth_parcellation_flag != 0) {
      fmt::printf("smoothing parcellation...\n");
      mri2 = MRIsmoothParcellation(mri, smooth_parcellation_count);
      if (mri2 == nullptr) {
        if (in_like_flag != 0) {
          MRIfree(&mri_in_like);
        }
        exit(1);
      }
      MRIfree(&mri);
      mri = mri2;
    }

    resample_type_val = SAMPLE_NEAREST;
    no_scale_flag = TRUE;

  } else if (roi_flag != 0) {
    if ((in_like_flag == 0) && (in_n_k_flag == 0)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "roi read: must specify a volume"
                                 "depth with either in_like or in_k_count");
      if (in_like_flag != 0) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    if (in_like_flag != 0) {
      mri = MRIreadGeRoi(in_name.data(), mri_in_like->depth);
    } else {
      mri = MRIreadGeRoi(in_name.data(), in_n_k);
    }

    if (mri == nullptr) {
      if (in_like_flag != 0) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    resample_type_val = SAMPLE_NEAREST;
    no_scale_flag = TRUE;
  } else {
    if ((read_only_flag != 0) &&
        ((in_info_flag != 0) || (in_matrix_flag != 0)) &&
        (in_stats_flag == 0)) {
      if (force_in_type_flag != 0) {
        mri = MRIreadHeader(in_name.data(), in_volume_type);
      } else {
        mri = MRIreadInfo(in_name.data());
      }
    } else {
      if (force_in_type_flag != 0) {
        // fmt::printf("MRIreadType()\n");
        mri = MRIreadType(in_name.data(), in_volume_type);
      } else {
        if (nthframe < 0) {
          if (InStatTableFlag == 0) {
            mri = MRIread(in_name.data());
          } else {
            fmt::printf("Loading in stat table %s\n", in_name.data());
            StatTable = LoadStatTable(in_name.data());
            if (StatTable == nullptr) {
              fmt::printf("ERROR: loading y %s as a stat table\n",
                          in_name.data());
              exit(1);
            }
            mri = StatTable->mri;
          }
        } else {
          mri = MRIreadEx(in_name.data(), nthframe);
        }
      }
    }
  }

  if (mri == nullptr) {
    if (in_like_flag != 0) {
      MRIfree(&mri_in_like);
    }
    exit(1);
  }

  if (outside_val > 0) {
    mri->outside_val = outside_val;
  }
  if (UpsampleFlag != 0) {
    fmt::printf("UpsampleFactor = %d\n", UpsampleFactor);
    mritmp = MRIupsampleN(mri, nullptr, UpsampleFactor);
    MRIfree(&mri);
    mri = mritmp;
  }

  if (slice_crop_flag != 0) {
    fmt::printf("Cropping slices from %d to %d\n", slice_crop_start,
                slice_crop_stop);
    mri2 = MRIcrop(mri, 0, 0, slice_crop_start, mri->width - 1, mri->height - 1,
                   slice_crop_stop);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (LeftRightReverse != 0) {
    // Performs a left-right reversal of the geometry by finding the
    // dimension that is most left-right oriented and negating its
    // column in the vox2ras matrix. The pixel data itself is not
    // changed. It would also have worked to have negated the first
    // row in the vox2ras, but negating a column is the header
    // equivalent to reversing the pixel data in a dimension. This
    // also adjusts the CRAS so that if the pixel data are
    // subsequently reversed then a header registration will bring
    // the new and the original volume into perfect registration.
    fmt::printf("WARNING: applying left-right reversal to the input geometry\n"
                "without reversing the pixel data. This will likely make \n"
                "the volume geometry WRONG, so make sure you know what you  \n"
                "are doing.\n");

    fmt::printf("  CRAS Before: %g %g %g\n", mri->c_r, mri->c_a, mri->c_s);
    T = MRIxfmCRS2XYZ(mri, 0);
    vmid = MatrixAlloc(4, 1, MATRIX_REAL);
    vmid->rptr[4][1] = 1;

    MRIdircosToOrientationString(mri, ostr);
    if (ostr[0] == 'L' || ostr[0] == 'R') {
      fmt::printf("  Reversing geometry for the columns\n");
      vmid->rptr[1][1] = mri->width / 2.0 - 1;
      vmid->rptr[2][1] = mri->height / 2.0;
      vmid->rptr[3][1] = mri->depth / 2.0;
      cras = MatrixMultiply(T, vmid, NULL);
      mri->x_r *= -1.0;
      mri->x_a *= -1.0;
      mri->x_s *= -1.0;
    }
    if (ostr[1] == 'L' || ostr[1] == 'R') {
      fmt::printf("  Reversing geometry for the rows\n");
      vmid->rptr[1][1] = mri->width / 2.0;
      vmid->rptr[2][1] = mri->height / 2.0 - 1;
      vmid->rptr[3][1] = mri->depth / 2.0;
      cras = MatrixMultiply(T, vmid, NULL);
      mri->y_r *= -1.0;
      mri->y_a *= -1.0;
      mri->y_s *= -1.0;
    }
    if (ostr[2] == 'L' || ostr[2] == 'R') {
      fmt::printf("  Reversing geometry for the slices\n");
      vmid->rptr[1][1] = mri->width / 2.0;
      vmid->rptr[2][1] = mri->height / 2.0;
      vmid->rptr[3][1] = mri->depth / 2.0 - 1;
      cras = MatrixMultiply(T, vmid, NULL);
      mri->z_r *= -1.0;
      mri->z_a *= -1.0;
      mri->z_s *= -1.0;
    }
    mri->c_r = cras->rptr[1][1];
    mri->c_a = cras->rptr[2][1];
    mri->c_s = cras->rptr[3][1];
    fmt::printf("  CRAS After %g %g %g\n", mri->c_r, mri->c_a, mri->c_s);
    MatrixFree(&T);
    MatrixFree(&vmid);
    MatrixFree(&cras);
  }

  if (LeftRightSwapLabel != 0) {
    fmt::printf("Performing left-right swap of labels\n");
    // Good for aseg, aparc+aseg, wmparc, etc. Does not change geometry
    mri2 = MRIlrswapAseg(mri);
    MRIfree(&mri);
    mri = mri2;
  }

  if (LeftRightReversePix != 0) {
    // Performs a left-right reversal of the pixels by finding the
    // dimension that is most left-right oriented and reversing
    // the order of the pixels. The geometry itself is not
    // changed.
    fmt::printf("WARNING: applying left-right reversal to the input pixels\n"
                "without changing geometry. This will likely make \n"
                "the volume geometry WRONG, so make sure you know what you  \n"
                "are doing.\n");

    MRIdircosToOrientationString(mri, ostr);
    mri2 = MRIcopy(mri, nullptr);
    if (ostr[0] == 'L' || ostr[0] == 'R') {
      fmt::printf("  Reversing pixels for the columns\n");
      for (c = 0; c < mri->width; c++) {
        c2 = mri->width - c - 1;
        for (r = 0; r < mri->height; r++) {
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              v = MRIgetVoxVal(mri, c, r, s, f);
              MRIsetVoxVal(mri2, c2, r, s, f, v);
            }
          }
        }
      }
    }
    if (ostr[1] == 'L' || ostr[1] == 'R') {
      fmt::printf("  Reversing pixels for the rows\n");
      for (c = 0; c < mri->width; c++) {
        for (r = 0; r < mri->height; r++) {
          r2 = mri->height - r - 1;
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              v = MRIgetVoxVal(mri, c, r, s, f);
              MRIsetVoxVal(mri2, c, r2, s, f, v);
            }
          }
        }
      }
    }
    if (ostr[2] == 'L' || ostr[2] == 'R') {
      fmt::printf("  Reversing pixels for the slices\n");
      for (c = 0; c < mri->width; c++) {
        for (r = 0; r < mri->height; r++) {
          for (s = 0; s < mri->depth; s++) {
            s2 = mri->depth - s - 1;
            for (f = 0; f < mri->nframes; f++) {
              v = MRIgetVoxVal(mri, c, r, s, f);
              MRIsetVoxVal(mri2, c, r, s2, f, v);
            }
          }
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if ((LeftRightMirrorFlag != 0) || (LeftRightKeepFlag != 0)) //(mr, 2012-03-08)
  {
    // Mirror one half of the image into the other half (left-right)
    // the split point is at the middle of the dimension that is
    // most left-right oriented. The header geometry is not changed
    // If LeftRightKeep, then don't mirror but fill with zero and
    // only keep the specified hemisphere
    fmt::printf("WARNING: Mirroring %s half into the other half,\n"
                "or masking one half of the image is only meaningful if\n"
                "the image is upright and centerd, see make_upright.\n",
                LeftRightMirrorHemi);

    MRIdircosToOrientationString(mri, ostr);
    fmt::printf("  Orientation string: %s\n", ostr);
    mri2 = MRIcopy(mri, nullptr);
    v = 0;
    if (ostr[0] == 'L' || ostr[0] == 'R') {
      fmt::printf("  Mirror or keep pixels for the columns\n");
      for (c = 0; c < mri->width / 2; c++) {
        c1 = c;
        if (ostr[0] == toupper(LeftRightMirrorHemi[0])) {
          c1 = mri->width - c - 1;
        }
        c2 = mri->width - c1 - 1;
        for (r = 0; r < mri->height; r++) {
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              if (LeftRightMirrorFlag != 0) {
                v = MRIgetVoxVal(mri, c1, r, s, f);
              }
              MRIsetVoxVal(mri2, c2, r, s, f, v);
            }
          }
        }
      }
    }
    if (ostr[1] == 'L' || ostr[1] == 'R') {
      fmt::printf("  Mirror or keep pixels for the rows\n");
      for (c = 0; c < mri->width; c++) {
        for (r = 0; r < mri->height / 2; r++) {
          r1 = r;
          if (ostr[1] == toupper(LeftRightMirrorHemi[0])) {
            r1 = mri->height - r - 1;
          }
          r2 = mri->height - r1 - 1;
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              if (LeftRightMirrorFlag != 0) {
                v = MRIgetVoxVal(mri, c, r1, s, f);
              }
              MRIsetVoxVal(mri2, c, r2, s, f, v);
            }
          }
        }
      }
    }
    if (ostr[2] == 'L' || ostr[2] == 'R') {
      fmt::printf("  Mirror or keep pixels for the slices\n");
      for (c = 0; c < mri->width; c++) {
        for (r = 0; r < mri->height; r++) {
          for (s = 0; s < mri->depth / 2; s++) {
            s1 = s;
            if (ostr[2] == toupper(LeftRightMirrorHemi[0])) {
              s1 = mri->depth - s - 1;
            }
            s2 = mri->depth - s1 - 1;
            for (f = 0; f < mri->nframes; f++) {
              if (LeftRightMirrorFlag != 0) {
                v = MRIgetVoxVal(mri, c, r, s1, f);
              }
              MRIsetVoxVal(mri2, c, r, s2, f, v);
            }
          }
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (FlipCols != 0) {
    // Reverses the columns WITHOUT changing the geometry in the
    // header.  Know what you are going here.
    fmt::printf("WARNING: flipping cols without changing geometry\n");
    mri2 = MRIcopy(mri, nullptr);
    fmt::printf("type %d %d\n", mri->type, mri2->type);
    for (c = 0; c < mri->width; c++) {
      c2 = mri->width - c - 1;
      for (r = 0; r < mri->height; r++) {
        for (s = 0; s < mri->depth; s++) {
          for (f = 0; f < mri->nframes; f++) {
            v = MRIgetVoxVal(mri, c, r, s, f);
            MRIsetVoxVal(mri2, c2, r, s, f, v);
          }
        }
      }
    }
    fmt::printf("type %d %d\n", mri->type, mri2->type);
    MRIfree(&mri);
    mri = mri2;
  }

  if (SliceReverse != 0) {
    fmt::printf("Reversing slices, updating vox2ras\n");
    mri2 = MRIreverseSlices(mri, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (SliceBias != 0) {
    fmt::printf("Applying Half-Cosine Slice Bias, Alpha = %g\n",
                SliceBiasAlpha);
    MRIhalfCosBias(mri, SliceBiasAlpha, mri);
  }

  if (reduce > 0) {
    MRI *mri_tmp;
    int r;

    if (mri->depth == 1) {
      for (r = 0; r < reduce; r++) {
        mri_tmp = MRIreduce2D(mri, nullptr);
        MRIfree(&mri);
        mri = mri_tmp;
      }
    } else {
      for (r = 0; r < reduce; r++) {
        mri_tmp = MRIreduce(mri, nullptr);
        MRIfree(&mri);
        mri = mri_tmp;
      }
    }
  }

  if (fwhm > 0) {
    fmt::printf("Smoothing input at fwhm = %g, gstd = %g\n", fwhm, gstd);
    MRIgaussianSmooth(mri, gstd, 1, mri);
  }

  if (!FEQUAL(rescale_factor, 1.0)) {
    // Rescale so that the global mean of input is rescale_factor
    double globalmean = 0;
    for (c = 0; c < mri->width; c++) {
      for (r = 0; r < mri->height; r++) {
        for (s = 0; s < mri->depth; s++) {
          for (f = 0; f < mri->nframes; f++) {
            v = MRIgetVoxVal(mri, c, r, s, f);
            globalmean += v;
          }
        }
      }
    }
    globalmean /= static_cast<double>(mri->width * mri->height * mri->depth *
                                      mri->nframes);
    fmt::printf("Global rescaling input mean from %g to %g\n", globalmean,
                rescale_factor);
    MRIscalarMul(mri, mri, rescale_factor / globalmean);
  }

  MRIaddCommandLine(mri, cmdline);
  if (!FEQUAL(scale_factor, 1.0)) {
    fmt::printf("scaling input volume by %2.3f\n", scale_factor);
    MRIscalarMul(mri, mri, scale_factor);
  }

  if (zero_ge_z_offset_flag != 0) // E/
  {
    mri->c_s = 0.0;
  }

  fmt::printf("TR=%2.2f, TE=%2.2f, TI=%2.2f, flip angle=%2.2f\n", mri->tr,
              mri->te, mri->ti, DEGREES(mri->flip_angle));
  if (in_volume_type != OTL_FILE) {
    if (fill_parcellation_flag != 0) {
      fmt::printf("fill_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
    if (smooth_parcellation_flag != 0) {
      fmt::printf("smooth_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
  }

  /* ----- apply the in_like volume if it's been read ----- */
  if (in_like_flag != 0) {
    if (mri->width != mri_in_like->width ||
        mri->height != mri_in_like->height ||
        mri->depth != mri_in_like->depth) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "volume sizes do not match\n");
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  in_name.data(), mri->width, mri->height, mri->depth,
                  mri->nframes);
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  in_like_name, mri_in_like->width, mri_in_like->height,
                  mri_in_like->depth, mri_in_like->nframes);
      MRIfree(&mri);
      MRIfree(&mri_in_like);
      exit(1);
    }
    if (mri->nframes != mri_in_like->nframes) {
      printf("INFO: frames are not the same\n");
    }
    temp_type = mri->type;

    mritmp = MRIcopyHeader(mri_in_like, mri);
    if (mritmp == nullptr) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM,
                  "error copying information from "
                  "%s structure to %s structure\n",
                  in_like_name, in_name.data());
      MRIfree(&mri);
      MRIfree(&mri_in_like);
      exit(1);
    }

    mri->type = temp_type;
    MRIfree(&mri_in_like);
  }

  if (mri->ras_good_flag == 0) {
    printf("WARNING: it does not appear that there "
           "was sufficient information\n"
           "in the input to assign orientation to the volume... \n");
    if (force_ras_good != 0) {
      printf("However, you have specified that the "
             "default orientation should\n"
             "be used with by adding --force_ras_good "
             "on the command-line.\n");
      mri->ras_good_flag = 1;
    }
    if ((in_i_direction_flag != 0) || (in_j_direction_flag != 0) ||
        (in_k_direction_flag != 0)) {
      printf("However, you have specified one or more "
             "orientations on the \n"
             "command-line using -i?d or --in-?-direction (?=i,j,k).\n");
      mri->ras_good_flag = 1;
    }
  }

  /* ----- apply command-line parameters ----- */
  if (in_i_size_flag != 0) {
    mri->xsize = in_i_size;
  }
  if (in_j_size_flag != 0) {
    mri->ysize = in_j_size;
  }
  if (in_k_size_flag != 0) {
    mri->zsize = in_k_size;
  }
  if (in_i_direction_flag != 0) {
    mri->x_r = in_i_directions[0];
    mri->x_a = in_i_directions[1];
    mri->x_s = in_i_directions[2];
    mri->ras_good_flag = 1;
  }
  if (in_j_direction_flag != 0) {
    mri->y_r = in_j_directions[0];
    mri->y_a = in_j_directions[1];
    mri->y_s = in_j_directions[2];
    mri->ras_good_flag = 1;
  }
  if (in_k_direction_flag != 0) {
    mri->z_r = in_k_directions[0];
    mri->z_a = in_k_directions[1];
    mri->z_s = in_k_directions[2];
    mri->ras_good_flag = 1;
  }
  if (DeleteCMDs != 0) {
    mri->ncmds = 0;
  }
  if (DoNewTransformFname != 0) {
    printf("Changing xform name to %s\n", NewTransformFname);
    strcpy(mri->transform_fname, NewTransformFname);
  }
  if (in_orientation_flag != 0) {
    printf("Setting input orientation to %s\n", in_orientation_string);
    MRIorientationStringToDircos(mri, in_orientation_string);
    mri->ras_good_flag = 1;
  }
  if (in_center_flag != 0) {
    mri->c_r = in_center[0];
    mri->c_a = in_center[1];
    mri->c_s = in_center[2];
  }
  if (delta_in_center_flag != 0) {
    mri->c_r += delta_in_center[0];
    mri->c_a += delta_in_center[1];
    mri->c_s += delta_in_center[2];
  }
  if (subject_name[0] != '\0') {
    strcpy(mri->subject_name, subject_name.data());
  }

  if (in_tr_flag != 0) {
    mri->tr = in_tr;
  }
  if (in_ti_flag != 0) {
    mri->ti = in_ti;
  }
  if (in_te_flag != 0) {
    mri->te = in_te;
  }
  if (in_flip_angle_flag != 0) {
    mri->flip_angle = in_flip_angle;
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((in_i_size_flag != 0) || (in_j_size_flag != 0) || (in_k_size_flag != 0)) {

    fov_x = mri->xsize * mri->width;
    fov_y = mri->ysize * mri->height;
    fov_z = mri->zsize * mri->depth;

    mri->xend = fov_x / 2.0;
    mri->xstart = -mri->xend;
    mri->yend = fov_y / 2.0;
    mri->ystart = -mri->yend;
    mri->zend = fov_z / 2.0;
    mri->zstart = -mri->zend;

    mri->fov = (fov_x > fov_y ? (fov_x > fov_z ? fov_x : fov_z)
                              : (fov_y > fov_z ? fov_y : fov_z));
  }

  /* ----- give a warning for non-orthogonal directions ----- */
  i_dot_j = mri->x_r * mri->y_r + mri->x_a * mri->y_a + mri->x_s * mri->y_s;
  i_dot_k = mri->x_r * mri->z_r + mri->x_a * mri->z_a + mri->x_s * mri->z_s;
  j_dot_k = mri->y_r * mri->z_r + mri->y_a * mri->z_a + mri->y_s * mri->z_s;
  if (fabs(i_dot_j) > CLOSE_ENOUGH || fabs(i_dot_k) > CLOSE_ENOUGH ||
      fabs(i_dot_k) > CLOSE_ENOUGH) {
    printf("warning: input volume axes are not orthogonal:"
           "i_dot_j = %.6f, i_dot_k = %.6f, j_dot_k = %.6f\n",
           i_dot_j, i_dot_k, j_dot_k);
  }
  printf("i_ras = (%g, %g, %g)\n", mri->x_r, mri->x_a, mri->x_s);
  printf("j_ras = (%g, %g, %g)\n", mri->y_r, mri->y_a, mri->y_s);
  printf("k_ras = (%g, %g, %g)\n", mri->z_r, mri->z_a, mri->z_s);

  /* ----- catch the in info flag ----- */
  if (in_info_flag != 0) {
    printf("input structure:\n");
    MRIdump(mri, stdout);
  }

  if (in_matrix_flag != 0) {
    MATRIX *i_to_r;
    i_to_r = extract_i_to_r(mri);
    if (i_to_r != nullptr) {
      printf("input ijk -> ras:\n");
      MatrixPrint(stdout, i_to_r);
      MatrixFree(&i_to_r);
    } else {
      printf("error getting input matrix\n");
    }
  }

  /* ----- catch the in stats flag ----- */
  if (in_stats_flag != 0) {
    MRIprintStats(mri, stdout);
  }

  // Load the transform
  if (transform_flag != 0) {
    printf("INFO: Reading transformation from file %s...\n", transform_fname);
    transform_type = TransformFileNameType(transform_fname);
    if (transform_type == MNI_TRANSFORM_TYPE ||
        transform_type == TRANSFORM_ARRAY_TYPE ||
        transform_type == REGISTER_DAT || transform_type == FSLREG_TYPE) {
      printf("Reading transform with LTAreadEx()\n");
      // lta_transform = LTAread(transform_fname);
      lta_transform = LTAreadEx(transform_fname);
      if (lta_transform == nullptr) {
        fmt::fprintf(stderr, "ERROR: Reading transform from file %s\n",
                     transform_fname);
        exit(1);
      }
      if (transform_type == FSLREG_TYPE) {
        MRI *tmp = nullptr;
        if (out_like_flag == 0) {
          printf("ERROR: fslmat does not have the information "
                 "on the dst volume\n");
          printf("ERROR: you must give option '--like volume' to specify the"
                 " dst volume info\n");
          MRIfree(&mri);
          exit(1);
        }
        // now setup dst volume info
        tmp = MRIreadHeader(out_like_name, MRI_VOLUME_TYPE_UNKNOWN);
        // flsmat does not contain src and dst info
        LTAmodifySrcDstGeom(lta_transform, mri, tmp);
        // add src and dst information
        LTAchangeType(lta_transform, LINEAR_VOX_TO_VOX);
        MRIfree(&tmp);
      } // end FSLREG_TYPE

      if (DevXFM != 0) {
        printf("INFO: devolving XFM (%s)\n", devxfm_subject);
        printf("-------- before ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        T = DevolveXFM(devxfm_subject, lta_transform->xforms[0].m_L, nullptr);
        if (T == nullptr) {
          exit(1);
        }
        printf("-------- after ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        printf("-----------------\n");
      } // end DevXFM

      if (invert_transform_flag != 0) {
        inverse_transform_matrix =
            MatrixInverse(lta_transform->xforms[0].m_L, nullptr);
        if (inverse_transform_matrix == nullptr) {
          fmt::fprintf(stderr, "ERROR: inverting transform\n");
          MatrixPrint(stdout, lta_transform->xforms[0].m_L);
          exit(1);
        }

        MatrixFree(&(lta_transform->xforms[0].m_L));
        lta_transform->xforms[0].m_L = inverse_transform_matrix;
        // reverse src and dst target info.
        // since it affects the c_ras values of the result
        // in LTAtransform()
        // question is what to do when transform src info is invalid.
        lt = &lta_transform->xforms[0];
        if (lt->src.valid == 0) {
          char buf[512];
          char *p;
          MRI *mriOrig;

          fmt::fprintf(stderr, "INFO: Trying to get the source "
                               "volume information from the transform name\n");
          // copy transform filename
          strcpy(buf, transform_fname);
          // reverse look for the first '/'
          p = strrchr(buf, '/');
          if (p != nullptr) {
            p++;
            *p = '\0';
            // set the terminator. i.e.
            // ".... mri/transforms" from "
            //..../mri/transforms/talairach.xfm"
          } else // no / present means only a filename is given
          {
            strcpy(buf, "./");
          }
          strcat(buf, "../orig"); // go to mri/orig from
          // mri/transforms/
          // check whether we can read header info or not
          mriOrig = MRIreadHeader(buf, MRI_VOLUME_TYPE_UNKNOWN);
          if (mriOrig != nullptr) {
            getVolGeom(mriOrig, &lt->src);
            fmt::fprintf(stderr, "INFO: Succeeded in retrieving "
                                 "the source volume info.\n");
          } else {
            printf("INFO: Failed to find %s as a source volume.  \n"
                   "      The inverse c_(ras) may not be valid.\n",
                   buf);
          }
        } // end not valid
        copyVolGeom(&lt->dst, &vgtmp);
        copyVolGeom(&lt->src, &lt->dst);
        copyVolGeom(&vgtmp, &lt->src);
      } // end invert_transform_flag

    } // end transform type
    // if type is non-linear, load transform below when applying it...
  } // end transform_flag

  if (reslice_like_flag != 0) {

    if (force_template_type_flag != 0) {
      printf("reading template info from (type %s) volume %s...\n",
             template_type_string, reslice_like_name);
      mri_template = MRIreadHeader(reslice_like_name, forced_template_type);
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     reslice_like_name);
        exit(1);
      }
    } else {
      printf("reading template info from volume %s...\n", reslice_like_name);
      mri_template = MRIreadInfo(reslice_like_name);
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     reslice_like_name);
        exit(1);
      }
      // if we loaded a transform above, set the lta target geometry from
      // mri_template:
      if (lta_transform != nullptr) {
        lta_transform = LTAchangeType(lta_transform, LINEAR_RAS_TO_RAS);
        LTAmodifySrcDstGeom(lta_transform, nullptr, mri_template);
      }
    }
  } else {
    mri_template = MRIallocHeader(mri->width, mri->height, mri->depth,
                                  mri->type, mri->nframes);
    MRIcopyHeader(mri, mri_template);

    // if we loaded a transform above, get target geometry from lta:
    if ((lta_transform != nullptr) && lta_transform->xforms[0].dst.valid == 1) {
      useVolGeomToMRI(&lta_transform->xforms[0].dst, mri_template);
    }

    if (conform_flag != 0) {
      conform_width = 256;
      if (conform_min == TRUE) {
        conform_size = MRIfindMinSize(mri, &conform_width);
      } else {
        if (conform_width_256_flag != 0) {
          conform_width = 256; // force it
        } else {
          conform_width = MRIfindRightSize(mri, conform_size);
        }
      }
      mri_template =
          MRIconformedTemplate(mri, conform_width, conform_size, ConfKeepDC);
    } else if (voxel_size_flag != 0) {
      mri_template = MRIallocHeader(mri->width, mri->height, mri->depth,
                                    mri->type, mri->nframes);
      MRIcopyHeader(mri, mri_template);

      mri_template->nframes = mri->nframes;

      mri_template->width =
          static_cast<int>(ceil(mri->xsize * mri->width / voxel_size[0]));
      mri_template->height =
          static_cast<int>(ceil(mri->ysize * mri->height / voxel_size[1]));
      mri_template->depth =
          static_cast<int>(ceil(mri->zsize * mri->depth / voxel_size[2]));

      mri_template->xsize = voxel_size[0];
      mri_template->ysize = voxel_size[1];
      mri_template->zsize = voxel_size[2];

      mri_template->xstart = -mri_template->width / 2;
      mri_template->xend = mri_template->width / 2;
      mri_template->ystart = -mri_template->height / 2;
      mri_template->yend = mri_template->height / 2;
      mri_template->zstart = -mri_template->depth / 2;
      mri_template->zend = mri_template->depth / 2;

    } else if (downsample_flag != 0) {
      mri_template = MRIallocHeader(mri->width, mri->height, mri->depth,
                                    mri->type, mri->nframes);
      MRIcopyHeader(mri, mri_template);

      mri_template->nframes = mri->nframes;

      mri_template->width =
          static_cast<int>(ceil(mri->width / downsample_factor[0]));
      mri_template->height =
          static_cast<int>(ceil(mri->height / downsample_factor[1]));
      mri_template->depth =
          static_cast<int>(ceil(mri->depth / downsample_factor[2]));

      mri_template->xsize *= downsample_factor[0];
      mri_template->ysize *= downsample_factor[1];
      mri_template->zsize *= downsample_factor[2];

      mri_template->xstart = -mri_template->xsize * mri_template->width / 2;
      mri_template->xend = mri_template->xsize * mri_template->width / 2;
      mri_template->ystart = -mri_template->ysize * mri_template->height / 2;
      mri_template->yend = mri_template->ysize * mri_template->height / 2;
      mri_template->zstart = -mri_template->zsize * mri_template->depth / 2;
      mri_template->zend = mri_template->zsize * mri_template->depth / 2;
    }
  }

  /* ----- apply command-line parameters ----- */
  if (out_i_size_flag != 0) {
    float scale;
    scale = mri_template->xsize / out_i_size;
    mri_template->xsize = out_i_size;
    mri_template->width = nint(mri_template->width * scale);
  }
  if (out_j_size_flag != 0) {
    float scale;
    scale = mri_template->ysize / out_j_size;
    mri_template->ysize = out_j_size;
    mri_template->height = nint(mri_template->height * scale);
  }
  if (out_k_size_flag != 0) {
    float scale;
    scale = mri_template->zsize / out_k_size;
    mri_template->zsize = out_k_size;
    mri_template->depth = nint(mri_template->depth * scale);
  }
  if (out_n_i_flag != 0) {
    mri_template->width = out_n_i;
  }
  if (out_n_j_flag != 0) {
    mri_template->height = out_n_j;
  }
  if (out_n_k_flag != 0) {
    mri_template->depth = out_n_k;
    mri_template->imnr1 = mri_template->imnr0 + out_n_k - 1;
  }
  if (out_i_direction_flag != 0) {
    mri_template->x_r = out_i_directions[0];
    mri_template->x_a = out_i_directions[1];
    mri_template->x_s = out_i_directions[2];
  }
  if (out_j_direction_flag != 0) {
    mri_template->y_r = out_j_directions[0];
    mri_template->y_a = out_j_directions[1];
    mri_template->y_s = out_j_directions[2];
  }
  if (out_k_direction_flag != 0) {
    mri_template->z_r = out_k_directions[0];
    mri_template->z_a = out_k_directions[1];
    mri_template->z_s = out_k_directions[2];
  }
  if (out_orientation_flag != 0) {
    printf("Setting output orientation to %s\n", out_orientation_string);
    MRIorientationStringToDircos(mri_template, out_orientation_string);
  }
  if (out_center_flag != 0) {
    mri_template->c_r = out_center[0];
    mri_template->c_a = out_center[1];
    mri_template->c_s = out_center[2];
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((out_i_size_flag != 0) || (out_j_size_flag != 0) ||
      (out_k_size_flag != 0) || (out_n_i_flag != 0) || (out_n_j_flag != 0) ||
      (out_n_k_flag != 0)) {

    fov_x = mri_template->xsize * mri_template->width;
    fov_y = mri_template->ysize * mri_template->height;
    fov_z = mri_template->zsize * mri_template->depth;
    mri_template->xend = fov_x / 2.0;
    mri_template->xstart = -mri_template->xend;
    mri_template->yend = fov_y / 2.0;
    mri_template->ystart = -mri_template->yend;
    mri_template->zend = fov_z / 2.0;
    mri_template->zstart = -mri_template->zend;

    mri_template->fov = (fov_x > fov_y ? (fov_x > fov_z ? fov_x : fov_z)
                                       : (fov_y > fov_z ? fov_y : fov_z));
  }

  /* ----- give a warning for non-orthogonal directions ----- */
  i_dot_j = mri_template->x_r * mri_template->y_r +
            mri_template->x_a * mri_template->y_a +
            mri_template->x_s * mri_template->y_s;
  i_dot_k = mri_template->x_r * mri_template->z_r +
            mri_template->x_a * mri_template->z_a +
            mri_template->x_s * mri_template->z_s;
  j_dot_k = mri_template->y_r * mri_template->z_r +
            mri_template->y_a * mri_template->z_a +
            mri_template->y_s * mri_template->z_s;
  if (fabs(i_dot_j) > CLOSE_ENOUGH || fabs(i_dot_k) > CLOSE_ENOUGH ||
      fabs(i_dot_k) > CLOSE_ENOUGH) {
    printf("warning: output volume axes are not orthogonal:"
           "i_dot_j = %.6f, i_dot_k = %.6f, j_dot_k = %.6f\n",
           i_dot_j, i_dot_k, j_dot_k);
    printf("i_ras = (%g, %g, %g)\n", mri_template->x_r, mri_template->x_a,
           mri_template->x_s);
    printf("j_ras = (%g, %g, %g)\n", mri_template->y_r, mri_template->y_a,
           mri_template->y_s);
    printf("k_ras = (%g, %g, %g)\n", mri_template->z_r, mri_template->z_a,
           mri_template->z_s);
  }
  if (out_data_type >= 0) {
    mri_template->type = out_data_type;
  }

  /* ----- catch the mri_template info flag ----- */
  if (template_info_flag != 0) {
    printf("template structure:\n");
    MRIdump(mri_template, stdout);
  }

  /* ----- exit here if read only is desired ----- */
  if (read_only_flag != 0) {
    exit(0);
  }

  /* ----- apply a transformation if requested ----- */
  if (transform_flag != 0) {
    printf("INFO: Applying transformation from file %s...\n", transform_fname);
    transform_type = TransformFileNameType(transform_fname);
    if (transform_type == MNI_TRANSFORM_TYPE ||
        transform_type == TRANSFORM_ARRAY_TYPE ||
        transform_type == REGISTER_DAT || transform_type == FSLREG_TYPE) {

      // in these cases the lta_transform was loaded above
      if (lta_transform == nullptr) {
        fmt::fprintf(stderr, "ERROR: lta should have been loaded \n");
        exit(1);
      }

      /* Think about calling MRIlinearTransform() here; need vox2vox
         transform. Can create NN version. In theory, LTAtransform()
         can handle multiple transforms, but the inverse assumes only
         one. NN is good for ROI*/

      printf("---------------------------------\n");
      printf("INFO: Transform Matrix (%s)\n",
             LTAtransformTypeName(lta_transform->type));
      MatrixPrint(stdout, lta_transform->xforms[0].m_L);
      printf("---------------------------------\n");

      /* LTAtransform() runs either MRIapplyRASlinearTransform()
         for RAS2RAS or MRIlinearTransform() for Vox2Vox. */
      /* MRIlinearTransform() calls MRIlinearTransformInterp() */
      if (out_like_flag == 1) {
        MRI *tmp = nullptr;
        printf("INFO: transform dst into the like-volume (resample_type %d)\n",
               resample_type_val);
        tmp = MRIreadHeader(out_like_name, MRI_VOLUME_TYPE_UNKNOWN);
        mri_transformed =
            MRIalloc(tmp->width, tmp->height, tmp->depth, mri->type);
        if (mri_transformed == nullptr) {
          ErrorExit(ERROR_NOMEMORY, "could not allocate memory");
        }
        MRIcopyHeader(tmp, mri_transformed);
        MRIfree(&tmp);
        tmp = nullptr;
        mri_transformed = LTAtransformInterp(mri, mri_transformed,
                                             lta_transform, resample_type_val);
      } else {
        if (out_center_flag != 0) {
          MATRIX *m;
          MATRIX *mtmp;
          m = MRIgetResampleMatrix(mri_template, mri);
          mtmp = MRIrasXformToVoxelXform(mri, mri, lta_transform->xforms[0].m_L,
                                         nullptr);
          MatrixFree(&lta_transform->xforms[0].m_L);
          lta_transform->xforms[0].m_L = MatrixMultiply(m, mtmp, NULL);
          MatrixFree(&m);
          MatrixFree(&mtmp);
          lta_transform->type = LINEAR_VOX_TO_VOX;
        }

        printf("Applying LTAtransformInterp (resample_type %d)\n",
               resample_type_val);
        if (Gdiag_no > 0) {
          printf("Dumping LTA ---------++++++++++++-----------------------\n");
          LTAdump(stdout, lta_transform);
          printf("========-------------++++++++++++-------------------=====\n");
        }
        mri_transformed =
            LTAtransformInterp(mri, nullptr, lta_transform, resample_type_val);
      } // end out_like_flag treatment

      if (mri_transformed == nullptr) {
        fmt::fprintf(stderr, "ERROR: applying transform to volume\n");
        exit(1);
      }
      if (out_center_flag != 0) {
        mri_transformed->c_r = mri_template->c_r;
        mri_transformed->c_a = mri_template->c_a;
        mri_transformed->c_s = mri_template->c_s;
      }
      LTAfree(&lta_transform);
      MRIfree(&mri);
      mri = mri_transformed;
    } else if (transform_type == MORPH_3D_TYPE) {
      printf("Applying morph_3d ...\n");
      // this is a non-linear vox-to-vox transform
      // note that in this case trilinear
      // interpolation is always used, and -rt
      // option has no effect! -xh
      TRANSFORM *tran = TransformRead(transform_fname);
      // check whether the volume to be morphed and the morph have the same
      // dimensions
      if (tran == nullptr) {
        ErrorExit(ERROR_NOFILE, "%s: could not read xform from %s\n", Progname,
                  transform_fname);
      }
      if (invert_transform_flag == 0) {
        printf("morphing to atlas with resample type %d\n", resample_type_val);
        mri_transformed =
            GCAMmorphToAtlas(mri, static_cast<GCA_MORPH *>(tran->xform),
                             nullptr, 0, resample_type_val);
        useVolGeomToMRI(&(static_cast<GCA_MORPH *>(tran->xform))->atlas,
                        mri_template);
      } else // invert
      {
        auto *gcam = static_cast<GCA_MORPH *>(tran->xform);

        // check whether the volume to be morphed and the morph have the same
        // dimensions
#if 0
        if ((gcam->image.width == mri->width) &&
	    (gcam->image.height == mri->height) &&
	    (gcam->image.depth == mri->depth))
	  mri_transformed = MRIclone(mri, NULL);
	else  // when morphing atlas to subject using -ait <3d morph> must resample atlas to 256 before applying morph
	{
	  MRI *mri_tmp ;
	  mri_transformed = MRIalloc(gcam->image.width,  gcam->image.height, gcam->image.depth, mri->type) ;
	  useVolGeomToMRI(&gcam->image, mri_transformed) ;
	  mri_tmp = MRIresample(mri, mri_transformed, resample_type_val);
	  MRIfree(&mri) ; MRIfree(&mri_template) ;
	  mri = mri_tmp ;
	}
#endif
        printf("morphing from atlas with resample type %d\n",
               resample_type_val);
        mri_transformed =
            GCAMmorphFromAtlas(mri, gcam, mri_transformed, resample_type_val);
        MRIwrite(mri_transformed, "t.mgz");
      }
      TransformFree(&tran);
      MRIfree(&mri);
      mri = mri_transformed;
    } else {
      fmt::fprintf(stderr, "unknown transform type in file %s\n",
                   transform_fname);
      exit(1);
    }
  } else if ((out_like_flag != 0) &&
             (OutStatTableFlag == 0)) // flag set but no transform
  {
    // modify direction cosines to use the
    // out-like volume direction cosines
    //
    //   src --> RAS
    //    |       |
    //    |       |
    //    V       V
    //   dst --> RAS   where dst i_to_r is taken from out volume
    MRI *tmp = nullptr;
    MATRIX *src2dst = nullptr;

    printf("INFO: transform src into the like-volume: %s\n", out_like_name);
    tmp = MRIreadHeader(out_like_name, MRI_VOLUME_TYPE_UNKNOWN);
    mri_transformed = MRIalloc(tmp->width, tmp->height, tmp->depth, mri->type);
    if (mri_transformed == nullptr) {
      ErrorExit(ERROR_NOMEMORY, "could not allocate memory");
    }
    MRIcopyHeader(tmp, mri_transformed);
    MRIfree(&tmp);
    tmp = nullptr;
    // just to make sure
    MATRIX *tmp2;
    if ((mri->i_to_r__) == nullptr) {
      tmp2 = extract_i_to_r(mri);
      AffineMatrixAlloc(&(mri->i_to_r__));
      SetAffineMatrix(mri->i_to_r__, tmp2);
    } else {
      tmp2 = MatrixAlloc(4, 4, MATRIX_REAL);
      GetAffineMatrix(tmp2, mri->i_to_r__);
    }

    if ((mri_transformed->r_to_i__) == nullptr) {
      mri_transformed->r_to_i__ = extract_r_to_i(mri_transformed);
    }
    // got the transform
    src2dst = MatrixMultiply(mri_transformed->r_to_i__, tmp2, NULL);
    // now get the values (tri-linear)
    MRIlinearTransform(mri, mri_transformed, src2dst);
    MatrixFree(&src2dst);
    MatrixFree(&tmp2);
    MRIfree(&mri);
    mri = mri_transformed;
  }

  /* ----- change type if necessary ----- */
  if (mri->type != mri_template->type && nochange_flag == FALSE) {
    printf("changing data type from %s to %s (noscale = %d)...\n",
           MRItype2str(mri->type), MRItype2str(mri_template->type),
           no_scale_flag);
    mri2 = MRISeqchangeType(mri, mri_template->type, 0.0, 0.999, no_scale_flag);
    if (mri2 == nullptr) {
      printf("ERROR: MRISeqchangeType\n");
      exit(1);
    }
    /* (mr) now always map 0 to 0 (was only in conform case before)
       should this be part of MRISeqchangeType? */
    /* (dng) this used to use MRImask, but that did not work for multiple
     * frame*/
    /* Neither mr nor dng can remember where a problem came up that required
     * this fix*/
    for (c = 0; c < mri->width; c++) {
      for (r = 0; r < mri->height; r++) {
        for (s = 0; s < mri->depth; s++) {
          for (f = 0; f < mri->nframes; f++) {
            v = MRIgetVoxVal(mri, c, r, s, f);
            if (v == 0) {
              MRIsetVoxVal(mri2, c, r, s, f, 0);
            }
          }
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /*printf("mri  vs.  mri_template\n");
  printf(" dim  ( %i , %i , %i ) vs ( %i , %i , %i
  )\n",mri->width,mri->height,mri->depth,mri_template->width,mri_template->height,mri_template->depth);
  printf(" size ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->xsize,mri->ysize,mri->zsize,mri_template->xsize,mri_template->ysize,mri_template->zsize);
  printf(" xras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->x_r,mri->x_a,mri->x_s,mri_template->x_r,mri_template->x_a,mri_template->x_s);
  printf(" yras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->y_r,mri->x_a,mri->y_s,mri_template->y_r,mri_template->y_a,mri_template->y_s);
  printf(" zras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->z_r,mri->x_a,mri->z_s,mri_template->z_r,mri_template->z_a,mri_template->z_s);
  printf(" cras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->c_r,mri->c_a,mri->c_s,mri_template->c_r,mri_template->c_a,mri_template->c_s);
  */

  /* ----- reslice if necessary and not performed during transform ----- */
  float eps = 1e-05; /* (mr) do eps testing to avoid reslicing due to tiny
                        differences, e.g. from IO */
  if ((out_like_flag == 0) && (transform_type != MORPH_3D_TYPE) &&
      (fabs(mri->xsize - mri_template->xsize) > eps ||
       fabs(mri->ysize - mri_template->ysize) > eps ||
       fabs(mri->zsize - mri_template->zsize) > eps ||
       mri->width != mri_template->width ||
       mri->height != mri_template->height ||
       mri->depth != mri_template->depth ||
       fabs(mri->x_r - mri_template->x_r) > eps ||
       fabs(mri->x_a - mri_template->x_a) > eps ||
       fabs(mri->x_s - mri_template->x_s) > eps ||
       fabs(mri->y_r - mri_template->y_r) > eps ||
       fabs(mri->y_a - mri_template->y_a) > eps ||
       fabs(mri->y_s - mri_template->y_s) > eps ||
       fabs(mri->z_r - mri_template->z_r) > eps ||
       fabs(mri->z_a - mri_template->z_a) > eps ||
       fabs(mri->z_s - mri_template->z_s) > eps ||
       fabs(mri->c_r - mri_template->c_r) > eps ||
       fabs(mri->c_a - mri_template->c_a) > eps ||
       fabs(mri->c_s - mri_template->c_s) > eps)) {
    printf("Reslicing using ");
    switch (resample_type_val) {
    case SAMPLE_TRILINEAR:
      printf("trilinear interpolation \n");
      break;
    case SAMPLE_NEAREST:
      printf("nearest \n");
      break;
      /*    case SAMPLE_SINC:
            printf("sinc \n");
            break;*/
    case SAMPLE_CUBIC:
      printf("cubic \n");
      break;
    case SAMPLE_WEIGHTED:
      printf("weighted \n");
      break;
    case SAMPLE_VOTE:
      printf("voting \n");
      break;
    }
    mri2 = MRIresample(mri, mri_template, resample_type_val);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- invert contrast if necessary ----- */
  if (invert_val >= 0) {
    printf("inverting contrast...\n");
    mri2 = MRIinvertContrast(mri, nullptr, invert_val);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- reorder if necessary ----- */
  if (reorder_flag != 0) {
    printf("reordering axes...\n");
    mri2 = MRIreorder(mri, nullptr, reorder_vals[0], reorder_vals[1],
                      reorder_vals[2]);
    if (mri2 == nullptr) {
      fmt::fprintf(stderr, "error reordering axes\n");
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- reorder if necessary ----- */
  if (reorder4_flag != 0) {
    printf("reordering all axes...\n");
    printf("reordering, values are %d %d %d %d\n", reorder4_vals[0],
           reorder4_vals[1], reorder4_vals[2], reorder4_vals[3]);
    mri2 = MRIreorder4(mri, reorder4_vals);
    if (mri2 == nullptr) {
      fmt::fprintf(stderr, "error reordering all axes\n");
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- store the gdf file stem ----- */
  strcpy(mri->gdf_image_stem, gdf_image_stem.data());

  /* ----- catch the out info flag ----- */
  if (out_info_flag != 0) {
    printf("output structure:\n");
    MRIdump(mri, stdout);
  }

  /* ----- catch the out matrix flag ----- */
  if (out_matrix_flag != 0) {
    MATRIX *i_to_r;
    i_to_r = extract_i_to_r(mri);
    if (i_to_r != nullptr) {
      printf("output ijk -> ras:\n");
      MatrixPrint(stdout, i_to_r);
      MatrixFree(&i_to_r);
    } else {
      printf("error getting output matrix\n");
    }
  }

  /* ----- catch the out stats flag ----- */
  if (out_stats_flag != 0) {
    MRIprintStats(mri, stdout);
  }

  if (erode_seg_flag == TRUE) {
    printf("Eroding segmentation %d\n", n_erode_seg);
    mri2 = MRIerodeSegmentation(mri, nullptr, n_erode_seg, 0);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (dil_seg_flag == TRUE) {
    mritmp = nullptr;
    if (dil_seg_mask[0] == '\0') {
      printf("Dilating segmentation %d\n", n_dil_seg);
    } else {
      printf("Dilating segmentation, mask %s\n", dil_seg_mask);
      mritmp = MRIread(dil_seg_mask.data());
      if (mritmp == nullptr) {
        printf("ERROR: could not read %s\n", dil_seg_mask);
        exit(1);
      }
    }
    mri2 = MRIdilateSegmentation(mri, nullptr, n_dil_seg, mritmp, 0, 0.5, &i);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (frame_flag == TRUE) {
    if (mid_frame_flag == TRUE) {
      nframes = 1;
      frames[0] = nint(mri->nframes / 2);
    }
    if (nframes == 1) {
      printf("keeping frame %d\n", frames[0]);
    } else {
      printf("keeping frames");
      for (f = 0; f < nframes; f++) {
        printf(" %d", frames[f]);
      }
      printf("\n");
    }
    mri2 = MRIallocSequence(mri->width, mri->height, mri->depth, mri->type,
                            nframes);
    if (mri2 == nullptr) {
      ErrorExit(ERROR_NOMEMORY, "could not allocate memory");
    }
    MRIcopyHeader(mri, mri2);
    MRIcopyPulseParameters(mri, mri2);
    for (f = 0; f < nframes; f++) {
      if (frames[f] < 0 || frames[f] >= mri->nframes) {
        printf("   ERROR: valid frame numbers are between 0 and %d\n",
               mri->nframes - 1);
        exit(1);
      } else {
        for (s = 0; s < mri2->depth; s++) {
          for (r = 0; r < mri2->height; r++) {
            for (c = 0; c < mri2->width; c++) {
              MRIsetVoxVal(mri2, c, r, s, f,
                           MRIgetVoxVal(mri, c, r, s, frames[f]));
            }
          }
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- drop the last ndrop frames (drop before skip) ------ */
  if (ndrop > 0) {
    printf("Dropping last %d frames\n", ndrop);
    if (mri->nframes <= ndrop) {
      printf("   ERROR: can't drop, volume only has %d frames\n", mri->nframes);
      exit(1);
    }
    mri2 = fMRIndrop(mri, ndrop, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- skip the first nskip frames ------ */
  if (nskip > 0) {
    printf("Skipping %d frames\n", nskip);
    if (mri->nframes <= nskip) {
      printf("   ERROR: can't skip, volume only has %d frames\n", mri->nframes);
      exit(1);
    }
    mri2 = fMRInskip(mri, nskip, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (subsample_flag != 0) {
    printf("SubSample: Start = %d  Delta = %d, End = %d\n", SubSampStart,
           SubSampDelta, SubSampEnd);
    mri2 = fMRIsubSample(mri, SubSampStart, SubSampDelta, SubSampEnd, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cutends_flag == TRUE) {
    printf("Cutting ends: n = %d\n", ncutends);
    mri2 = MRIcutEndSlices(mri, ncutends);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- crops ---------*/
  if (crop_flag == TRUE) {
    MRI *mri_tmp;
    int x0;
    int y0;
    int z0;

    x0 = crop_center[0];
    y0 = crop_center[1];
    z0 = crop_center[2];

    x0 -= crop_size[0] / 2;
    y0 -= crop_size[1] / 2;
    z0 -= crop_size[2] / 2;
    mri_tmp = MRIextract(mri, nullptr, x0, y0, z0, crop_size[0], crop_size[1],
                         crop_size[2]);
    MRIfree(&mri);
    mri = mri_tmp;
  }

  if (!FEQUAL(out_scale_factor, 1.0)) {
    printf("scaling output intensities by %2.3f\n", out_scale_factor);
    MRIscalarMul(mri, mri, out_scale_factor);
  }

  if (sphinx_flag != 0) {
    printf("Changing orientation information to Sphinx\n");
    MRIhfs2Sphinx(mri);
  }

  if (AutoAlign != nullptr) {
    mri->AutoAlign = AutoAlign;
  }

  // ----- modify color lookup table (specified by --ctab option) -----
  if (strcmp("remove", colortablefile) == 0) {
    // remove an embedded ctab
    if (mri->ct != nullptr) {
      std::cout << "removing color lookup table" << std::endl;
      CTABfree(&mri->ct);
    }
  } else if (strlen(colortablefile) != 0) {
    // add a user-specified ctab
    std::cout << "embedding color lookup table" << std::endl;
    if (mri->ct != nullptr) {
      CTABfree(&mri->ct);
    }
    mri->ct = CTABreadASCII(colortablefile);
    if (mri->ct == nullptr) {
      fs::fatal() << "could not read lookup table from " << colortablefile;
    }
  }

  /*------ Finally, write the output -----*/

  if (OutStatTableFlag != 0) {
    printf("Writing as Stats-Table to %s using template %s\n", out_name,
           out_like_name);

    OutStatTable = InitStatTableFromMRI(mri, out_like_name);
    WriteStatTable(out_name.data(), OutStatTable);

    printf("done\n");
    exit(0);
  }

  if (ascii_flag != 0) {
    printf("Writing as ASCII to %s\n", out_name);
    fptmp = fopen(out_name.data(), "w");
    if (ascii_flag == 1 || ascii_flag == 2) {
      for (f = 0; f < mri->nframes; f++) {
        for (s = 0; s < mri->depth; s++) {
          for (r = 0; r < mri->height; r++) {
            for (c = 0; c < mri->width; c++) {
              if (ascii_flag == 1) {
                fmt::fprintf(fptmp, "%lf\n", MRIgetVoxVal(mri, c, r, s, f));
              }
              if (ascii_flag == 2) {
                fmt::fprintf(fptmp, "%3d %3d %3d %3d %lf\n", c, r, s, f,
                             MRIgetVoxVal(mri, c, r, s, f));
              }
            }
          }
        }
      }
    }
    if (ascii_flag == 3) {
      for (s = 0; s < mri->depth; s++) {
        for (r = 0; r < mri->height; r++) {
          for (c = 0; c < mri->width; c++) {
            for (f = 0; f < mri->nframes; f++) {
              fmt::fprintf(fptmp, "%lf", MRIgetVoxVal(mri, c, r, s, f));
              if (f != mri->nframes - 1) {
                fmt::fprintf(fptmp, " ");
              }
            }
            fmt::fprintf(fptmp, "\n");
          }
        }
      }
    }
    fclose(fptmp);
    printf("done\n");
    exit(0);
  }
  if (no_write_flag == 0) {
    if (SplitFrames == 0) {
      printf("writing to %s...\n", out_name);
      if (force_out_type_flag != 0) {
        err = MRIwriteType(mri, out_name.data(), out_volume_type);
        if (err != NO_ERROR) {
          printf("ERROR: failure writing %s as volume type %d\n", out_name,
                 out_volume_type);
          exit(1);
        }
      } else {
        err = MRIwrite(mri, out_name.data());
        if (err != NO_ERROR) {
          printf("ERROR: failure writing %s\n", out_name);
          exit(1);
        }
      }
    } else {
      stem = IDstemFromName(out_name.data());
      ext = IDextensionFromName(out_name.data());

      printf("Splitting frames, stem = %s, ext = %s\n", stem, ext);
      mri2 = nullptr;
      for (i = 0; i < mri->nframes; i++) {
        mri2 = MRIcopyFrame(mri, mri2, i, 0);
        sprintf(tmpstr, "%s%04d.%s", stem, i, ext);
        printf("%2d %s\n", i, tmpstr);
        err = MRIwrite(mri2, tmpstr);
        if (err != NO_ERROR) {
          printf("ERROR: failure writing %s\n", tmpstr);
          exit(1);
        }
      }
    }
  }
  // free memory
  // MRIfree(&mri); // This causes a seg fault with change of type
  MRIfree(&mri_template);

  exit(0);

} /* end main() */
/*----------------------------------------------------------------------*/

void get_ints(int argc, char *argv[], int *pos, int *vals, int nvals) {

  char *ep;
  int i;

  if (*pos + nvals >= argc) {
    fmt::fprintf(stderr,
                 "\n%s: argument %s expects %d integers; "
                 "only %d arguments after flag\n",
                 Progname, argv[*pos], nvals, argc - *pos - 1);
    usage_message(stdout);
    exit(1);
  }

  for (i = 0; i < nvals; i++) {
    if (argv[*pos + i + 1][0] == '\0') {
      fmt::fprintf(stderr, "\n%s: argument to %s flag is null\n", Progname,
                   argv[*pos]);
      usage_message(stdout);
      exit(1);
    }

    vals[i] = static_cast<int>(strtol(argv[*pos + i + 1], &ep, 10));

    if (*ep != '\0') {
      fmt::fprintf(stderr,
                   "\n%s: error converting \"%s\" to an "
                   "integer for %s flag, incorrect # of args (need %d)?\n",
                   Progname, argv[*pos + i + 1], argv[*pos], nvals);
      usage_message(stdout);
      exit(1);
    }
  }

  *pos += nvals;

} /* end get_ints() */

void get_floats(int argc, char *argv[], int *pos, float *vals, int nvals) {

  char *ep;
  int i;

  if (*pos + nvals >= argc) {
    fmt::fprintf(stderr,
                 "\n%s: argument %s expects %d floats; "
                 "only %d arguments after flag\n",
                 Progname, argv[*pos], nvals, argc - *pos - 1);
    usage_message(stdout);
    exit(1);
  }

  for (i = 0; i < nvals; i++) {
    if (argv[*pos + i + 1][0] == '\0') {
      fmt::fprintf(stderr, "\n%s: argument to %s flag is null\n", Progname,
                   argv[*pos]);
      usage_message(stdout);
      exit(1);
    }

    vals[i] = static_cast<float>(strtod(argv[*pos + i + 1], &ep));

    if (*ep != '\0') {
      fmt::fprintf(stderr,
                   "\n%s: error converting \"%s\" to a "
                   "float for %s flag: incorrect # of args? Need %d\n",
                   Progname, argv[*pos + i + 1], argv[*pos], nvals);
      usage_message(stdout);
      exit(1);
    }
  }

  *pos += nvals;

} /* end get_floats() */

void get_string(int argc, char *argv[], int *pos, char *val) {

  if (*pos + 1 >= argc) {
    fmt::fprintf(stderr,
                 "\n%s: argument %s expects an extra argument; "
                 "none found\n",
                 Progname, argv[*pos]);
    usage_message(stdout);
    exit(1);
  }

  strcpy(val, argv[*pos + 1]);

  (*pos)++;

} /* end get_string() */

void usage_message(FILE *stream) {

  fmt::fprintf(stream, "\n");
  fmt::fprintf(stream, "type %s -u for usage\n", Progname);
  fmt::fprintf(stream, "\n");

} /* end usage_message() */

#include "mri_convert.help.xml.h"
void usage(FILE *stream) {
  outputHelpXml(mri_convert_help_xml, mri_convert_help_xml_len);
} /* end usage() */

static bool good_cmdline_args(CMDARGS *cmdargs, ENV *env) {

  podesc desc("\nUSAGE: mri_wbc <options> --lh <lhsurface> -o "
              "<outdir>\n\nAvailable Options");
  povm vm;

  initArgDesc(&desc, cmdargs);
  auto args = cmdargs->raw;
  auto ac = static_cast<int>(args.size());
  auto av = args.data();
  if (ac == 1) {
    print_usage(desc, env);
    return false;
  }

  try {
    auto parsed_opts =
        po::command_line_parser(ac, av).options(desc).style(cl_style).run();
    po::store(parsed_opts, vm);
  } catch (std::exception const &e) {
    spdlog::get("stderr")->critical(e.what());
    return false;
  }

  if (vm.count("help") != 0U) {
    print_help(desc, env);
    return false;
  }

  if ((vm.count("version") != 0U) || (vm.count("all-info") != 0U)) {
    handle_version_option(vm.count("all-info") != 0U, args, env->vcid,
                          "$Name:  $");
    return false;
  }

  try {
    po::notify(vm);
  } catch (std::exception const &e) {
    spdlog::get("stderr")->critical(e.what());
    return false;
  }

  if (cmdargs->debug) {
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
  } else {
    spdlog::set_level(spdlog::level::warn);
  }
  return false;
}

void initArgDesc(podesc *desc, CMDARGS *cmdargs) {

  desc->add_options()                                               /**/
                                                                    /**/
      ("help,h",                                                    /**/
       "print out information on how to use this program and exit") /**/
                                                                    /**/
      ("version,v",                                                 /**/
       "print out version and exit")                                /**/
                                                                    /**/
      ;
}

/* EOF */
