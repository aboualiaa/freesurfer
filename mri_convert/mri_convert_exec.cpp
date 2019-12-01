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

#include "mri_convert.hpp"
#include "mri_convert_lib.hpp"
#include <armadillo>

auto main(int argc, char *argv[]) -> int {
  auto err_logger = spdlog::stderr_color_mt("stderr");

  auto env = ENV();
  auto cmdargs = CMDARGS(argc, argv);
  if (!good_cmdline_args(&cmdargs, &env)) {
    return 1;
  }

  int ncutends{};
  bool cutends_flag{};
  bool slice_crop_flag{};
  int slice_crop_start{};
  int slice_crop_stop{};
  int SplitFrames{};
  int DeleteCMDs{};
  std::array<char, 2000> NewTransformFname{};
  int DoNewTransformFname{};
  int outside_val{};
  int nargs{};
  MRI *mri{};
  MRI *mri2{};
  MRI *mri_template{};
  MRI *mri_in_like{};
  int i{};
  int err{};
  std::array<int, 3> reorder_vals{};
  float invert_val{1.0};
  bool in_info_flag{};
  bool out_info_flag{};
  bool template_info_flag{};
  bool voxel_size_flag{};
  bool nochange_flag{};
  bool conform_flag{};
  bool conform_min{}; // conform to the smallest dimension
  int conform_width{};
  bool conform_width_256_flag{};
  int ConfKeepDC{};
  bool parse_only_flag{};
  bool reorder_flag{};
  std::array<int, 4> reorder4_vals{};
  bool reorder4_flag{};
  bool in_stats_flag{};
  bool out_stats_flag{};
  bool read_only_flag{};
  bool no_write_flag{};
  std::string in_name{};
  std::string out_name{};
  int in_volume_type{};
  int out_volume_type{MRI_VOLUME_TYPE_UNKNOWN};
  std::string resample_type{};
  int resample_type_val{SAMPLE_TRILINEAR};
  bool in_i_size_flag{};
  bool in_j_size_flag{};
  bool in_k_size_flag{};
  bool crop_flag{};
  bool out_i_size_flag{};
  bool out_j_size_flag{};
  bool out_k_size_flag{};
  double in_i_size{};
  float in_j_size{};
  double in_k_size{};
  float out_i_size{};
  float out_j_size{};
  double out_k_size{};
  std::array<int, 3> crop_center{};
  bool sizes_good_flag{};
  std::array<int, 3> crop_size{};
  std::vector<double> in_i_directions{};
  std::vector<double> in_j_directions{};
  std::vector<double> in_k_directions{};
  std::vector<double> out_i_directions{};
  std::vector<double> out_j_directions{};
  std::vector<double> out_k_directions{};
  std::array<float, 3> voxel_size{};
  bool in_i_direction_flag{};
  bool in_j_direction_flag{};
  bool in_k_direction_flag{};
  bool out_i_direction_flag{};
  bool out_j_direction_flag{};
  bool out_k_direction_flag{};
  bool in_orientation_flag{};
  std::string in_orientation_string{};
  bool out_orientation_flag{};
  std::string out_orientation_string{};
  std::string colortablefile{};
  std::string tmpstr{};
  char *stem{};
  char *ext{};
  std::array<char, 4> ostr{};
  char *errmsg{};
  bool in_tr_flag{};
  float in_tr{};
  bool in_ti_flag{};
  float in_ti{};
  bool in_te_flag{};
  float in_te{};
  bool in_flip_angle_flag{};
  float in_flip_angle{};
  double magnitude;
  float i_dot_j{};
  float i_dot_k{};
  float j_dot_k{};
  std::array<float, 3> in_center{};
  std::array<float, 3> out_center{};
  std::array<float, 3> delta_in_center{};
  bool in_center_flag{};
  bool out_center_flag{};
  bool delta_in_center_flag{};
  int out_data_type{-1};
  std::string out_data_type_string{};
  int out_n_i{};
  int out_n_j{};
  int out_n_k{};
  bool out_n_i_flag{};
  bool out_n_j_flag{};
  bool out_n_k_flag{};
  float fov_x{};
  float fov_y{};
  float fov_z{};
  bool force_in_type_flag{};
  bool force_out_type_flag{};
  int forced_in_type{MRI_VOLUME_TYPE_UNKNOWN};
  int forced_out_type{MRI_VOLUME_TYPE_UNKNOWN};
  std::string in_type_string{};
  std::string out_type_string{};
  std::string subject_name{};
  bool force_template_type_flag{};
  int forced_template_type{MRI_VOLUME_TYPE_UNKNOWN};
  std::string template_type_string{};
  std::string reslice_like_name{};
  bool reslice_like_flag{};
  int nframes{};
  bool frame_flag{};
  bool mid_frame_flag{};
  std::array<int, 2000> frames{};
  char *errormsg{};
  bool subsample_flag{};
  int SubSampStart{};
  int SubSampDelta{};
  int SubSampEnd{};
  bool downsample2_flag{};
  bool downsample_flag{};
  std::array<float, 3> downsample_factor{};
  std::string in_name_only{};
  std::string transform_fname{};
  bool transform_flag{};
  bool invert_transform_flag{};
  LTA *lta_transform{};
  MRI *mri_transformed{};
  MRI *mritmp{};
  int transform_type{-1};
  MATRIX *inverse_transform_matrix{};
  bool smooth_parcellation_flag{};
  int smooth_parcellation_count{};
  bool in_like_flag{};
  std::string in_like_name{};
  std::string out_like_name{};
  bool out_like_flag{};
  int in_n_i{};
  int in_n_j{};
  int in_n_k{};
  bool in_n_i_flag{};
  bool in_n_j_flag{};
  bool in_n_k_flag{};
  bool fill_parcellation_flag{};
  bool read_parcellation_volume_flag{};
  bool zero_outlines_flag{};
  bool erode_seg_flag{};
  int n_erode_seg{};
  bool dil_seg_flag{};
  int n_dil_seg{};
  std::string dil_seg_mask{};
  int read_otl_flags{};
  bool color_file_flag{};
  std::string color_file_name{};
  int no_scale_flag{}; // TODO(aboualiaa): convert to bool
  int temp_type{};
  bool roi_flag{};
  FILE *fptmp{};
  int j{};
  bool translate_labels_flag{};
  bool force_ras_good{};
  std::string gdf_image_stem{};
  bool in_matrix_flag{};
  bool out_matrix_flag{};
  float conform_size{1.0};
  bool zero_ge_z_offset_flag{}; // E/
  int nskip{};                  // number of frames to skip from start
  int ndrop{};                  // number of frames to skip from end
  VOL_GEOM vgtmp;
  LT *lt{};
  int DevXFM{};
  std::string devxfm_subject{};
  MATRIX *T{};
  float scale_factor{1};
  float out_scale_factor{1};
  float rescale_factor{1};
  int nthframe{-1};
  int reduce{};
  float fwhm{-1};
  float gstd{-1};
  std::string cmdline{};
  bool sphinx_flag{};
  bool LeftRightReverse{};
  bool LeftRightReversePix{};
  bool LeftRightMirrorFlag{};        // mirror half of the image
  std::string LeftRightMirrorHemi{}; // which half to mirror (lh, rh)
  bool LeftRightKeepFlag{};          // keep half of the image
  bool LeftRightSwapLabel{};
  bool FlipCols{};
  bool SliceReverse{};
  bool SliceBias{};
  float SliceBiasAlpha{1.0};
  float v{};
  std::string AutoAlignFile{};
  MATRIX *AutoAlign{};
  MATRIX *cras{};
  MATRIX *vmid{};
  int ascii_flag{};
  int c{};
  int r{};
  int s{};
  int f{};
  int c1{};
  int c2{};
  int r1{};
  int r2{};
  int s1{};
  int s2{};
  bool InStatTableFlag{};
  bool OutStatTableFlag{};
  int UpsampleFlag{};
  int UpsampleFactor{};
  arma::mat A = arma::randu<arma::mat>(4, 5);

  FSinit();
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  make_cmd_version_string(
      argc, argv, "$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $",
      "$Name:  $", cmdline.data());

  for (i = 0; i < argc; i++) {
    fmt::printf("%s ", argv[i]);
  }
  fmt::printf("\n");
  fflush(stdout);

  crop_size.fill(256);
  crop_center.fill(128);
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
  STAT_TABLE *StatTable{};
  STAT_TABLE *OutStatTable{};

  /* rkt: check for and handle version tag */
  nargs = handle_version_option(
      argc, argv, "$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $",
      "$Name:  $");
  if ((nargs != 0) && argc - nargs == 1) {
    exit(0);
  }
  argc -= nargs;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--ascii") == 0) {
      ascii_flag = 1;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "--ascii+crsf") == 0) {
      ascii_flag = 2;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "--ascii-fcol") == 0) {
      ascii_flag = 3;
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "-iid") == 0 ||
               strcmp(argv[i], "--in_i_direction") == 0) {
      get_doubles(argc, argv, &i, in_i_directions.data(), 3);

      magnitude = fs::math::frobenius_norm(&in_i_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_i_direction = (%g, %g, %g)\n",
                     Progname, in_i_directions[0], in_i_directions[1],
                     in_i_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_i_direction: (%g, %g, %g) -> ",
                    in_i_directions[0], in_i_directions[1], in_i_directions[2]);
        fs::math::frobenius_normalize(&in_i_directions);
        fmt::printf("(%g, %g, %g)\n", in_i_directions[0], in_i_directions[1],
                    in_i_directions[2]);
      }
      in_i_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ijd") == 0 ||
               strcmp(argv[i], "--in_j_direction") == 0) {
      get_doubles(argc, argv, &i, in_j_directions.data(), 3);
      magnitude = fs::math::frobenius_norm(&in_j_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_j_direction = (%g, %g, %g)\n",
                     Progname, in_j_directions[0], in_j_directions[1],
                     in_j_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_j_direction: (%g, %g, %g) -> ",
                    in_j_directions[0], in_j_directions[1], in_j_directions[2]);
        fs::math::frobenius_normalize(&in_j_directions);
        fmt::printf("(%g, %g, %g)\n", in_j_directions[0], in_j_directions[1],
                    in_j_directions[2]);
      }
      in_j_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ikd") == 0 ||
               strcmp(argv[i], "--in_k_direction") == 0) {
      get_doubles(argc, argv, &i, in_k_directions.data(), 3);
      magnitude = fs::math::frobenius_norm(&in_k_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "in_k_direction = (%g, %g, %g)\n",
                     Progname, in_k_directions[0], in_k_directions[1],
                     in_k_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing in_k_direction: (%g, %g, %g) -> ",
                    in_k_directions[0], in_k_directions[1], in_k_directions[2]);
        fs::math::frobenius_normalize(&in_k_directions);
        fmt::printf("(%g, %g, %g)\n", in_k_directions[0], in_k_directions[1],
                    in_k_directions[2]);
      }
      in_k_direction_flag = TRUE;
    } else if (strcmp(argv[i], "--in_orientation") == 0) {
      get_string(argc, argv, &i, in_orientation_string.data());
      errmsg = MRIcheckOrientationString(in_orientation_string.data());
      if (errmsg != nullptr) {
        fmt::printf("ERROR: with in orientation string %s\n",
                    in_orientation_string.data());
        fmt::printf("%s\n", errmsg);
        exit(1);
      }
      in_orientation_flag = TRUE;
    }

    else if (strcmp(argv[i], "--out_orientation") == 0) {
      get_string(argc, argv, &i, out_orientation_string.data());
      errmsg = MRIcheckOrientationString(out_orientation_string.data());
      if (errmsg != nullptr) {
        fmt::printf("ERROR: with out_orientation string %s\n",
                    out_orientation_string.data());
        fmt::printf("%s\n", errmsg);
        exit(1);
      }
      out_orientation_flag = TRUE;
    }

    else if (strcmp(argv[i], "-oid") == 0 ||
             strcmp(argv[i], "--out_i_direction") == 0) {
      get_doubles(argc, argv, &i, out_i_directions.data(), 3);
      magnitude = fs::math::frobenius_norm(&out_i_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_i_direction = (%g, %g, %g)\n",
                     Progname, out_i_directions[0], out_i_directions[1],
                     out_i_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_i_direction: (%g, %g, %g) -> ",
                    out_i_directions[0], out_i_directions[1],
                    out_i_directions[2]);
        fs::math::frobenius_normalize(&out_i_directions);
        fmt::printf("(%g, %g, %g)\n", out_i_directions[0], out_i_directions[1],
                    out_i_directions[2]);
      }
      out_i_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ojd") == 0 ||
               strcmp(argv[i], "--out_j_direction") == 0) {
      get_doubles(argc, argv, &i, out_j_directions.data(), 3);
      magnitude = fs::math::frobenius_norm(&out_j_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_j_direction = (%g, %g, %g)\n",
                     Progname, out_j_directions[0], out_j_directions[1],
                     out_j_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_j_direction: (%g, %g, %g) -> ",
                    out_j_directions[0], out_j_directions[1],
                    out_j_directions[2]);
        fs::math::frobenius_normalize(&out_j_directions);
        fmt::printf("(%g, %g, %g)\n", out_j_directions[0], out_j_directions[1],
                    out_j_directions[2]);
      }
      out_j_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-okd") == 0 ||
               strcmp(argv[i], "--out_k_direction") == 0) {
      get_doubles(argc, argv, &i, out_k_directions.data(), 3);
      magnitude = fs::math::frobenius_norm(&out_k_directions);
      if (magnitude == 0.0) {
        fmt::fprintf(stderr,
                     "\n%s: directions must have non-zero magnitude; "
                     "out_k_direction = (%g, %g, %g)\n",
                     Progname, out_k_directions[0], out_k_directions[1],
                     out_k_directions[2]);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      if (magnitude != 1.0) {
        fmt::printf("normalizing out_k_direction: (%g, %g, %g) -> ",
                    out_k_directions[0], out_k_directions[1],
                    out_k_directions[2]);
        fs::math::frobenius_normalize(&out_k_directions);
        fmt::printf("(%g, %g, %g)\n", out_k_directions[0], out_k_directions[1],
                    out_k_directions[2]);
      }
      out_k_direction_flag = TRUE;
    } else if (strcmp(argv[i], "-ic") == 0 ||
               strcmp(argv[i], "--in_center") == 0) {
      get_floats(argc, argv, &i, in_center.data(), 3);
      in_center_flag = TRUE;
    } else if (strcmp(argv[i], "-dic") == 0 ||
               strcmp(argv[i], "--delta_in_center") == 0) {
      get_floats(argc, argv, &i, delta_in_center.data(), 3);
      delta_in_center_flag = TRUE;
    } else if (strcmp(argv[i], "-oc") == 0 ||
               strcmp(argv[i], "--out_center") == 0) {
      get_floats(argc, argv, &i, out_center.data(), 3);
      out_center_flag = TRUE;
    } else if (strcmp(argv[i], "-vs") == 0 ||
               strcmp(argv[i], "--voxsize") == 0 ||
               strcmp(argv[i], "-voxsize") == 0) {
      get_floats(argc, argv, &i, voxel_size.data(), 3);
      voxel_size_flag = TRUE;
    } else if (strcmp(argv[i], "-ds") == 0 ||
               strcmp(argv[i], "--downsample") == 0 ||
               strcmp(argv[i], "-downsample") == 0) {
      get_floats(argc, argv, &i, downsample_factor.data(), 3);
      downsample_flag = TRUE;
    } else if (strcmp(argv[i], "--fwhm") == 0) {
      get_floats(argc, argv, &i, &fwhm, 1);
      gstd = fwhm / sqrt(log(256.0));
      fmt::printf("fwhm = %g, gstd = %g\n", fwhm, gstd);
    } else if (strcmp(argv[i], "--reduce") == 0) {
      get_ints(argc, argv, &i, &reduce, 1);
      fmt::printf("reducing input image %d times\n", reduce);
    } else if (strcmp(argv[i], "-odt") == 0 ||
               strcmp(argv[i], "--out_data_type") == 0) {
      get_string(argc, argv, &i, out_data_type_string.data());
      if (strcmp(StrLower(out_data_type_string.data()), "uchar") == 0) {
        out_data_type = MRI_UCHAR;
      } else if (strcmp(StrLower(out_data_type_string.data()), "short") == 0) {
        out_data_type = MRI_SHORT;
      } else if (strcmp(StrLower(out_data_type_string.data()), "int") == 0) {
        out_data_type = MRI_INT;
      } else if (strcmp(StrLower(out_data_type_string.data()), "float") == 0) {
        out_data_type = MRI_FLOAT;
      } else if (strcmp(StrLower(out_data_type_string.data()), "rgb") == 0) {
        out_data_type = MRI_RGB;
      } else {
        fmt::fprintf(stderr, "\n%s: unknown data type \"%s\"\n", Progname,
                     argv[i]);
        fs::utils::cli::usage_message(stdout, Progname);
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
      get_string(argc, argv, &i, resample_type.data());
      if (strcmp(StrLower(resample_type.data()), "interpolate") == 0) {
        resample_type_val = SAMPLE_TRILINEAR;
      } else if (strcmp(StrLower(resample_type.data()), "nearest") == 0) {
        resample_type_val = SAMPLE_NEAREST;
      } else if (strcmp(StrLower(resample_type.data()), "vote") == 0) {
        resample_type_val = SAMPLE_VOTE;
      } else if (strcmp(StrLower(resample_type.data()), "weighted") == 0) {
        resample_type_val = SAMPLE_WEIGHTED;
      }
      /*       else if(strcmp(StrLower(resample_type), "sinc") == 0)
            {
              resample_type_val = SAMPLE_SINC;
            }*/
      else if (strcmp(StrLower(resample_type.data()), "cubic") == 0) {
        resample_type_val = SAMPLE_CUBIC_BSPLINE;
      } else {
        fmt::fprintf(stderr, "\n%s: unknown resample type \"%s\"\n", Progname,
                     argv[i]);
        fs::utils::cli::usage_message(stdout, Progname);
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
      get_string(argc, argv, &i, out_type_string.data());
      forced_out_type = string_to_type(out_type_string.data());
      /* see mri_identify.c */
      force_out_type_flag = TRUE;
    } else if (strcmp(argv[i], "-tt") == 0 ||
               strcmp(argv[i], "--template_type") == 0) {
      get_string(argc, argv, &i, template_type_string.data());
      forced_template_type = string_to_type(template_type_string.data());
      force_template_type_flag = TRUE;
    } else if (strcmp(argv[i], "-sn") == 0 ||
               strcmp(argv[i], "--subject_name") == 0) {
      get_string(argc, argv, &i, subject_name.data());
    } else if (strcmp(argv[i], "-gis") == 0 ||
               strcmp(argv[i], "--gdf_image_stem") == 0) {
      get_string(argc, argv, &i, gdf_image_stem.data());
      if (gdf_image_stem.empty()) {
        fmt::fprintf(stderr, "\n%s: zero length GDF image stem given\n",
                     Progname);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
    } else if (strcmp(argv[i], "-rl") == 0 ||
               strcmp(argv[i], "--reslice_like") == 0) {
      get_string(argc, argv, &i, reslice_like_name.data());
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

      get_ints(argc, argv, &i, frames.data(), nframes);
      frame_flag = TRUE;
    } else if (strcmp(argv[i], "--slice-bias") == 0) {
      get_floats(argc, argv, &i, &SliceBiasAlpha, 1);
      SliceBias = TRUE;
    } else if (strcmp(argv[i], "-il") == 0 ||
               strcmp(argv[i], "--in_like") == 0) {
      get_string(argc, argv, &i, in_like_name.data());
      in_like_flag = TRUE;
    } else if (strcmp(argv[i], "-sp") == 0 ||
               strcmp(argv[i], "--smooth_parcellation") == 0) {
      get_ints(argc, argv, &i, &smooth_parcellation_count, 1);
      if (smooth_parcellation_count < 14 || smooth_parcellation_count > 26) {
        fmt::fprintf(stderr,
                     "\n%s: clean parcellation count must "
                     "be between 14 and 26, inclusive\n",
                     Progname);
        fs::utils::cli::usage_message(stdout, Progname);
        exit(1);
      }
      smooth_parcellation_flag = TRUE;
    } else if (strcmp(argv[i], "-cf") == 0 ||
               strcmp(argv[i], "--color_file") == 0) {
      get_string(argc, argv, &i, color_file_name.data());
      color_file_flag = TRUE;
    } else if (strcmp(argv[i], "-ns") == 0 ||
               strcmp(argv[i], "--no_scale") == 0) {
      get_ints(argc, argv, &i, &no_scale_flag, 1);
      // no_scale_flag = (no_scale_flag == 0 ? FALSE : TRUE);
    } else if (strcmp(argv[i], "-cg") == 0 ||
               strcmp(argv[i], "--crop_gdf") == 0) {
      mriio_set_gdf_crop_flag(TRUE);
    }
  }
  /**** Finished parsing command line ****/
  /* option inconsistency checks */
  if (((force_ras_good)) && ((in_i_direction_flag) || (in_j_direction_flag) ||
                             (in_k_direction_flag))) {
    fmt::fprintf(stderr, "ERROR: cannot use --force_ras_good and "
                         "--in_?_direction_flag\n");
    exit(1);
  }
  if (static_cast<int>(conform_flag) == FALSE &&
      static_cast<int>(conform_min) == TRUE) {
    fmt::fprintf(stderr, "In order to use -cm (--conform_min), "
                         "you must set -c (--conform)  at the same time.\n");
    exit(1);
  }

  /* ----- catch zero or negative voxel dimensions ----- */

  sizes_good_flag = TRUE;

  if (((in_i_size_flag) && in_i_size <= 0.0) ||
      ((in_j_size_flag) && in_j_size <= 0.0) ||
      ((in_k_size_flag) && in_k_size <= 0.0) ||
      ((out_i_size_flag) && out_i_size <= 0.0) ||
      ((out_j_size_flag) && out_j_size <= 0.0) ||
      ((out_k_size_flag) && out_k_size <= 0.0)) {
    fmt::fprintf(stderr,
                 "\n%s: voxel sizes must be "
                 "greater than zero\n",
                 Progname);
    sizes_good_flag = FALSE;
  }

  if ((in_i_size_flag) && in_i_size <= 0.0) {
    fmt::fprintf(stderr, "in i size = %g\n", in_i_size);
  }
  if ((in_j_size_flag) && in_j_size <= 0.0) {
    fmt::fprintf(stderr, "in j size = %g\n", in_j_size);
  }
  if ((in_k_size_flag) && in_k_size <= 0.0) {
    fmt::fprintf(stderr, "in k size = %g\n", in_k_size);
  }
  if ((out_i_size_flag) && out_i_size <= 0.0) {
    fmt::fprintf(stderr, "out i size = %g\n", out_i_size);
  }
  if ((out_j_size_flag) && out_j_size <= 0.0) {
    fmt::fprintf(stderr, "out j size = %g\n", out_j_size);
  }
  if ((out_k_size_flag) && out_k_size <= 0.0) {
    fmt::fprintf(stderr, "out k size = %g\n", out_k_size);
  }

  if (sizes_good_flag) {
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  if (out_name[0] == '\0' && !((read_only_flag) || (no_write_flag))) {
    fmt::fprintf(stderr, "\n%s: missing output volume name\n", Progname);
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  /* ---- catch no --like flag for OutStatTableFlag ----- */
  if ((OutStatTableFlag) && out_like_flag) {
    fmt::fprintf(
        stderr,
        "\n%s: pass example (or empty) stats-table with --like to specify "
        "measure, column and row headers\n",
        Progname);
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  /* ----- copy file name (only -- strip '@' and '#') ----- */
  MRIgetVolumeName(in_name.data(), in_name_only.data());

  /* If input type is spm and N_Zero_Pad_Input < 0, set to 3*/
  if (in_type_string == "spm" && N_Zero_Pad_Input < 0) {
    N_Zero_Pad_Input = 3;
  }

  /* ----- catch unknown volume types ----- */
  if ((force_in_type_flag) && forced_in_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr,
                 "\n%s: unknown input "
                 "volume type %s\n",
                 Progname, in_type_string.data());
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  if ((force_template_type_flag) &&
      forced_template_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr, "\n%s: unknown template volume type %s\n", Progname,
                 template_type_string.data());
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  if ((force_out_type_flag) && forced_out_type == MRI_VOLUME_TYPE_UNKNOWN &&
      (ascii_flag == 0)) {
    fmt::fprintf(stderr, "\n%s: unknown output volume type %s\n", Progname,
                 out_type_string.data());
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  /* ----- warn if read only is desired and an
     output volume is specified or the output info flag is set ----- */
  if ((read_only_flag) && out_name[0] != '\0') {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "nothing will be written to %s\n",
                 Progname, out_name.data());
  }
  if ((read_only_flag) && ((out_info_flag) || (out_matrix_flag))) {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "no output information will be printed\n",
                 Progname);
  }

  /* ----- get the type of the output ----- */
  if (!read_only_flag) {
    if ((!force_out_type_flag) && (!OutStatTableFlag)) {
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
  if (parse_only_flag) {
    fmt::printf("input volume name: %s\n", in_name.data());
    fmt::printf("input name only: %s\n", in_name_only.data());
    fmt::printf("output volume name: %s\n", out_name.data());
    fmt::printf("parse_only_flag = %d\n", parse_only_flag);
    fmt::printf("conform_flag = %d\n", conform_flag);
    fmt::printf("conform_size = %f\n", conform_size);
    fmt::printf("in_info_flag = %d\n", in_info_flag);
    fmt::printf("out_info_flag = %d\n", out_info_flag);
    fmt::printf("in_matrix_flag = %d\n", in_matrix_flag);
    fmt::printf("out_matrix_flag = %d\n", out_matrix_flag);

    if (force_in_type_flag) {
      fmt::printf("input type is %d\n", forced_in_type);
    }
    if (force_out_type_flag) {
      fmt::printf("output type is %d\n", forced_out_type);
    }

    if (subject_name[0] != '\0') {
      fmt::printf("subject name is %s\n", subject_name.data());
    }

    if (invert_val >= 0) {
      fmt::printf("inversion, value is %g\n", invert_val);
    }

    if (reorder_flag) {
      fmt::printf("reordering, values are %d %d %d\n", reorder_vals[0],
                  reorder_vals[1], reorder_vals[2]);
    }

    if (reorder4_flag) {
      fmt::printf("reordering, values are %d %d %d %d\n", reorder4_vals[0],
                  reorder4_vals[1], reorder4_vals[2], reorder4_vals[3]);
    }

    fmt::printf("translation of otl labels is %s\n",
                translate_labels_flag ? "on" : "off");

    exit(0);
  }

  /* ----- check for a gdf image stem if the output type is gdf ----- */
  if (out_volume_type == GDF_FILE && gdf_image_stem.empty()) {
    fmt::fprintf(stderr,
                 "%s: GDF output type, "
                 "but no GDF image file stem\n",
                 Progname);
    exit(1);
  }

  /* ----- read the in_like volume ----- */
  if (in_like_flag) {
    fmt::printf("reading info from %s...\n", in_like_name.data());
    mri_in_like = MRIreadInfo(in_like_name.data());
    if (mri_in_like == nullptr) {
      exit(1);
    }
  }

  /* ----- read the volume ----- */
  in_volume_type = MRI_VOLUME_TYPE_UNKNOWN;
  if (!InStatTableFlag) {
    if (force_in_type_flag) {
      in_volume_type = forced_in_type;
    } else {
      in_volume_type = mri_identify(in_name_only.data());
    }
    if (in_volume_type == MRI_VOLUME_TYPE_UNKNOWN) {
      errno = 0;
      if (fio_FileExistsReadable(in_name_only.data()) == 0) {
        fmt::printf("ERROR: file %s does not exist\n", in_name_only.data());
      } else {
        fmt::printf("ERROR: cannot determine file type for %s \n",
                    in_name_only.data());
      }
      if (in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }
  }

  if ((roi_flag) && in_volume_type != GENESIS_FILE) {
    errno = 0;
    ErrorPrintf(ERROR_BADPARM, "rois must be in GE format");
    if (in_like_flag) {
      MRIfree(&mri_in_like);
    }
    exit(1);
  }

  if (((zero_ge_z_offset_flag)) && in_volume_type != DICOM_FILE) // E/
  {
    zero_ge_z_offset_flag = FALSE;
    fmt::fprintf(stderr, "Not a GE dicom volume: -zgez "
                         "= --zero_ge_z_offset option ignored.\n");
  }

  fmt::printf("$Id: mri_convert.c,v 1.227 2017/02/16 19:15:42 greve Exp $\n");
  fmt::printf("reading from %s...\n", in_name_only.data());

  if (in_volume_type == MGH_MORPH) {
    GCA_MORPH *gcam;
    GCA_MORPH *gcam_out;
    gcam = GCAMread(in_name_only.data());
    if (gcam == nullptr) {
      ErrorExit(ERROR_NOFILE, "%s: could not read input morph from %s",
                Progname, in_name_only.data());
    }
    if (downsample2_flag) {
      gcam_out = GCAMdownsample2(gcam);
    } else {
      gcam_out = gcam;
    }

    GCAMwrite(gcam_out, out_name.data());
    exit(0);
  }

  if (in_volume_type == OTL_FILE) {

    if ((!in_like_flag) && (!in_n_k_flag)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM,
                  "parcellation read: must specify"
                  "a volume depth with either in_like or in_k_count");
      exit(1);
    }

    if (!color_file_flag) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "parcellation read: must specify a"
                                 "color file name");
      if (in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    read_parcellation_volume_flag = TRUE;
    if ((read_only_flag) && ((in_info_flag) || (in_matrix_flag)) &&
        (!in_stats_flag)) {
      read_parcellation_volume_flag = FALSE;
    }

    read_otl_flags = 0x00;

    if (read_parcellation_volume_flag) {
      read_otl_flags |= READ_OTL_READ_VOLUME_FLAG;
    }

    if (fill_parcellation_flag) {
      read_otl_flags |= READ_OTL_FILL_FLAG;
    } else {
      fmt::printf("notice: unfilled parcellations "
                  "currently unimplemented\n");
      fmt::printf("notice: filling outlines\n");
      read_otl_flags |= READ_OTL_FILL_FLAG;
    }

    if (translate_labels_flag) {
      read_otl_flags |= READ_OTL_TRANSLATE_LABELS_FLAG;
    }

    if (zero_outlines_flag) {
      read_otl_flags |= READ_OTL_ZERO_OUTLINES_FLAG;
    }

    if (in_like_flag) {
      mri = MRIreadOtl(in_name.data(), mri_in_like->width, mri_in_like->height,
                       mri_in_like->depth, color_file_name.data(),
                       read_otl_flags);
    } else {
      mri = MRIreadOtl(in_name.data(), 0, 0, in_n_k, color_file_name.data(),
                       read_otl_flags);
    }

    if (mri == nullptr) {
      if (in_like_flag) {
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
    if (smooth_parcellation_flag) {
      fmt::printf("smoothing parcellation...\n");
      mri2 = MRIsmoothParcellation(mri, smooth_parcellation_count);
      if (mri2 == nullptr) {
        if (in_like_flag) {
          MRIfree(&mri_in_like);
        }
        exit(1);
      }
      MRIfree(&mri);
      mri = mri2;
    }

    resample_type_val = SAMPLE_NEAREST;
    no_scale_flag = TRUE;

  } else if (roi_flag) {
    if ((!in_like_flag) && (!in_n_k_flag)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "roi read: must specify a volume"
                                 "depth with either in_like or in_k_count");
      if (in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    if (in_like_flag) {
      mri = MRIreadGeRoi(in_name.data(), mri_in_like->depth);
    } else {
      mri = MRIreadGeRoi(in_name.data(), in_n_k);
    }

    if (mri == nullptr) {
      if (in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    resample_type_val = SAMPLE_NEAREST;
    no_scale_flag = TRUE;
  } else {
    if ((read_only_flag) && ((in_info_flag) || (in_matrix_flag)) &&
        (!in_stats_flag)) {
      if (force_in_type_flag) {
        mri = MRIreadHeader(in_name.data(), in_volume_type);
      } else {
        mri = MRIreadInfo(in_name.data());
      }
    } else {
      if (force_in_type_flag) {
        // fmt::printf("MRIreadType()\n");
        mri = MRIreadType(in_name.data(), in_volume_type);
      } else {
        if (nthframe < 0) {
          if (!InStatTableFlag) {
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
    if (in_like_flag) {
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

  if (slice_crop_flag) {
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

  if (LeftRightReverse) {
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

    MRIdircosToOrientationString(mri, ostr.data());
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

  if (LeftRightSwapLabel) {
    fmt::printf("Performing left-right swap of labels\n");
    // Good for aseg, aparc+aseg, wmparc, etc. Does not change geometry
    mri2 = MRIlrswapAseg(mri);
    MRIfree(&mri);
    mri = mri2;
  }

  if (LeftRightReversePix) {
    // Performs a left-right reversal of the pixels by finding the
    // dimension that is most left-right oriented and reversing
    // the order of the pixels. The geometry itself is not
    // changed.
    fmt::printf("WARNING: applying left-right reversal to the input pixels\n"
                "without changing geometry. This will likely make \n"
                "the volume geometry WRONG, so make sure you know what you  \n"
                "are doing.\n");

    MRIdircosToOrientationString(mri, ostr.data());
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

  if ((LeftRightMirrorFlag) || (LeftRightKeepFlag)) //(mr, 2012-03-08)
  {
    // Mirror one half of the image into the other half (left-right)
    // the split point is at the middle of the dimension that is
    // most left-right oriented. The header geometry is not changed
    // If LeftRightKeep, then don't mirror but fill with zero and
    // only keep the specified hemisphere
    fmt::printf("WARNING: Mirroring %s half into the other half,\n"
                "or masking one half of the image is only meaningful if\n"
                "the image is upright and centerd, see make_upright.\n",
                LeftRightMirrorHemi.data());

    MRIdircosToOrientationString(mri, ostr.data());
    fmt::printf("  Orientation string: %s\n", ostr.data());
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
              if (LeftRightMirrorFlag) {
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
              if (LeftRightMirrorFlag) {
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
              if (LeftRightMirrorFlag) {
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

  if (FlipCols) {
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

  if (SliceReverse) {
    fmt::printf("Reversing slices, updating vox2ras\n");
    mri2 = MRIreverseSlices(mri, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (SliceBias) {
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

  MRIaddCommandLine(mri, cmdline.data());
  if (!FEQUAL(scale_factor, 1.0)) {
    fmt::printf("scaling input volume by %2.3f\n", scale_factor);
    MRIscalarMul(mri, mri, scale_factor);
  }

  if (zero_ge_z_offset_flag) // E/
  {
    mri->c_s = 0.0;
  }

  fmt::printf("TR=%2.2f, TE=%2.2f, TI=%2.2f, flip angle=%2.2f\n", mri->tr,
              mri->te, mri->ti, DEGREES(mri->flip_angle));
  if (in_volume_type != OTL_FILE) {
    if (fill_parcellation_flag) {
      fmt::printf("fill_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
    if (smooth_parcellation_flag) {
      fmt::printf("smooth_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
  }

  /* ----- apply the in_like volume if it's been read ----- */
  if (in_like_flag) {
    if (mri->width != mri_in_like->width ||
        mri->height != mri_in_like->height ||
        mri->depth != mri_in_like->depth) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "volume sizes do not match\n");
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  in_name.c_str(), mri->width, mri->height, mri->depth,
                  mri->nframes);
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  in_like_name.c_str(), mri_in_like->width, mri_in_like->height,
                  mri_in_like->depth, mri_in_like->nframes);
      MRIfree(&mri);
      MRIfree(&mri_in_like);
      exit(1);
    }
    if (mri->nframes != mri_in_like->nframes) {
      fmt::printf("INFO: frames are not the same\n");
    }
    temp_type = mri->type;

    mritmp = MRIcopyHeader(mri_in_like, mri);
    if (mritmp == nullptr) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM,
                  "error copying information from "
                  "%s structure to %s structure\n",
                  in_like_name.c_str(), in_name.data());
      MRIfree(&mri);
      MRIfree(&mri_in_like);
      exit(1);
    }

    mri->type = temp_type;
    MRIfree(&mri_in_like);
  }

  if (mri->ras_good_flag == 0) {
    fmt::printf("WARNING: it does not appear that there "
                "was sufficient information\n"
                "in the input to assign orientation to the volume... \n");
    if (force_ras_good) {
      fmt::printf("However, you have specified that the "
                  "default orientation should\n"
                  "be used with by adding --force_ras_good "
                  "on the command-line.\n");
      mri->ras_good_flag = 1;
    }
    if ((in_i_direction_flag) || (in_j_direction_flag) ||
        (in_k_direction_flag)) {
      fmt::printf("However, you have specified one or more "
                  "orientations on the \n"
                  "command-line using -i?d or --in-?-direction (?=i,j,k).\n");
      mri->ras_good_flag = 1;
    }
  }

  /* ----- apply command-line parameters ----- */
  if (in_i_size_flag) {
    mri->xsize = in_i_size;
  }
  if (in_j_size_flag) {
    mri->ysize = in_j_size;
  }
  if (in_k_size_flag) {
    mri->zsize = in_k_size;
  }
  if (in_i_direction_flag) {
    mri->x_r = in_i_directions[0];
    mri->x_a = in_i_directions[1];
    mri->x_s = in_i_directions[2];
    mri->ras_good_flag = 1;
  }
  if (in_j_direction_flag) {
    mri->y_r = in_j_directions[0];
    mri->y_a = in_j_directions[1];
    mri->y_s = in_j_directions[2];
    mri->ras_good_flag = 1;
  }
  if (in_k_direction_flag) {
    mri->z_r = in_k_directions[0];
    mri->z_a = in_k_directions[1];
    mri->z_s = in_k_directions[2];
    mri->ras_good_flag = 1;
  }
  if (DeleteCMDs != 0) {
    mri->ncmds = 0;
  }
  if (DoNewTransformFname != 0) {
    fmt::printf("Changing xform name to %s\n", NewTransformFname.data());
    strcpy(mri->transform_fname, NewTransformFname.data());
  }
  if (in_orientation_flag) {
    fmt::printf("Setting input orientation to %s\n",
                in_orientation_string.data());
    MRIorientationStringToDircos(mri, in_orientation_string.data());
    mri->ras_good_flag = 1;
  }
  if (in_center_flag) {
    mri->c_r = in_center[0];
    mri->c_a = in_center[1];
    mri->c_s = in_center[2];
  }
  if (delta_in_center_flag) {
    mri->c_r += delta_in_center[0];
    mri->c_a += delta_in_center[1];
    mri->c_s += delta_in_center[2];
  }
  if (subject_name[0] != '\0') {
    strcpy(mri->subject_name, subject_name.data());
  }

  if (in_tr_flag) {
    mri->tr = in_tr;
  }
  if (in_ti_flag) {
    mri->ti = in_ti;
  }
  if (in_te_flag) {
    mri->te = in_te;
  }
  if (in_flip_angle_flag) {
    mri->flip_angle = in_flip_angle;
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((in_i_size_flag) || (in_j_size_flag) || (in_k_size_flag)) {

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
    fmt::printf("warning: input volume axes are not orthogonal:"
                "i_dot_j = %.6f, i_dot_k = %.6f, j_dot_k = %.6f\n",
                i_dot_j, i_dot_k, j_dot_k);
  }
  fmt::printf("i_ras = (%g, %g, %g)\n", mri->x_r, mri->x_a, mri->x_s);
  fmt::printf("j_ras = (%g, %g, %g)\n", mri->y_r, mri->y_a, mri->y_s);
  fmt::printf("k_ras = (%g, %g, %g)\n", mri->z_r, mri->z_a, mri->z_s);

  /* ----- catch the in info flag ----- */
  if (in_info_flag) {
    fmt::printf("input structure:\n");
    MRIdump(mri, stdout);
  }

  if (in_matrix_flag) {
    MATRIX *i_to_r;
    i_to_r = extract_i_to_r(mri);
    if (i_to_r != nullptr) {
      fmt::printf("input ijk -> ras:\n");
      MatrixPrint(stdout, i_to_r);
      MatrixFree(&i_to_r);
    } else {
      fmt::printf("error getting input matrix\n");
    }
  }

  /* ----- catch the in stats flag ----- */
  if (in_stats_flag) {
    MRIprintStats(mri, stdout);
  }

  // Load the transform
  if (transform_flag) {
    fmt::printf("INFO: Reading transformation from file %s...\n",
                transform_fname.data());
    transform_type = TransformFileNameType(transform_fname.data());
    if (transform_type == MNI_TRANSFORM_TYPE ||
        transform_type == TRANSFORM_ARRAY_TYPE ||
        transform_type == REGISTER_DAT || transform_type == FSLREG_TYPE) {
      fmt::printf("Reading transform with LTAreadEx()\n");
      // lta_transform = LTAread(transform_fname);
      lta_transform = LTAreadEx(transform_fname.data());
      if (lta_transform == nullptr) {
        fmt::fprintf(stderr, "ERROR: Reading transform from file %s\n",
                     transform_fname.data());
        exit(1);
      }
      if (transform_type == FSLREG_TYPE) {
        MRI *tmp = nullptr;
        if (!out_like_flag) {
          fmt::printf("ERROR: fslmat does not have the information "
                      "on the dst volume\n");
          fmt::printf(
              "ERROR: you must give option '--like volume' to specify the"
              " dst volume info\n");
          MRIfree(&mri);
          exit(1);
        }
        // now setup dst volume info
        tmp = MRIreadHeader(out_like_name.data(), MRI_VOLUME_TYPE_UNKNOWN);
        // flsmat does not contain src and dst info
        LTAmodifySrcDstGeom(lta_transform, mri, tmp);
        // add src and dst information
        LTAchangeType(lta_transform, LINEAR_VOX_TO_VOX);
        MRIfree(&tmp);
      } // end FSLREG_TYPE

      if (DevXFM != 0) {
        fmt::printf("INFO: devolving XFM (%s)\n", devxfm_subject.data());
        fmt::printf("-------- before ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        T = DevolveXFM(devxfm_subject.data(), lta_transform->xforms[0].m_L,
                       nullptr);
        if (T == nullptr) {
          exit(1);
        }
        fmt::printf("-------- after ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        fmt::printf("-----------------\n");
      } // end DevXFM

      if (invert_transform_flag) {
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
          strcpy(buf, transform_fname.data());
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
            fmt::printf("INFO: Failed to find %s as a source volume.  \n"
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

  if (reslice_like_flag) {

    if (force_template_type_flag) {
      fmt::printf("reading template info from (type %s) volume %s...\n",
                  template_type_string.data(), reslice_like_name.data());
      mri_template =
          MRIreadHeader(reslice_like_name.data(), forced_template_type);
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     reslice_like_name.data());
        exit(1);
      }
    } else {
      fmt::printf("reading template info from volume %s...\n",
                  reslice_like_name.data());
      mri_template = MRIreadInfo(reslice_like_name.data());
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     reslice_like_name.data());
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

    if (conform_flag) {
      conform_width = 256;
      if (conform_min) {
        conform_size = MRIfindMinSize(mri, &conform_width);
      } else {
        if (conform_width_256_flag) {
          conform_width = 256; // force it
        } else {
          conform_width = MRIfindRightSize(mri, conform_size);
        }
      }
      mri_template =
          MRIconformedTemplate(mri, conform_width, conform_size, ConfKeepDC);
    } else if (voxel_size_flag) {
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

    } else if (downsample_flag) {
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
  if (out_i_size_flag) {
    float scale;
    scale = mri_template->xsize / out_i_size;
    mri_template->xsize = out_i_size;
    mri_template->width = nint(mri_template->width * scale);
  }
  if (out_j_size_flag) {
    float scale;
    scale = mri_template->ysize / out_j_size;
    mri_template->ysize = out_j_size;
    mri_template->height = nint(mri_template->height * scale);
  }
  if (out_k_size_flag) {
    float scale;
    scale = mri_template->zsize / out_k_size;
    mri_template->zsize = out_k_size;
    mri_template->depth = nint(mri_template->depth * scale);
  }
  if (out_n_i_flag) {
    mri_template->width = out_n_i;
  }
  if (out_n_j_flag) {
    mri_template->height = out_n_j;
  }
  if (out_n_k_flag) {
    mri_template->depth = out_n_k;
    mri_template->imnr1 = mri_template->imnr0 + out_n_k - 1;
  }
  if (out_i_direction_flag) {
    mri_template->x_r = out_i_directions[0];
    mri_template->x_a = out_i_directions[1];
    mri_template->x_s = out_i_directions[2];
  }
  if (out_j_direction_flag) {
    mri_template->y_r = out_j_directions[0];
    mri_template->y_a = out_j_directions[1];
    mri_template->y_s = out_j_directions[2];
  }
  if (out_k_direction_flag) {
    mri_template->z_r = out_k_directions[0];
    mri_template->z_a = out_k_directions[1];
    mri_template->z_s = out_k_directions[2];
  }
  if (out_orientation_flag) {
    fmt::printf("Setting output orientation to %s\n",
                out_orientation_string.data());
    MRIorientationStringToDircos(mri_template, out_orientation_string.data());
  }
  if (out_center_flag) {
    mri_template->c_r = out_center[0];
    mri_template->c_a = out_center[1];
    mri_template->c_s = out_center[2];
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((out_i_size_flag) || (out_j_size_flag) || (out_k_size_flag) ||
      (out_n_i_flag) || (out_n_j_flag) || (out_n_k_flag)) {

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
    fmt::printf("warning: output volume axes are not orthogonal:"
                "i_dot_j = %.6f, i_dot_k = %.6f, j_dot_k = %.6f\n",
                i_dot_j, i_dot_k, j_dot_k);
    fmt::printf("i_ras = (%g, %g, %g)\n", mri_template->x_r, mri_template->x_a,
                mri_template->x_s);
    fmt::printf("j_ras = (%g, %g, %g)\n", mri_template->y_r, mri_template->y_a,
                mri_template->y_s);
    fmt::printf("k_ras = (%g, %g, %g)\n", mri_template->z_r, mri_template->z_a,
                mri_template->z_s);
  }
  if (out_data_type >= 0) {
    mri_template->type = out_data_type;
  }

  /* ----- catch the mri_template info flag ----- */
  if (template_info_flag) {
    fmt::printf("template structure:\n");
    MRIdump(mri_template, stdout);
  }

  /* ----- exit here if read only is desired ----- */
  if (read_only_flag) {
    exit(0);
  }

  /* ----- apply a transformation if requested ----- */
  if (transform_flag) {
    fmt::printf("INFO: Applying transformation from file %s...\n",
                transform_fname.data());
    transform_type = TransformFileNameType(transform_fname.data());
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

      fmt::printf("---------------------------------\n");
      fmt::printf("INFO: Transform Matrix (%s)\n",
                  LTAtransformTypeName(lta_transform->type));
      MatrixPrint(stdout, lta_transform->xforms[0].m_L);
      fmt::printf("---------------------------------\n");

      /* LTAtransform() runs either MRIapplyRASlinearTransform()
         for RAS2RAS or MRIlinearTransform() for Vox2Vox. */
      /* MRIlinearTransform() calls MRIlinearTransformInterp() */
      if (out_like_flag) {
        MRI *tmp = nullptr;
        fmt::printf(
            "INFO: transform dst into the like-volume (resample_type %d)\n",
            resample_type_val);
        tmp = MRIreadHeader(out_like_name.data(), MRI_VOLUME_TYPE_UNKNOWN);
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
        if (out_center_flag) {
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

        fmt::printf("Applying LTAtransformInterp (resample_type %d)\n",
                    resample_type_val);
        if (Gdiag_no > 0) {
          fmt::printf(
              "Dumping LTA ---------++++++++++++-----------------------\n");
          LTAdump(stdout, lta_transform);
          fmt::printf(
              "========-------------++++++++++++-------------------=====\n");
        }
        mri_transformed =
            LTAtransformInterp(mri, nullptr, lta_transform, resample_type_val);
      } // end out_like_flag treatment

      if (mri_transformed == nullptr) {
        fmt::fprintf(stderr, "ERROR: applying transform to volume\n");
        exit(1);
      }
      if (out_center_flag) {
        mri_transformed->c_r = mri_template->c_r;
        mri_transformed->c_a = mri_template->c_a;
        mri_transformed->c_s = mri_template->c_s;
      }
      LTAfree(&lta_transform);
      MRIfree(&mri);
      mri = mri_transformed;
    } else if (transform_type == MORPH_3D_TYPE) {
      fmt::printf("Applying morph_3d ...\n");
      // this is a non-linear vox-to-vox transform
      // note that in this case trilinear
      // interpolation is always used, and -rt
      // option has no effect! -xh
      TRANSFORM *tran = TransformRead(transform_fname.data());
      // check whether the volume to be morphed and the morph have the same
      // dimensions
      if (tran == nullptr) {
        ErrorExit(ERROR_NOFILE, "%s: could not read xform from %s\n", Progname,
                  transform_fname.c_str());
      }
      if (static_cast<int>(invert_transform_flag) == 0) {
        fmt::printf("morphing to atlas with resample type %d\n",
                    resample_type_val);
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
        fmt::printf("morphing from atlas with resample type %d\n",
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
                   transform_fname.data());
      exit(1);
    }
  } else if (((out_like_flag)) &&
             (!OutStatTableFlag)) // flag set but no transform
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

    fmt::printf("INFO: transform src into the like-volume: %s\n",
                out_like_name.data());
    tmp = MRIreadHeader(out_like_name.data(), MRI_VOLUME_TYPE_UNKNOWN);
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
  if (mri->type != mri_template->type &&
      static_cast<int>(nochange_flag) == FALSE) {
    fmt::printf("changing data type from %s to %s (noscale = %d)...\n",
                MRItype2str(mri->type), MRItype2str(mri_template->type),
                no_scale_flag);
    mri2 = MRISeqchangeType(mri, mri_template->type, 0.0, 0.999, no_scale_flag);
    if (mri2 == nullptr) {
      fmt::printf("ERROR: MRISeqchangeType\n");
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
  fmt::printf(" dim  ( %i , %i , %i ) vs ( %i , %i , %i
  )\n",mri->width,mri->height,mri->depth,mri_template->width,mri_template->height,mri_template->depth);
  fmt::printf(" size ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->xsize,mri->ysize,mri->zsize,mri_template->xsize,mri_template->ysize,mri_template->zsize);
  fmt::printf(" xras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->x_r,mri->x_a,mri->x_s,mri_template->x_r,mri_template->x_a,mri_template->x_s);
  fmt::printf(" yras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->y_r,mri->x_a,mri->y_s,mri_template->y_r,mri_template->y_a,mri_template->y_s);
  fmt::printf(" zras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->z_r,mri->x_a,mri->z_s,mri_template->z_r,mri_template->z_a,mri_template->z_s);
  fmt::printf(" cras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g
  )\n",mri->c_r,mri->c_a,mri->c_s,mri_template->c_r,mri_template->c_a,mri_template->c_s);
  */

  /* ----- reslice if necessary and not performed during transform ----- */
  float eps = 1e-05; /* (mr) do eps testing to avoid reslicing due to tiny
                        differences, e.g. from IO */
  if ((!out_like_flag) && (transform_type != MORPH_3D_TYPE) &&
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
    fmt::printf("Reslicing using ");
    switch (resample_type_val) {
    case SAMPLE_TRILINEAR:
      fmt::printf("trilinear interpolation \n");
      break;
    case SAMPLE_NEAREST:
      fmt::printf("nearest \n");
      break;
      /*    case SAMPLE_SINC:
            fmt::printf("sinc \n");
            break;*/
    case SAMPLE_CUBIC:
      fmt::printf("cubic \n");
      break;
    case SAMPLE_WEIGHTED:
      fmt::printf("weighted \n");
      break;
    case SAMPLE_VOTE:
      fmt::printf("voting \n");
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
    fmt::printf("inverting contrast...\n");
    mri2 = MRIinvertContrast(mri, nullptr, invert_val);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- reorder if necessary ----- */
  if (reorder_flag) {
    fmt::printf("reordering axes...\n");
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
  if (reorder4_flag) {
    fmt::printf("reordering all axes...\n");
    fmt::printf("reordering, values are %d %d %d %d\n", reorder4_vals[0],
                reorder4_vals[1], reorder4_vals[2], reorder4_vals[3]);
    mri2 = MRIreorder4(mri, reorder4_vals.data());
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
  if (out_info_flag) {
    fmt::printf("output structure:\n");
    MRIdump(mri, stdout);
  }

  /* ----- catch the out matrix flag ----- */
  if (out_matrix_flag) {
    MATRIX *i_to_r;
    i_to_r = extract_i_to_r(mri);
    if (i_to_r != nullptr) {
      fmt::printf("output ijk -> ras:\n");
      MatrixPrint(stdout, i_to_r);
      MatrixFree(&i_to_r);
    } else {
      fmt::printf("error getting output matrix\n");
    }
  }

  /* ----- catch the out stats flag ----- */
  if (out_stats_flag) {
    MRIprintStats(mri, stdout);
  }

  if (erode_seg_flag) {
    fmt::printf("Eroding segmentation %d\n", n_erode_seg);
    mri2 = MRIerodeSegmentation(mri, nullptr, n_erode_seg, 0);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (dil_seg_flag) {
    mritmp = nullptr;
    if (dil_seg_mask[0] == '\0') {
      fmt::printf("Dilating segmentation %d\n", n_dil_seg);
    } else {
      fmt::printf("Dilating segmentation, mask %s\n", dil_seg_mask.data());
      mritmp = MRIread(dil_seg_mask.data());
      if (mritmp == nullptr) {
        fmt::printf("ERROR: could not read %s\n", dil_seg_mask.data());
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

  if (frame_flag) {
    if (mid_frame_flag) {
      nframes = 1;
      frames[0] = nint(mri->nframes / 2);
    }
    if (nframes == 1) {
      fmt::printf("keeping frame %d\n", frames[0]);
    } else {
      fmt::printf("keeping frames");
      for (f = 0; f < nframes; f++) {
        fmt::printf(" %d", frames[f]);
      }
      fmt::printf("\n");
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
        fmt::printf("   ERROR: valid frame numbers are between 0 and %d\n",
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
    fmt::printf("Dropping last %d frames\n", ndrop);
    if (mri->nframes <= ndrop) {
      fmt::printf("   ERROR: can't drop, volume only has %d frames\n",
                  mri->nframes);
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
    fmt::printf("Skipping %d frames\n", nskip);
    if (mri->nframes <= nskip) {
      fmt::printf("   ERROR: can't skip, volume only has %d frames\n",
                  mri->nframes);
      exit(1);
    }
    mri2 = fMRInskip(mri, nskip, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (subsample_flag) {
    fmt::printf("SubSample: Start = %d  Delta = %d, End = %d\n", SubSampStart,
                SubSampDelta, SubSampEnd);
    mri2 = fMRIsubSample(mri, SubSampStart, SubSampDelta, SubSampEnd, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cutends_flag) {
    fmt::printf("Cutting ends: n = %d\n", ncutends);
    mri2 = MRIcutEndSlices(mri, ncutends);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- crops ---------*/
  if (crop_flag) {
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
    fmt::printf("scaling output intensities by %2.3f\n", out_scale_factor);
    MRIscalarMul(mri, mri, out_scale_factor);
  }

  if (sphinx_flag) {
    fmt::printf("Changing orientation information to Sphinx\n");
    MRIhfs2Sphinx(mri);
  }

  if (AutoAlign != nullptr) {
    mri->AutoAlign = AutoAlign;
  }

  // ----- modify color lookup table (specified by --ctab option) -----
  if (colortablefile == "remove") {
    // remove an embedded ctab
    if (mri->ct != nullptr) {
      std::cout << "removing color lookup table" << std::endl;
      CTABfree(&mri->ct);
    }
  } else if (!colortablefile.empty()) {
    // add a user-specified ctab
    std::cout << "embedding color lookup table" << std::endl;
    if (mri->ct != nullptr) {
      CTABfree(&mri->ct);
    }
    mri->ct = CTABreadASCII(colortablefile.data());
    if (mri->ct == nullptr) {
      fs::fatal() << "could not read lookup table from "
                  << colortablefile.data();
    }
  }

  /*------ Finally, write the output -----*/

  if (OutStatTableFlag) {
    fmt::printf("Writing as Stats-Table to %s using template %s\n",
                out_name.data(), out_like_name.data());

    OutStatTable = InitStatTableFromMRI(mri, out_like_name.data());
    WriteStatTable(out_name.data(), OutStatTable);

    fmt::printf("done\n");
    exit(0);
  }

  if (ascii_flag != 0) {
    fmt::printf("Writing as ASCII to %s\n", out_name.data());
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
    fmt::printf("done\n");
    exit(0);
  }
  if (static_cast<int>(no_write_flag) == 0) {
    if (SplitFrames == 0) {
      fmt::printf("writing to %s...\n", out_name.data());
      if (force_out_type_flag) {
        err = MRIwriteType(mri, out_name.data(), out_volume_type);
        if (err != NO_ERROR) {
          fmt::printf("ERROR: failure writing %s as volume type %d\n",
                      out_name.data(), out_volume_type);
          exit(1);
        }
      } else {
        err = MRIwrite(mri, out_name.data());
        if (err != NO_ERROR) {
          fmt::printf("ERROR: failure writing %s\n", out_name.data());
          exit(1);
        }
      }
    } else {
      stem = IDstemFromName(out_name.data());
      ext = IDextensionFromName(out_name.data());

      fmt::printf("Splitting frames, stem = %s, ext = %s\n", stem, ext);
      mri2 = nullptr;
      for (i = 0; i < mri->nframes; i++) {
        mri2 = MRIcopyFrame(mri, mri2, i, 0);
        fmt::sprintf(tmpstr.data(), "%s%04d.%s", stem, i, ext);
        fmt::printf("%2d %s\n", i, tmpstr.data());
        err = MRIwrite(mri2, tmpstr.data());
        if (err != NO_ERROR) {
          fmt::printf("ERROR: failure writing %s\n", tmpstr.data());
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

void get_ints(int argc, char *argv[], int *pos, int *vals, int nvals) {
  char *ep;
  int i;

  if (*pos + nvals >= argc) {
    fmt::fprintf(stderr,
                 "\n%s: argument %s expects %d integers; "
                 "only %d arguments after flag\n",
                 Progname, argv[*pos], nvals, argc - *pos - 1);
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  for (i = 0; i < nvals; i++) {
    if (argv[*pos + i + 1][0] == '\0') {
      fmt::fprintf(stderr, "\n%s: argument to %s flag is null\n", Progname,
                   argv[*pos]);
      fs::utils::cli::usage_message(stdout, Progname);
      exit(1);
    }

    vals[i] = static_cast<int>(strtol(argv[*pos + i + 1], &ep, 10));

    if (*ep != '\0') {
      fmt::fprintf(stderr,
                   "\n%s: error converting \"%s\" to an "
                   "integer for %s flag, incorrect # of args (need %d)?\n",
                   Progname, argv[*pos + i + 1], argv[*pos], nvals);
      fs::utils::cli::usage_message(stdout, Progname);
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
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  for (i = 0; i < nvals; i++) {
    if (argv[*pos + i + 1][0] == '\0') {
      fmt::fprintf(stderr, "\n%s: argument to %s flag is null\n", Progname,
                   argv[*pos]);
      fs::utils::cli::usage_message(stdout, Progname);
      exit(1);
    }

    vals[i] = static_cast<float>(strtod(argv[*pos + i + 1], &ep));

    if (*ep != '\0') {
      fmt::fprintf(stderr,
                   "\n%s: error converting \"%s\" to a "
                   "float for %s flag: incorrect # of args? Need %d\n",
                   Progname, argv[*pos + i + 1], argv[*pos], nvals);
      fs::utils::cli::usage_message(stdout, Progname);
      exit(1);
    }
  }

  *pos += nvals;

} /* end get_floats() */

void get_doubles(int argc, char *argv[], int *pos, double *vals, int nvals) {

  char *ep;
  int i;

  if (*pos + nvals >= argc) {
    fmt::fprintf(stderr,
                 "\n%s: argument %s expects %d floats; "
                 "only %d arguments after flag\n",
                 Progname, argv[*pos], nvals, argc - *pos - 1);
    fs::utils::cli::usage_message(stdout, Progname);
    exit(1);
  }

  for (i = 0; i < nvals; i++) {
    if (argv[*pos + i + 1][0] == '\0') {
      fmt::fprintf(stderr, "\n%s: argument to %s flag is null\n", Progname,
                   argv[*pos]);
      fs::utils::cli::usage_message(stdout, Progname);
      exit(1);
    }

    vals[i] = strtod(argv[*pos + i + 1], &ep);

    if (*ep != '\0') {
      fmt::fprintf(stderr,
                   "\n%s: error converting \"%s\" to a "
                   "float for %s flag: incorrect # of args? Need %d\n",
                   Progname, argv[*pos + i + 1], argv[*pos], nvals);
      fs::utils::cli::usage_message(stdout, Progname);
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
    fs::utils::cli::usage_message(stdout, Progname);
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

static auto good_cmdline_args(CMDARGS *cmdargs, ENV *env) noexcept -> bool {

  namespace po = boost::program_options;
  using opt_deps = std::multimap<std::string, std::string>;

  po::options_description desc(
      "\nUSAGE: mri_convert_exec [options] <in volume> <out volume>\n"
      "\n\nAvailable Options");
  po::positional_options_description pos;
  pos.add("in_name", 1).add("out_name", 1);
  po::variables_map vm;

  initArgDesc(&desc, cmdargs);
  auto args = cmdargs->raw;
  auto ac = static_cast<int>(args.size());
  auto av = args.data();
  if (ac == 1) {
    print_usage(desc, env);
    return false;
  }

  try {
    auto parsed_opts = po::command_line_parser(ac, av)
                           .options(desc)
                           .positional(pos)
                           .style(fs::utils::cli::po_style)
                           .run();
    po::store(parsed_opts, vm);

    opt_deps conflicts;
    opt_deps dependant_opts;

    conflicts.insert(std::make_pair("reorder4", "r4"));
    conflicts.insert(std::make_pair("outside_val", "oval"));
    conflicts.insert(std::make_pair("nochange", "nc"));
    conflicts.insert(std::make_pair("conform_min", "cm"));
    conflicts.insert(std::make_pair("conform_size", "cs"));
    conflicts.insert(std::make_pair("apply_transform", "at"));
    conflicts.insert(std::make_pair("apply_inverse_transform", "ait"));
    conflicts.insert(std::make_pair("nth_frame", "nth"));
    conflicts.insert(std::make_pair("no_zero_ge_z_offset", "nozgez"));
    conflicts.insert(std::make_pair("no_zero_ge_z_offset", "zero_ge_z_offset"));
    conflicts.insert(std::make_pair("no_zero_ge_z_offset", "zgez"));
    conflicts.insert(std::make_pair("zero_ge_z_offset", "zgez"));
    conflicts.insert(std::make_pair("zero_ge_z_offset", "no_zero_ge_z_offset"));
    conflicts.insert(std::make_pair("zero_ge_z_offset", "nozgez"));

    for (auto const &[key, val] : conflicts) {
      po::conflicting_options(vm, key, val);
    }

    for (auto const &[key, val] : dependant_opts) {
      po::option_dependency(vm, key, val);
    }

  } catch (std::exception const &e) {
    spdlog::get("stderr")->critical(e.what());
    return false;
  }

  if (vm.count("version2") != 0U) {
    exit(97);
  }

  if (vm.count("help") != 0U || vm.count("usage") != 0U) {
    usage(nullptr);
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

  if (vm.count("reorder_vals") != 0U) {
    auto vals = vm["reorder_vals"].as<std::vector<int>>();
    if (vals.size() != 3) {
      spdlog::get("stderr")->critical(
          "Incorrect number of values, reorder_vals expects 3 values");
      return false;
    }
    cmdargs->reorder_flag = true;
  }

  if (vm.count("reorder4_vals") != 0U) {
    auto vals = vm["reorder4_vals"].as<std::vector<int>>();
    if (vals.size() != 4) {
      spdlog::get("stderr")->critical(
          "Incorrect number of values, reorder4_vals expects 4 values");
      return false;
    }
    cmdargs->reorder4_flag = true;

  } else if (vm.count("r4") != 0U) {
    auto vals = vm["r4"].as<std::vector<int>>();
    if (vals.size() != 4) {
      spdlog::get("stderr")->critical(
          "Incorrect number of values, r4 expects 4 values");
      return false;
    }
    cmdargs->reorder4_flag = true;
  }

  if ((vm.count("outside_val") != 0U) || (vm.count("oval") != 0U)) {
    fmt::printf("setting outside val to %d\n", cmdargs->outside_val);
  }

  if (vm.count("no-dwi") != 0U) {
    setenv("FS_LOAD_DWI", "0", 1);
  }

  if (vm.count("left-right-mirror") != 0U) {
    if ((cmdargs->left_right_mirror != "lh") &&
        (cmdargs->left_right_mirror != "rh")) {
      fmt::printf(
          "ERROR: pass either 'lh' or 'rh' with --left-right-mirror!\n");
      exit(1);
    }
  }

  if (vm.count("left-right-keep") != 0U) {
    if ((cmdargs->left_right_keep != "lh") &&
        (cmdargs->left_right_keep != "rh")) {
      fmt::printf("ERROR: pass either 'lh' or 'rh' with --left-right-keep!\n");
      exit(1);
    }
  }

  if (vm.count("conform-dc") != 0U) {
    cmdargs->conform_flag = true;
  }

  if (vm.count("cw256") != 0U) {
    cmdargs->conform_flag = true;
  }

  if (vm.count("new-transform-fname") != 0U) {
    cmdargs->DoNewTransformFname = true;
  }

  if (vm.count("rescale-dicom") != 0U) {
    // DO  apply rescale intercept and slope based on (0028,1052) (0028,1053).
    setenv("FS_RESCALE_DICOM", "1", 1);
  }

  if (vm.count("no-rescale-dicom") != 0U) {
    // Do NOT apply rescale intercept and slope based on (0028,1052)
    // (0028,1053).
    setenv("FS_RESCALE_DICOM", "0", 1);
  }

  if (vm.count("bvec-scanner") != 0U) {
    // force bvecs to be in scanner space. only applies when
    // reading dicoms
    setenv("FS_DESIRED_BVEC_SPACE", "1", 1);
  }

  if (vm.count("bvec-voxel") != 0U) {
    // force bvecs to be in voxel space. only applies when
    // reading dicoms.
    setenv("FS_DESIRED_BVEC_SPACE", "2", 1);
  }

  if (vm.count("no-analyze-rescale") != 0U) {
    // Turns off rescaling of analyze files
    setenv("FS_ANALYZE_NO_RESCALE", "1", 1);
  }

  if (vm.count("autoalign") != 0U) {
    cmdargs->AutoAlign =
        MatrixReadTxt(cmdargs->autoalign_file.c_str(), nullptr);
    fmt::printf("Auto Align Matrix\n");
    MatrixPrint(stdout, cmdargs->AutoAlign);
  }

  if ((vm.count("conform_min") != 0U) || (vm.count("cm") != 0U)) {
    cmdargs->conform_flag = true;
  }

  if ((vm.count("conform_size") != 0U) || (vm.count("cs") != 0U)) {
    cmdargs->conform_flag = true;
  }

  if ((vm.count("apply_transform") != 0U) || (vm.count("at") != 0U)) {
    cmdargs->transform_flag = true;
    cmdargs->invert_transform_flag = false;
  }

  if (vm.count("like") != 0U) {
    cmdargs->out_like_flag = true;
    // creates confusion when this is printed:
    // fmt::printf("WARNING: --like does not work on multi-frame data\n");
    // but we'll leave it here for the interested coder
  }

  if (vm.count("crop") != 0U) {
    auto crops = vm["crop"].as<std::vector<int>>();
    if (crops.size() != 3) {
      fmt::printf("ERROR: --crop must have 3 arguments");
      exit(1);
    }
    cmdargs->crop_flag = true;
  }

  if (vm.count("slice-crop") != 0U) {
    auto vals = vm["slice-crop"].as<std::vector<int>>();
    if (vals.size() != 2) {
      fmt::printf("ERROR: need 2 arguments (start and end position ");
      exit(1);
    }
    cmdargs->slice_crop_flag = true;
    cmdargs->slice_crop_start = vals[0];
    cmdargs->slice_crop_stop = vals[1];
    if (cmdargs->slice_crop_start > cmdargs->slice_crop_stop) {
      fmt::fprintf(stderr, "ERROR: s_start > s_end\n");
      exit(1);
    }
  }

  if (vm.count("cropsize") != 0U) {
    auto crops = vm["cropsize"].as<std::vector<int>>();
    if (crops.size() != 3) {
      fmt::printf("ERROR: --cropsize must have 3 arguments");
      exit(1);
    }
    cmdargs->crop_flag = true;
  }

  if (vm.count("devolvexfm") != 0U) {
    /* devolve xfm to account for cras != 0 */
    cmdargs->DevXFM = 0;
  }

  if ((vm.count("apply_inverse_transform") != 0U) || (vm.count("ait") != 0U)) {
    if (FileExists(cmdargs->transform_fname.data()) == 0) {
      fmt::fprintf(stderr, "ERROR: cannot find transform file %s\n",
                   cmdargs->transform_fname.data());
      exit(1);
    }
    cmdargs->transform_flag = true;
    cmdargs->invert_transform_flag = true;
  }

  if (vm.count("upsample") != 0U) {
    cmdargs->upsample_flag = true;
  }

  if ((vm.count("in_i_size") != 0U) || (vm.count("iis") != 0U)) {
    cmdargs->in_i_size_flag = true;
  }

  if ((vm.count("in_j_size") != 0U) || (vm.count("ijs") != 0U)) {
    cmdargs->in_j_size_flag = true;
  }

  if ((vm.count("in_k_size") != 0U) || (vm.count("iks") != 0U)) {
    cmdargs->in_k_size_flag = true;
  }

  if ((vm.count("out_i_size") != 0U) || (vm.count("ois") != 0U)) {
    cmdargs->out_i_size_flag = true;
  }

  if ((vm.count("out_j_size") != 0U) || (vm.count("ojs") != 0U)) {
    cmdargs->out_j_size_flag = true;
  }

  if ((vm.count("out_k_size") != 0U) || (vm.count("oks") != 0U)) {
    cmdargs->out_k_size_flag = true;
  }

  if (vm.count("erode-seg") != 0U) {
    cmdargs->erode_seg_flag = true;
  }

  if (vm.count("dil-seg") != 0U) {
    cmdargs->dil_seg_flag = true;
  }

  if (vm.count("cutends") != 0U) {
    cmdargs->ncutends_flag = true;
  }

  if ((vm.count("out_i_count") != 0U) || (vm.count("oni") != 0U) ||
      (vm.count("oic") != 0U)) {
    cmdargs->out_n_i_flag = true;
  }

  if ((vm.count("out_j_count") != 0U) || (vm.count("onj") != 0U) ||
      (vm.count("ojc") != 0U)) {
    cmdargs->out_n_j_flag = true;
  }

  if ((vm.count("out_k_count") != 0U) || (vm.count("onk") != 0U) ||
      (vm.count("okc") != 0U)) {
    cmdargs->out_n_k_flag = true;
  }

  if ((vm.count("in_i_count") != 0U) || (vm.count("ini") != 0U) ||
      (vm.count("iic") != 0U)) {
    cmdargs->in_n_i_flag = true;
  }

  if ((vm.count("in_j_count") != 0U) || (vm.count("inj") != 0U) ||
      (vm.count("ijc") != 0U)) {
    cmdargs->in_n_j_flag = true;
  }

  if ((vm.count("in_k_count") != 0U) || (vm.count("ink") != 0U) ||
      (vm.count("ikc") != 0U)) {
    cmdargs->in_n_k_flag = true;
  }

  if (vm.count("tr") != 0U) {
    cmdargs->in_tr_flag = true;
  }

  if (vm.count("TI") != 0U) {
    cmdargs->in_ti_flag = true;
  }

  if (vm.count("te") != 0U) {
    cmdargs->in_te_flag = true;
  }

  if (vm.count("flip_angle") != 0U) {
    cmdargs->in_flip_angle_flag = true;
  }

  if (vm.count("nskip") != 0) {
    if (cmdargs->nskip < 0) {
      spdlog::get("stderr")->critical("nskip cannot be negativ");
      exit(1);
    }
  }

  if (vm.count("ndrop") != 0) {
    if (cmdargs->ndrop < 0) {
      spdlog::get("stderr")->critical("nskip cannot be negativ");
      exit(1);
    }
  }

  if (vm.count("mra") != 0) {
    SliceResElTag1 = 0x50;
    SliceResElTag2 = 0x88;
  }

  if (vm.count("auto-slice-res") != 0) {
    AutoSliceResElTag = 1;
  }

  if (vm.count("no-strip-pound") != 0) {
    MRIIO_Strip_Pound = 0;
  }

  if (vm.count("mosaic-fix-noascii") != 0) {
    setenv("FS_MOSAIC_FIX_NOASCII", "1", 1);
  }

  if (vm.count("nslices-override") != 0) {
    fmt::printf("NSlicesOverride %d\n", cmdargs->nslices_override);
    std::string tmpstr = std::to_string(cmdargs->nslices_override);
    setenv("NSLICES_OVERRIDE", tmpstr.data(), 1);
  }

  if (vm.count("ncols-override") != 0) {
    fmt::printf("NColsOverride %d\n", cmdargs->ncols_override);
    std::string tmpstr = std::to_string(cmdargs->ncols_override);
    setenv("NCOLS_OVERRIDE", tmpstr.data(), 1);
  }

  if (vm.count("nrows-override") != 0) {
    fmt::printf("NRowsOverride %d\n", cmdargs->nrows_override);
    std::string tmpstr = std::to_string(cmdargs->nrows_override);
    setenv("NROWS_OVERRIDE", tmpstr.data(), 1);
  }

  if (vm.count("statusfile") != 0 || vm.count("status") != 0) {
    SDCMStatusFile = cmdargs->statusfile.data();
    std::ofstream ofs(SDCMStatusFile);
    if (!ofs.is_open()) {
      fmt::fprintf(stderr, "ERROR: could not open %s for writing\n",
                   SDCMStatusFile);
      exit(1);
    }
    ofs << "0\n";
    ofs.close();
  }

  if (vm.count("sdcmlist") != 0) {
    SDCMListFile = cmdargs->sdcmlist.data();
    std::ifstream fptmp(SDCMListFile);
    if (!fptmp.is_open()) {
      fmt::fprintf(stderr, "ERROR: could not open %s for reading\n",
                   SDCMListFile);
      exit(1);
    }
    fptmp.close();
  }

  if (vm.count("fsubsample") != 0) {
    auto opts = vm["fsubsample"].as<std::vector<int>>();
    if (opts.size() != 3) {
      fmt::fprintf(stderr,
                   "ERROR: fsubsample neads 3 values: start, delta, end\n");
    }
    cmdargs->SubSampStart = opts[0];
    cmdargs->SubSampDelta = opts[1];
    cmdargs->SubSampEnd = opts[2];
    if (cmdargs->SubSampDelta == 0) {
      fmt::printf("ERROR: don't use subsample delta = 0\n");
      exit(1);
    }
    cmdargs->subsample_flag = true;
  }

  if (vm.count("mid-frame") != 0) {
    cmdargs->frame_flag = true;
  }

  return false;
}

void initArgDesc(boost::program_options::options_description *desc,
                 CMDARGS *cmdargs) {

  namespace po = boost::program_options;

  desc->add_options()                                                   /**/
                                                                        /**/
      ("help,h",                                                        /**/
       "print out information on how to use this program and exit")     /**/
                                                                        /**/
      ("version,v",                                                     /**/
       "print out version and exit")                                    /**/
                                                                        /**/
      ("usage,u",                                                       /**/
       "print usage and exit")                                          /**/
                                                                        /**/
      ("version2",                                                      /**/
       "just exits")                                                    /**/
                                                                        /**/
      ("debug",                                                         /**/
       po::bool_switch(&cmdargs->debug),                                /**/
       "turn on debugging")                                             /**/
                                                                        /**/
      ("reorder, r",                                                    /**/
       po::value<std::vector<int>>(&cmdargs->reorder_vals),             /**/
       "todo")                                                          /**/
                                                                        /**/
      ("reorder4",                                                      /**/
       po::value<std::vector<int>>(&cmdargs->reorder4_vals),            /**/
       "todo")                                                          /**/
                                                                        /**/
      ("r4",                                                            /**/
       po::value<std::vector<int>>(&cmdargs->reorder4_vals),            /**/
       "todo")                                                          /**/
                                                                        /**/
      ("outside_val",                                                   /**/
       po::value<int>(&cmdargs->outside_val),                           /**/
       "Set the values outside of the image that may rotate in if a"    /**/
       " transform is applied to val")                                  /**/
                                                                        /**/
      ("oval",                                                          /**/
       po::value<int>(&cmdargs->outside_val),                           /**/
       "todo")                                                          /**/
                                                                        /**/
      ("no-dwi",                                                        /**/
       "set FS_LOAD_DWI to 0")                                          /**/
                                                                        /**/
      ("left-right-reverse",                                            /**/
       po::bool_switch(&cmdargs->left_right_reverse),                   /**/
       "left right reverse")                                            /**/
                                                                        /**/
      ("left-right-reverse-pix",                                        /**/
       po::bool_switch(&cmdargs->left_right_reverse_pix),               /**/
       "left-right-reverse-pix")                                        /**/
                                                                        /**/
      ("left-right-mirror",                                             /**/
       po::value<std::string>(&cmdargs->left_right_mirror),             /**/
       "left-right-mirror")                                             /**/
                                                                        /**/
      ("left-right-keep",                                               /**/
       po::value<std::string>(&cmdargs->left_right_keep),               /**/
       "left-right-keep")                                               /**/
                                                                        /**/
      ("left-right-swap-label",                                         /**/
       po::bool_switch(&cmdargs->left_right_swap_label),                /**/
       "left-right-swap-label")                                         /**/
                                                                        /**/
      ("flip-cols",                                                     /**/
       po::bool_switch(&cmdargs->flip_cols),                            /**/
       "flip-cols")                                                     /**/
                                                                        /**/
      ("slice-reverse",                                                 /**/
       po::bool_switch(&cmdargs->slice_reverse),                        /**/
       "slice-reverse")                                                 /**/
                                                                        /**/
      ("in_stats_table",                                                /**/
       po::bool_switch(&cmdargs->in_stats_table),                       /**/
       "Input data is a stats table as produced by asegstats2table or " /**/
       "aparcstats2table")                                              /**/
                                                                        /**/
      ("out_stats_table",                                               /**/
       po::bool_switch(&cmdargs->out_stats_table),                      /**/
       "Output data is a stats table (use --like to pass template"      /**/
       " table for measure, columns, and rows heads)")                  /**/
                                                                        /**/
      ("invert_contrast",                                               /**/
       po::value<float>(&cmdargs->invert_contrast),                     /**/
       "invert_contrast")                                               /**/
                                                                        /**/
      ("input_volume, i",                                               /**/
       po::value<std::string>(&cmdargs->input_volume),                  /**/
       "input_volume")                                                  /**/
                                                                        /**/
      ("output_volume, o",                                              /**/
       po::value<std::string>(&cmdargs->output_volume),                 /**/
       "output_volume")                                                 /**/
                                                                        /**/
      ("conform, c",                                                    /**/
       po::bool_switch(&cmdargs->conform_flag),                         /**/
       "conform")                                                       /**/
                                                                        /**/
      ("conform-dc",                                                    /**/
       po::value<int>(&cmdargs->conf_keep_dc),                          /**/
       "conform-dc")                                                    /**/
                                                                        /**/
      ("cw256",                                                         /**/
       po::bool_switch(&cmdargs->conform_width_256_flag),               /**/
       "cw256")                                                         /**/
                                                                        /**/
      ("delete-cmds",                                                   /**/
       po::bool_switch(&cmdargs->delete_cmds),                          /**/
       "delete-cmds")                                                   /**/
                                                                        /**/
      ("new-transform-fname",                                           /**/
       po::value<std::string>(&cmdargs->new_transform_fname),           /**/
       "new-transform-fname")                                           /**/
                                                                        /**/
      ("sphinx",                                                        /**/
       po::bool_switch(&cmdargs->sphinx_flag),                          /**/
       "Change orientation info to sphinx")                             /**/
                                                                        /**/
      ("rescale-dicom",                                                 /**/
       "rescale-dicom")                                                 /**/
                                                                        /**/
      ("no-rescale-dicom",                                              /**/
       "no-rescale-dicom")                                              /**/
                                                                        /**/
      ("bvec-scanner",                                                  /**/
       "bvec-scanner")                                                  /**/
                                                                        /**/
      ("bvec-voxel",                                                    /**/
       "bvec-voxel")                                                    /**/
                                                                        /**/
      ("no-analyze-rescale",                                            /**/
       "no-analyze-rescale")                                            /**/
                                                                        /**/
      ("autoalign",                                                     /**/
       po::value<std::string>(&cmdargs->autoalign_file),                /**/
       "autoalign")                                                     /**/
                                                                        /**/
      ("nochange",                                                      /**/
       po::bool_switch(&cmdargs->nochange),                             /**/
       "Don't change type of input to that of template")                /**/
                                                                        /**/
      ("nc",                                                            /**/
       po::bool_switch(&cmdargs->nochange),                             /**/
       "nc")                                                            /**/
                                                                        /**/
      ("conform_min",                                                   /**/
       po::bool_switch(&cmdargs->conform_min_flag),                     /**/
       "conform_min")                                                   /**/
                                                                        /**/
      ("conform_size",                                                  /**/
       po::value<float>(&cmdargs->conform_size),                        /**/
       "conform_size")                                                  /**/
                                                                        /**/
      ("cs",                                                            /**/
       po::value<float>(&cmdargs->conform_size),                        /**/
       "conform_size")                                                  /**/
                                                                        /**/
      ("parse_only",                                                    /**/
       po::bool_switch(&cmdargs->parse_only_flag),                      /**/
       "parse_only")                                                    /**/
                                                                        /**/
      ("po",                                                            /**/
       po::bool_switch(&cmdargs->parse_only_flag),                      /**/
       "parse_only")                                                    /**/
                                                                        /**/
      ("in_info",                                                       /**/
       po::bool_switch(&cmdargs->in_info_flag),                         /**/
       "in_info")                                                       /**/
                                                                        /**/
      ("ii",                                                            /**/
       po::bool_switch(&cmdargs->in_info_flag),                         /**/
       "in_info")                                                       /**/
                                                                        /**/
      ("out_info",                                                      /**/
       po::bool_switch(&cmdargs->out_info_flag),                        /**/
       "out_info")                                                      /**/
                                                                        /**/
      ("oi",                                                            /**/
       po::bool_switch(&cmdargs->out_info_flag),                        /**/
       "out_info")                                                      /**/
                                                                        /**/
      ("template_info",                                                 /**/
       po::bool_switch(&cmdargs->template_info_flag),                   /**/
       "template_info")                                                 /**/
                                                                        /**/
      ("ti",                                                            /**/
       po::bool_switch(&cmdargs->template_info_flag),                   /**/
       "template_info")                                                 /**/
                                                                        /**/
      ("in_stats",                                                      /**/
       po::bool_switch(&cmdargs->in_stats_flag),                        /**/
       "Print statistics on input volume")                              /**/
                                                                        /**/
      ("is",                                                            /**/
       po::bool_switch(&cmdargs->in_stats_flag),                        /**/
       "Print statistics on input volume")                              /**/
                                                                        /**/
      ("out_stats",                                                     /**/
       po::bool_switch(&cmdargs->out_stats_flag),                       /**/
       "Print statistics on output volume")                             /**/
                                                                        /**/
      ("os",                                                            /**/
       po::bool_switch(&cmdargs->out_stats_flag),                       /**/
       "Print statistics on output volume")                             /**/
                                                                        /**/
      ("read_only",                                                     /**/
       po::bool_switch(&cmdargs->read_only_flag),                       /**/
       "read_only")                                                     /**/
                                                                        /**/
      ("ro",                                                            /**/
       po::bool_switch(&cmdargs->read_only_flag),                       /**/
       "read_only")                                                     /**/
                                                                        /**/
      ("no_write",                                                      /**/
       po::bool_switch(&cmdargs->no_write_flag),                        /**/
       "no_write")                                                      /**/
                                                                        /**/
      ("nw",                                                            /**/
       po::bool_switch(&cmdargs->no_write_flag),                        /**/
       "no_write")                                                      /**/
                                                                        /**/
      ("in_matrix",                                                     /**/
       po::bool_switch(&cmdargs->in_matrix_flag),                       /**/
       "in_matrix")                                                     /**/
                                                                        /**/
      ("im",                                                            /**/
       po::bool_switch(&cmdargs->in_matrix_flag),                       /**/
       "in_matrix")                                                     /**/
                                                                        /**/
      ("out_matrix",                                                    /**/
       po::bool_switch(&cmdargs->out_matrix_flag),                      /**/
       "out_matrix")                                                    /**/
                                                                        /**/
      ("om",                                                            /**/
       po::bool_switch(&cmdargs->out_matrix_flag),                      /**/
       "out_matrix")                                                    /**/
                                                                        /**/
      ("force_ras_good",                                                /**/
       po::bool_switch(&cmdargs->force_ras_good),                       /**/
       "force_ras_good")                                                /**/
                                                                        /**/
      ("split",                                                         /**/
       po::bool_switch(&cmdargs->split_frames_flag),                    /**/
       "split")                                                         /**/
                                                                        /**/
      /* transform related things here */                               /**/
                                                                        /**/
      ("apply_transform, T",                                            /**/
       po::value<std::string>(&cmdargs->transform_fname),               /**/
       "apply_transform")                                               /**/
                                                                        /**/
      ("at",                                                            /**/
       po::value<std::string>(&cmdargs->transform_fname),               /**/
       "apply_transform")                                               /**/
                                                                        /**/
      ("like",                                                          /**/
       po::value<std::string>(&cmdargs->out_like_name),                 /**/
       "like")                                                          /**/
                                                                        /**/
      ("crop",                                                          /**/
       po::value<std::vector<int>>(&cmdargs->crop_center),              /**/
       "crop")                                                          /**/
                                                                        /**/
      ("slice-crop",                                                    /**/
       po::value<std::vector<int>>(&cmdargs->slice_crop),               /**/
       "slice-crop")                                                    /**/
                                                                        /**/
      ("cropsize",                                                      /**/
       po::value<std::vector<int>>(&cmdargs->cropsize),                 /**/
       "cropsize")                                                      /**/
                                                                        /**/
      ("devolvexfm",                                                    /**/
       po::value<std::string>(&cmdargs->devolvexfm),                    /**/
       "devolvexfm")                                                    /**/
                                                                        /**/
      ("apply_inverse_transform",                                       /**/
       po::value<std::string>(&cmdargs->transform_fname),               /**/
       "apply_inverse_transform")                                       /**/
                                                                        /**/
      ("ait",                                                           /**/
       po::value<std::string>(&cmdargs->transform_fname),               /**/
       "apply_inverse_transform")                                       /**/
                                                                        /**/
      ("upsample",                                                      /**/
       po::value<int>(&cmdargs->upsample_factor),                       /**/
       "Reduce voxel size by a factor of N in all dimensions")          /**/
                                                                        /**/
      ("in_i_size",                                                     /**/
       po::value<float>(&cmdargs->in_i_size),                           /**/
       "in_i_size")                                                     /**/
                                                                        /**/
      ("iis",                                                           /**/
       po::value<float>(&cmdargs->in_i_size),                           /**/
       "in_i_size")                                                     /**/
                                                                        /**/
      ("in_j_size",                                                     /**/
       po::value<float>(&cmdargs->in_j_size),                           /**/
       "in_j_size")                                                     /**/
                                                                        /**/
      ("ijs",                                                           /**/
       po::value<float>(&cmdargs->in_j_size),                           /**/
       "in_j_size")                                                     /**/
                                                                        /**/
      ("in_k_size",                                                     /**/
       po::value<float>(&cmdargs->in_k_size),                           /**/
       "in_k_size")                                                     /**/
                                                                        /**/
      ("iks",                                                           /**/
       po::value<float>(&cmdargs->in_k_size),                           /**/
       "in_k_size")                                                     /**/
                                                                        /**/
      ("out_i_size",                                                    /**/
       po::value<float>(&cmdargs->out_i_size),                          /**/
       "out_i_size")                                                    /**/
                                                                        /**/
      ("ois",                                                           /**/
       po::value<float>(&cmdargs->out_i_size),                          /**/
       "out_i_size")                                                    /**/
                                                                        /**/
      ("out_j_size",                                                    /**/
       po::value<float>(&cmdargs->out_j_size),                          /**/
       "out_j_size")                                                    /**/
                                                                        /**/
      ("ojs",                                                           /**/
       po::value<float>(&cmdargs->out_j_size),                          /**/
       "out_j_size")                                                    /**/
                                                                        /**/
      ("out_k_size",                                                    /**/
       po::value<float>(&cmdargs->out_k_size),                          /**/
       "out_k_size")                                                    /**/
                                                                        /**/
      ("oks",                                                           /**/
       po::value<float>(&cmdargs->out_k_size),                          /**/
       "out_k_size")                                                    /**/
                                                                        /**/
      ("ctab",                                                          /**/
       po::value<std::string>(&cmdargs->colortablefile),                /**/
       "ctab")                                                          /**/
                                                                        /**/
      ("nth_frame",                                                     /**/
       po::value<int>(&cmdargs->nthframe),                              /**/
       "nth_frame")                                                     /**/
                                                                        /**/
      ("nth",                                                           /**/
       po::value<int>(&cmdargs->nthframe),                              /**/
       "nth_frame")                                                     /**/
                                                                        /**/
      ("no_translate",                                                  /**/
       po::bool_switch(&cmdargs->translate_labels_flag),                /**/
       "no_translate")                                                  /**/
                                                                        /**/
      ("nt",                                                            /**/
       po::bool_switch(&cmdargs->translate_labels_flag),                /**/
       "no_translate")                                                  /**/
                                                                        /**/
      ("zero_outlines",                                                 /**/
       po::bool_switch(&cmdargs->zero_outlines_flag),                   /**/
       "zero_outlines")                                                 /**/
                                                                        /**/
      ("zo",                                                            /**/
       po::bool_switch(&cmdargs->zero_outlines_flag),                   /**/
       "zero_outlines")                                                 /**/
                                                                        /**/
      ("fill_parcellation",                                             /**/
       po::bool_switch(&cmdargs->fill_parcellation_flag),               /**/
       "fill_parcellation")                                             /**/
                                                                        /**/
      ("fp",                                                            /**/
       po::bool_switch(&cmdargs->fill_parcellation_flag),               /**/
       "fill_parcellation")                                             /**/
                                                                        /**/
      ("roi",                                                           /**/
       po::bool_switch(&cmdargs->roi_flag),                             /**/
       "roi flag")                                                      /**/
                                                                        /**/
      ("dil-seg-mask",                                                  /**/
       po::value<std::string>(&cmdargs->dil_seg_mask),                  /**/
       "dil-seg-mask")                                                  /**/
                                                                        /**/
      ("erode-seg",                                                     /**/
       po::value<int>(&cmdargs->erode_seg),                             /**/
       "erode-seg")                                                     /**/
                                                                        /**/
      ("dil-seg",                                                       /**/
       po::value<int>(&cmdargs->dil_seg),                               /**/
       "dil-seg")                                                       /**/
                                                                        /**/
      ("cutends",                                                       /**/
       po::value<int>(&cmdargs->ncutends),                              /**/
       "cutends")                                                       /**/
      /**/                                                              /**/
      ("out_i_count",                                                   /**/
       po::value<int>(&cmdargs->out_n_i),                               /**/
       "out_i_count")                                                   /**/
      /**/                                                              /**/
      ("oni",                                                           /**/
       po::value<int>(&cmdargs->out_n_i),                               /**/
       "out_i_count")                                                   /**/
      /**/                                                              /**/
      ("oic",                                                           /**/
       po::value<int>(&cmdargs->out_n_i),                               /**/
       "out_i_count")                                                   /**/
      /**/                                                              /**/
      ("out_j_count",                                                   /**/
       po::value<int>(&cmdargs->out_n_j),                               /**/
       "out_i_count")                                                   /**/
      /**/                                                              /**/
      ("onj",                                                           /**/
       po::value<int>(&cmdargs->out_n_j),                               /**/
       "out_j_count")                                                   /**/
      /**/                                                              /**/
      ("ojc",                                                           /**/
       po::value<int>(&cmdargs->out_n_j),                               /**/
       "out_j_count")                                                   /**/
      /**/                                                              /**/
      ("out_k_count",                                                   /**/
       po::value<int>(&cmdargs->out_n_k),                               /**/
       "out_i_count")                                                   /**/
      /**/                                                              /**/
      ("onk",                                                           /**/
       po::value<int>(&cmdargs->out_n_k),                               /**/
       "out_k_count")                                                   /**/
      /**/                                                              /**/
      ("okc",                                                           /**/
       po::value<int>(&cmdargs->out_n_k),                               /**/
       "out_k_count")                                                   /**/
      /**/                                                              /**/
      ("downsample2",                                                   /**/
       po::bool_switch(&cmdargs->downsample2_flag),                     /**/
       "downsample2")                                                   /**/
      /**/                                                              /**/
      ("ds2",                                                           /**/
       po::bool_switch(&cmdargs->downsample2_flag),                     /**/
       "downsample2")                                                   /**/
      /**/                                                              /**/
      ("in_i_count",                                                    /**/
       po::value<int>(&cmdargs->in_n_i),                                /**/
       "in_i_count")                                                    /**/
      /**/                                                              /**/
      ("ini",                                                           /**/
       po::value<int>(&cmdargs->in_n_i),                                /**/
       "in_i_count")                                                    /**/
      /**/                                                              /**/
      ("iic",                                                           /**/
       po::value<int>(&cmdargs->in_n_i),                                /**/
       "in_i_count")                                                    /**/
      /**/                                                              /**/
      ("in_j_count",                                                    /**/
       po::value<int>(&cmdargs->in_n_j),                                /**/
       "in_j_count")                                                    /**/
      /**/                                                              /**/
      ("inj",                                                           /**/
       po::value<int>(&cmdargs->in_n_j),                                /**/
       "in_i_count")                                                    /**/
      /**/                                                              /**/
      ("ijc",                                                           /**/
       po::value<int>(&cmdargs->in_n_j),                                /**/
       "in_j_count")                                                    /**/
      /**/                                                              /**/
      ("in_k_count",                                                    /**/
       po::value<int>(&cmdargs->in_n_k),                                /**/
       "in_k_count")                                                    /**/
      /**/                                                              /**/
      ("ink",                                                           /**/
       po::value<int>(&cmdargs->in_n_k),                                /**/
       "in_k_count")                                                    /**/
      /**/                                                              /**/
      ("ikc",                                                           /**/
       po::value<int>(&cmdargs->in_n_k),                                /**/
       "in_k_count")                                                    /**/
      /**/                                                              /**/
      ("tr",                                                            /**/
       po::value<float>(&cmdargs->in_tr),                               /**/
       "tr")                                                            /**/
      /**/                                                              /**/
      ("TI",                                                            /**/
       po::value<float>(&cmdargs->in_ti),                               /**/
       "ti")                                                            /**/
      /**/                                                              /**/
      ("te",                                                            /**/
       po::value<float>(&cmdargs->in_te),                               /**/
       "te")                                                            /**/
      /**/                                                              /**/
      ("flip_angle",                                                    /**/
       po::value<float>(&cmdargs->in_flip_angle),                       /**/
       "flip_angle")                                                    /**/
      /**/                                                              /**/
      ("in_name",                                                       /**/
       po::value<std::string>(&cmdargs->in_name)->required(),           /**/
       "in_name")                                                       /**/
      /**/                                                              /**/
      ("out_name",                                                      /**/
       po::value<std::string>(&cmdargs->out_name),                      /**/
       "out_name")                                                      /**/
      /**/                                                              /**/
      ("zero_ge_z_offset",                                              /**/
       po::bool_switch(&cmdargs->zero_ge_z_offset_flag),                /**/
       "zero_ge_z_offset")                                              /**/
      /**/                                                              /**/
      ("zgez",                                                          /**/
       po::bool_switch(&cmdargs->zero_ge_z_offset_flag),                /**/
       "zero_ge_z_offset")                                              /**/
      /**/                                                              /**/
      ("no_zero_ge_z_offset",                                           /**/
       po::bool_switch(&cmdargs->no_zero_ge_z_offset_flag),             /**/
       "no_zero_ge_z_offset")                                           /**/
      /**/                                                              /**/
      ("nozgez",                                                        /**/
       po::bool_switch(&cmdargs->no_zero_ge_z_offset_flag),             /**/
       "no_zero_ge_z_offset")                                           /**/
      /**/                                                              /**/
      ("nskip",                                                         /**/
       po::value<int>(&cmdargs->nskip),                                 /**/
       "nskip")                                                         /**/
      /**/                                                              /**/
      ("ndrop",                                                         /**/
       po::value<int>(&cmdargs->ndrop),                                 /**/
       "ndrop")                                                         /**/
      /**/                                                              /**/
      ("diag",                                                          /**/
       po::value<int>(&Gdiag_no),                                       /**/
       "diag")                                                          /**/
      /**/                                                              /**/
      ("mra",                                                           /**/
       "This flag forces DICOMread to first use 18,50 to get the slice" /**/
       " thickness instead of 18,88. This is needed with siemens mag "  /**/
       "res angiogram (MRAs)")                                          /**/
      /**/                                                              /**/
      ("auto-slice-res",                                                /**/
       "Automatically determine whether to get slice thickness "        /**/
       "from 18,50 or 18,88 depending upon  the value of 18,23")        /**/
      /**/                                                              /**/
      ("no-strip-pound",                                                /**/
       "no-strip-pound")                                                /**/
      /**/                                                              /**/
      ("in_nspmzeropad",                                                /**/
       po::value<int>(&N_Zero_Pad_Input),                               /**/
       "in_nspmzeropad")                                                /**/
      /**/                                                              /**/
      ("nspmzeropad",                                                   /**/
       po::value<int>(&N_Zero_Pad_Output),                              /**/
       "out_nspmzeropad")                                               /**/
      /**/                                                              /**/
      ("out_nspmzeropad",                                               /**/
       po::value<int>(&N_Zero_Pad_Output),                              /**/
       "out_nspmzeropad")                                               /**/
      /**/                                                              /**/
      ("mosaic-fix-noascii",                                            /**/
       "mosaic-fix-noascii")                                            /**/
      /**/                                                              /**/
      ("nslices-override",                                              /**/
       po::value<int>(&cmdargs->nslices_override),                      /**/
       "nslices-override")                                              /**/
      /**/                                                              /**/
      ("ncols-override",                                                /**/
       po::value<int>(&cmdargs->ncols_override),                        /**/
       "ncols-override")                                                /**/
      /**/                                                              /**/
      ("nrows-override",                                                /**/
       po::value<int>(&cmdargs->nrows_override),                        /**/
       "nrows-override")                                                /**/
      /**/                                                              /**/
      ("statusfile",                                                    /**/
       po::value<std::string>(&cmdargs->statusfile),                    /**/
       "File name to write percent complete for Siemens DICOM")         /**/
      /**/                                                              /**/
      ("status",                                                        /**/
       po::value<std::string>(&cmdargs->statusfile),                    /**/
       "File name to write percent complete for Siemens DICOM")         /**/
      /**/                                                              /**/
      ("sdcmlist",                                                      /**/
       po::value<std::string>(&cmdargs->sdcmlist),                      /**/
       "File name that contains a list of Siemens DICOM files that "    /**/
       "are in the same run as the one listed on the command-line. "    /**/
       "If not present, the directory will be scanned, but this can "   /**/
       "take a while.")                                                 /**/
      /**/                                                              /**/
      ("fsubsample",                                                    /**/
       po::value<std::vector<int>>(&cmdargs->fsubsample),               /**/
       "fsubsample")                                                    /**/
      /**/                                                              /**/
      ("mid-frame",                                                     /**/
       po::bool_switch(&cmdargs->mid_frame_flag),                       /**/
       "mid-frame");                                                    /**/
}
/* EOF */
