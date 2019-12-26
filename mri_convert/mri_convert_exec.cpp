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

#include "DICOMRead.h"
#include "cma.h"
#include "diag.h"
#include "fio.h"
#include "fmriutils.h"
#include "gcamorph.h"
#include "mri2.h"
#include "mri_conform.h"
#include "mri_identify.h"
#include "mriio.hpp"

#include <boost/algorithm/string.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

//#include <aboualiaa/banned.h>

auto main(int argc, char *argv[]) -> int {
  auto err_logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_level(spdlog::level::warn);
  auto env = ENV();
  auto cmdargs = CMDARGS(argc, argv);
  if (!good_cmdline_args(&cmdargs, &env)) {
    spdlog::get("stderr")->critical("error while parsing.");
    return 1;
  }

  MRI *mri{};
  MRI *mri2{};
  MRI *mri_template{};
  MRI *mri_in_like{};
  int i{};
  int err{};
  float invert_val{1.0};
  int conform_width{};
  int in_volume_type{};
  int out_volume_type{MRI_VOLUME_TYPE_UNKNOWN};
  bool sizes_good_flag{};
  std::string tmpstr{};
  char *stem{};
  char *ext{};
  std::array<char, 4> ostr{};
  float i_dot_j{};
  float i_dot_k{};
  float j_dot_k{};
  float fov_x{};
  float fov_y{};
  float fov_z{};
  int nframes{};
  std::string in_name_only;
  LTA *lta_transform{};
  MRI *mri_transformed{};
  MRI *mritmp{};
  int transform_type{-1};
  MATRIX *inverse_transform_matrix{};
  bool read_parcellation_volume_flag{};
  int read_otl_flags{};
  int temp_type{};
  FILE *fptmp{};
  int j{};
  VOL_GEOM vgtmp;
  LT *lt{};
  MATRIX *T{};
  std::string cmdline{};
  float v{};
  MATRIX *cras{};
  MATRIX *vmid{};
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
  STAT_TABLE *StatTable{};
  STAT_TABLE *OutStatTable{};

  DiagInit(nullptr, nullptr, nullptr);

  // TODO(aboualiaa): Implement a safe version of make_cmd_version_string (in
  // utils/version.cpp) then delete this loop
  for (auto arg : cmdargs.raw) {
    cmdline.append(arg);
    cmdline.append(" ");
  }

  Progname = GET_PROGRAM_NAME();

  sizes_good_flag = true;

  if (!sizes_good_flag) {
    spdlog::get("stderr")->critical("sizes_good_flag is not set");
    fs::util::cli::usage_message(stdout);
    exit(1);
  }

  /* ----- copy file name (only -- strip '@' and '#') ----- */
  in_name_only = fs::mri::io::getVolumeName(cmdargs.in_name);

  /* If input type is spm and N_Zero_Pad_Input < 0, set to 3*/
  if (cmdargs.in_type_string == "spm" && N_Zero_Pad_Input < 0) {
    N_Zero_Pad_Input = 3;
  }

  /* ----- catch unknown volume types ----- */
  if ((cmdargs.force_in_type_flag) &&
      cmdargs.forced_in_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr,
                 "\n%s: unknown input "
                 "volume type %s\n",
                 Progname, cmdargs.in_type_string.data());
    fs::util::cli::usage_message(stdout);
    exit(1);
  }

  if ((cmdargs.force_template_type_flag) &&
      cmdargs.forced_template_type == MRI_VOLUME_TYPE_UNKNOWN) {
    fmt::fprintf(stderr, "\n%s: unknown template volume type %s\n", Progname,
                 cmdargs.template_type_string.data());
    fs::util::cli::usage_message(stdout);
    exit(1);
  }

  if ((cmdargs.force_out_type_flag) &&
      cmdargs.forced_out_type == MRI_VOLUME_TYPE_UNKNOWN &&
      (cmdargs.ascii_flag == 0)) {
    fmt::fprintf(stderr, "\n%s: unknown output volume type %s\n", Progname,
                 cmdargs.out_type_string.data());
    fs::util::cli::usage_message(stdout);
    exit(1);
  }

  /* ----- warn if read only is desired and an
     output volume is specified or the output info flag is set ----- */
  if ((cmdargs.read_only_flag) && cmdargs.out_name[0] != '\0') {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "nothing will be written to %s\n",
                 Progname, cmdargs.out_name.data());
  }
  if ((cmdargs.read_only_flag) &&
      ((cmdargs.out_info_flag) || (cmdargs.out_matrix_flag))) {
    fmt::fprintf(stderr,
                 "%s: warning: read only flag is set; "
                 "no output information will be printed\n",
                 Progname);
  }

  /* ----- get the type of the output ----- */
  if (!cmdargs.read_only_flag) {
    if ((!cmdargs.force_out_type_flag) && (!cmdargs.out_stats_table_flag)) {
      // if(!read_only_flag && !no_write_flag)
      // because conform_flag value changes depending on type below
      {
        out_volume_type = mri_identify(cmdargs.out_name.data());
        if (out_volume_type == MRI_VOLUME_TYPE_UNKNOWN) {
          fmt::fprintf(stderr, "%s: can't determine type of output volume\n",
                       Progname);
          exit(1);
        }
      }
    } else {
      out_volume_type = cmdargs.forced_out_type;
    }

    // if output type is COR, then it is always conformed
    if (out_volume_type == MRI_CORONAL_SLICE_DIRECTORY) {
      cmdargs.conform_flag = TRUE;
    }
  }

  /* ----- catch the parse-only flag ----- */
  if (cmdargs.parse_only_flag) {
    fmt::printf("input volume name: %s\n", cmdargs.in_name.data());
    fmt::printf("input name only: %s\n", in_name_only.data());
    fmt::printf("output volume name: %s\n", cmdargs.out_name.data());
    fmt::printf("parse_only_flag = %d\n", cmdargs.parse_only_flag);
    fmt::printf("conform_flag = %d\n", cmdargs.conform_flag);
    fmt::printf("conform_size = %f\n", cmdargs.conform_size);
    fmt::printf("in_info_flag = %d\n", cmdargs.in_info_flag);
    fmt::printf("out_info_flag = %d\n", cmdargs.out_info_flag);
    fmt::printf("in_matrix_flag = %d\n", cmdargs.in_matrix_flag);
    fmt::printf("out_matrix_flag = %d\n", cmdargs.out_matrix_flag);

    if (cmdargs.force_in_type_flag) {
      fmt::printf("input type is %d\n", cmdargs.forced_in_type);
    }
    if (cmdargs.force_out_type_flag) {
      fmt::printf("output type is %d\n", cmdargs.forced_out_type);
    }

    if (cmdargs.subject_name[0] != '\0') {
      fmt::printf("subject name is %s\n", cmdargs.subject_name.data());
    }

    if (invert_val >= 0) {
      fmt::printf("inversion, value is %g\n", invert_val);
    }

    if (!cmdargs.reorder_vals.empty()) {
      fmt::printf("reordering, values are %d %d %d\n", cmdargs.reorder_vals[0],
                  cmdargs.reorder_vals[1], cmdargs.reorder_vals[2]);
    }

    if (!cmdargs.reorder4_vals.empty()) {
      fmt::printf("reordering, values are %d %d %d %d\n",
                  cmdargs.reorder4_vals[0], cmdargs.reorder4_vals[1],
                  cmdargs.reorder4_vals[2], cmdargs.reorder4_vals[3]);
    }

    fmt::printf("translation of otl labels is %s\n",
                cmdargs.translate_labels_flag ? "on" : "off");

    exit(0);
  }

  /* ----- check for a gdf image stem if the output type is gdf ----- */
  if (out_volume_type == GDF_FILE && cmdargs.gdf_image_stem.empty()) {
    fmt::fprintf(stderr,
                 "%s: GDF output type, "
                 "but no GDF image file stem\n",
                 Progname);
    exit(1);
  }

  /* ----- read the in_like volume ----- */
  if (cmdargs.in_like_flag) {
    fmt::printf("reading info from %s...\n", cmdargs.in_like_name.data());
    mri_in_like = MRIreadInfo(cmdargs.in_like_name.data());
    if (mri_in_like == nullptr) {
      exit(1);
    }
  }

  /* ----- read the volume ----- */
  in_volume_type = MRI_VOLUME_TYPE_UNKNOWN;
  if (!cmdargs.in_stats_table_flag) {
    if (cmdargs.force_in_type_flag) {
      in_volume_type = cmdargs.forced_in_type;
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
      if (cmdargs.in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }
  }

  if ((cmdargs.roi_flag) && in_volume_type != GENESIS_FILE) {
    errno = 0;
    ErrorPrintf(ERROR_BADPARM, "rois must be in GE format");
    if (cmdargs.in_like_flag) {
      MRIfree(&mri_in_like);
    }
    exit(1);
  }

  if (((cmdargs.zero_ge_z_offset_flag)) && in_volume_type != DICOM_FILE) // E/
  {
    cmdargs.zero_ge_z_offset_flag = FALSE;
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
    if (cmdargs.downsample2_flag) {
      gcam_out = GCAMdownsample2(gcam);
    } else {
      gcam_out = gcam;
    }

    GCAMwrite(gcam_out, cmdargs.out_name.data());
    exit(0);
  }

  if (in_volume_type == OTL_FILE) {

    if ((!cmdargs.in_like_flag) && (!cmdargs.in_n_k_flag)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM,
                  "parcellation read: must specify"
                  "a volume depth with either in_like or in_k_count");
      exit(1);
    }

    if (!cmdargs.color_file_flag) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "parcellation read: must specify a"
                                 "color file name");
      if (cmdargs.in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    read_parcellation_volume_flag = TRUE;
    if ((cmdargs.read_only_flag) &&
        ((cmdargs.in_info_flag) || (cmdargs.in_matrix_flag)) &&
        (!cmdargs.in_stats_flag)) {
      read_parcellation_volume_flag = FALSE;
    }

    read_otl_flags = 0x00;

    if (read_parcellation_volume_flag) {
      read_otl_flags |= READ_OTL_READ_VOLUME_FLAG;
    }

    if (cmdargs.fill_parcellation_flag) {
      read_otl_flags |= READ_OTL_FILL_FLAG;
    } else {
      fmt::printf("notice: unfilled parcellations "
                  "currently unimplemented\n");
      fmt::printf("notice: filling outlines\n");
      read_otl_flags |= READ_OTL_FILL_FLAG;
    }

    if (cmdargs.translate_labels_flag) {
      read_otl_flags |= READ_OTL_TRANSLATE_LABELS_FLAG;
    }

    if (cmdargs.zero_outlines_flag) {
      read_otl_flags |= READ_OTL_ZERO_OUTLINES_FLAG;
    }

    if (cmdargs.in_like_flag) {
      mri = MRIreadOtl(cmdargs.in_name.data(), mri_in_like->width,
                       mri_in_like->height, mri_in_like->depth,
                       cmdargs.color_file_name.data(), read_otl_flags);
    } else {
      mri = MRIreadOtl(cmdargs.in_name.data(), 0, 0, cmdargs.in_n_k,
                       cmdargs.color_file_name.data(), read_otl_flags);
    }

    if (mri == nullptr) {
      if (cmdargs.in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

#if 0
    if (out_volume_type == MGH_MORPH) {
      GCA_MORPH *gcam;

      if (mri->nframes != 3)
        ErrorExit(ERROR_UNSUPPORTED,
                  "%s: input volume must have 3 frames not %d to write to warp",
                  Progname, mri->nframes);
      gcam = GCAMalloc(mri->width, mri->height, mri->depth);
      GCAMinit(gcam, mri, NULL, NULL, 0);
      GCAMreadWarpFromMRI(gcam, mri);
      GCAMwrite(gcam, out_name);
      exit(0);
    }
#endif

    /* ----- smooth the parcellation if requested ----- */
    if (cmdargs.smooth_parcellation_count != 0) {
      fmt::printf("smoothing parcellation...\n");
      mri2 = MRIsmoothParcellation(mri, cmdargs.smooth_parcellation_count);
      if (mri2 == nullptr) {
        if (cmdargs.in_like_flag) {
          MRIfree(&mri_in_like);
        }
        exit(1);
      }
      MRIfree(&mri);
      mri = mri2;
    }

    cmdargs.resample_type_val = SAMPLE_NEAREST;
    cmdargs.no_scale_flag = TRUE;

  } else if (cmdargs.roi_flag) {
    if ((!cmdargs.in_like_flag) && (!cmdargs.in_n_k_flag)) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "roi read: must specify a volume"
                                 "depth with either in_like or in_k_count");
      if (cmdargs.in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    if (cmdargs.in_like_flag) {
      mri = MRIreadGeRoi(cmdargs.in_name.data(), mri_in_like->depth);
    } else {
      mri = MRIreadGeRoi(cmdargs.in_name.data(), cmdargs.in_n_k);
    }

    if (mri == nullptr) {
      if (cmdargs.in_like_flag) {
        MRIfree(&mri_in_like);
      }
      exit(1);
    }

    cmdargs.resample_type_val = SAMPLE_NEAREST;
    cmdargs.no_scale_flag = TRUE;
  } else {
    if ((cmdargs.read_only_flag) &&
        ((cmdargs.in_info_flag) || (cmdargs.in_matrix_flag)) &&
        (!cmdargs.in_stats_flag)) {
      if (cmdargs.force_in_type_flag) {
        mri = MRIreadHeader(cmdargs.in_name.data(), in_volume_type);
      } else {
        mri = MRIreadInfo(cmdargs.in_name.data());
      }
    } else {
      if (cmdargs.force_in_type_flag) {
        // fmt::printf("MRIreadType()\n");
        mri = MRIreadType(cmdargs.in_name.data(), in_volume_type);
      } else {
        if (cmdargs.nthframe < 0) {
          if (!cmdargs.in_stats_table_flag) {
            mri = MRIread(cmdargs.in_name.data());
          } else {
            fmt::printf("Loading in stat table %s\n", cmdargs.in_name.data());
            StatTable = LoadStatTable(cmdargs.in_name.data());
            if (StatTable == nullptr) {
              fmt::printf("ERROR: loading y %s as a stat table\n",
                          cmdargs.in_name.data());
              exit(1);
            }
            mri = StatTable->mri;
          }
        } else {
          mri = MRIreadEx(cmdargs.in_name.data(), cmdargs.nthframe);
        }
      }
    }
  }

  if (mri == nullptr) {
    if (cmdargs.in_like_flag) {
      MRIfree(&mri_in_like);
    }
    exit(1);
  }

  if (cmdargs.outside_val > 0) {
    mri->outside_val = cmdargs.outside_val;
  }
  if (cmdargs.upsample_flag) {
    fmt::printf("UpsampleFactor = %d\n", cmdargs.upsample_factor);
    mritmp = MRIupsampleN(mri, nullptr, cmdargs.upsample_factor);
    MRIfree(&mri);
    mri = mritmp;
  }

  if (cmdargs.slice_crop_flag) {
    fmt::printf("Cropping slices from %d to %d\n", cmdargs.slice_crop_start,
                cmdargs.slice_crop_stop);
    mri2 = MRIcrop(mri, 0, 0, cmdargs.slice_crop_start, mri->width - 1,
                   mri->height - 1, cmdargs.slice_crop_stop);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cmdargs.left_right_reverse) {
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

  if (cmdargs.left_right_swap_label) {
    fmt::printf("Performing left-right swap of labels\n");
    // Good for aseg, aparc+aseg, wmparc, etc. Does not change geometry
    mri2 = MRIlrswapAseg(mri);
    MRIfree(&mri);
    mri = mri2;
  }

  if (cmdargs.left_right_reverse_pix) {
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

  if ((cmdargs.left_right_mirror_flag) ||
      (cmdargs.left_right_keep_flag)) //(mr, 2012-03-08)
  {
    // Mirror one half of the image into the other half (left-right)
    // the split point is at the middle of the dimension that is
    // most left-right oriented. The header geometry is not changed
    // If LeftRightKeep, then don't mirror but fill with zero and
    // only keep the specified hemisphere
    fmt::printf("WARNING: Mirroring %s half into the other half,\n"
                "or masking one half of the image is only meaningful if\n"
                "the image is upright and centerd, see make_upright.\n",
                cmdargs.left_right_mirror_hemi.data());

    MRIdircosToOrientationString(mri, ostr.data());
    fmt::printf("  Orientation string: %s\n", ostr.data());
    mri2 = MRIcopy(mri, nullptr);
    v = 0;
    if (ostr[0] == 'L' || ostr[0] == 'R') {
      fmt::printf("  Mirror or keep pixels for the columns\n");
      for (c = 0; c < mri->width / 2; c++) {
        c1 = c;
        if (ostr[0] == toupper(cmdargs.left_right_mirror_hemi[0])) {
          c1 = mri->width - c - 1;
        }
        c2 = mri->width - c1 - 1;
        for (r = 0; r < mri->height; r++) {
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              if (cmdargs.left_right_mirror_flag) {
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
          if (ostr[1] == toupper(cmdargs.left_right_mirror_hemi[0])) {
            r1 = mri->height - r - 1;
          }
          r2 = mri->height - r1 - 1;
          for (s = 0; s < mri->depth; s++) {
            for (f = 0; f < mri->nframes; f++) {
              if (cmdargs.left_right_mirror_flag) {
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
            if (ostr[2] == toupper(cmdargs.left_right_mirror_hemi[0])) {
              s1 = mri->depth - s - 1;
            }
            s2 = mri->depth - s1 - 1;
            for (f = 0; f < mri->nframes; f++) {
              if (cmdargs.left_right_mirror_flag) {
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

  if (cmdargs.flip_cols) {
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

  if (cmdargs.slice_reverse) {
    fmt::printf("Reversing slices, updating vox2ras\n");
    mri2 = MRIreverseSlices(mri, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cmdargs.SliceBias) {
    fmt::printf("Applying Half-Cosine Slice Bias, Alpha = %g\n",
                cmdargs.SliceBiasAlpha);
    MRIhalfCosBias(mri, cmdargs.SliceBiasAlpha, mri);
  }

  if (cmdargs.reduce > 0) {
    MRI *mri_tmp;
    int r;

    if (mri->depth == 1) {
      for (r = 0; r < cmdargs.reduce; r++) {
        mri_tmp = MRIreduce2D(mri, nullptr);
        MRIfree(&mri);
        mri = mri_tmp;
      }
    } else {
      for (r = 0; r < cmdargs.reduce; r++) {
        mri_tmp = MRIreduce(mri, nullptr);
        MRIfree(&mri);
        mri = mri_tmp;
      }
    }
  }

  if (cmdargs.fwhm > 0) {
    fmt::printf("Smoothing input at fwhm = %g, gstd = %g\n", cmdargs.fwhm,
                cmdargs.gstd);
    MRIgaussianSmooth(mri, cmdargs.gstd, 1, mri);
  }

  if (!FEQUAL(cmdargs.rescale_factor, 1.0)) {
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
                cmdargs.rescale_factor);
    MRIscalarMul(mri, mri, cmdargs.rescale_factor / globalmean);
  }

  MRIaddCommandLine(mri, cmdline.data());
  if (!FEQUAL(cmdargs.scale_factor, 1.0)) {
    fmt::printf("scaling input volume by %2.3f\n", cmdargs.scale_factor);
    MRIscalarMul(mri, mri, cmdargs.scale_factor);
  }

  if (cmdargs.zero_ge_z_offset_flag) // E/
  {
    mri->c_s = 0.0;
  }

  fmt::printf("TR=%2.2f, TE=%2.2f, TI=%2.2f, flip angle=%2.2f\n", mri->tr,
              mri->te, mri->ti, DEGREES(mri->flip_angle));
  if (in_volume_type != OTL_FILE) {
    if (cmdargs.fill_parcellation_flag) {
      fmt::printf("fill_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
    if (cmdargs.smooth_parcellation_count != 0) {
      fmt::printf("smooth_parcellation flag ignored on a "
                  "non-parcellation read\n");
    }
  }

  /* ----- apply the in_like volume if it's been read ----- */
  if (cmdargs.in_like_flag) {
    if (mri->width != mri_in_like->width ||
        mri->height != mri_in_like->height ||
        mri->depth != mri_in_like->depth) {
      errno = 0;
      ErrorPrintf(ERROR_BADPARM, "volume sizes do not match\n");
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  cmdargs.in_name.c_str(), mri->width, mri->height, mri->depth,
                  mri->nframes);
      ErrorPrintf(ERROR_BADPARM,
                  "%s: (width, height, depth, frames) "
                  "= (%d, %d, %d, %d)\n",
                  cmdargs.in_like_name.c_str(), mri_in_like->width,
                  mri_in_like->height, mri_in_like->depth,
                  mri_in_like->nframes);
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
                  cmdargs.in_like_name.c_str(), cmdargs.in_name.data());
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
    if (cmdargs.force_ras_good) {
      fmt::printf("However, you have specified that the "
                  "default orientation should\n"
                  "be used with by adding --force_ras_good "
                  "on the command-line.\n");
      mri->ras_good_flag = 1;
    }
    if ((cmdargs.in_i_direction_flag) || (cmdargs.in_j_direction_flag) ||
        (cmdargs.in_k_direction_flag)) {
      fmt::printf("However, you have specified one or more "
                  "orientations on the \n"
                  "command-line using -i?d or --in-?-direction (?=i,j,k).\n");
      mri->ras_good_flag = 1;
    }
  }

  /* ----- apply command-line parameters ----- */
  if (cmdargs.in_i_size_flag) {
    mri->xsize = cmdargs.in_i_size;
  }
  if (cmdargs.in_j_size_flag) {
    mri->ysize = cmdargs.in_j_size;
  }
  if (cmdargs.in_k_size_flag) {
    mri->zsize = cmdargs.in_k_size;
  }
  if (cmdargs.in_i_direction_flag) {
    mri->x_r = cmdargs.in_i_directions[0];
    mri->x_a = cmdargs.in_i_directions[1];
    mri->x_s = cmdargs.in_i_directions[2];
    mri->ras_good_flag = 1;
  }
  if (cmdargs.in_j_direction_flag) {
    mri->y_r = cmdargs.in_j_directions[0];
    mri->y_a = cmdargs.in_j_directions[1];
    mri->y_s = cmdargs.in_j_directions[2];
    mri->ras_good_flag = 1;
  }
  if (cmdargs.in_k_direction_flag) {
    mri->z_r = cmdargs.in_k_directions[0];
    mri->z_a = cmdargs.in_k_directions[1];
    mri->z_s = cmdargs.in_k_directions[2];
    mri->ras_good_flag = 1;
  }
  if (cmdargs.delete_cmds) {
    mri->ncmds = 0;
  }
  if (!cmdargs.new_transform_fname.empty()) {
    fmt::printf("Changing xform name to %s\n",
                cmdargs.new_transform_fname.data());
    strcpy(mri->transform_fname, cmdargs.new_transform_fname.data());
  }
  if (!cmdargs.in_orientation_string.empty()) {
    fmt::printf("Setting input orientation to %s\n",
                cmdargs.in_orientation_string.data());
    MRIorientationStringToDircos(mri, cmdargs.in_orientation_string.data());
    mri->ras_good_flag = 1;
  }
  if (!cmdargs.in_center.empty()) {
    mri->c_r = cmdargs.in_center[0];
    mri->c_a = cmdargs.in_center[1];
    mri->c_s = cmdargs.in_center[2];
  }
  if (!cmdargs.delta_in_center.empty()) {
    mri->c_r += cmdargs.delta_in_center[0];
    mri->c_a += cmdargs.delta_in_center[1];
    mri->c_s += cmdargs.delta_in_center[2];
  }
  if (cmdargs.subject_name[0] != '\0') {
    strcpy(mri->subject_name, cmdargs.subject_name.data());
  }

  if (cmdargs.in_tr_flag) {
    mri->tr = cmdargs.in_tr;
  }
  if (cmdargs.in_ti_flag) {
    mri->ti = cmdargs.in_ti;
  }
  if (cmdargs.in_te_flag) {
    mri->te = cmdargs.in_te;
  }
  if (cmdargs.in_flip_angle_flag) {
    mri->flip_angle = cmdargs.in_flip_angle;
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((cmdargs.in_i_size_flag) || (cmdargs.in_j_size_flag) ||
      (cmdargs.in_k_size_flag)) {

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
  if (cmdargs.in_info_flag) {
    fmt::printf("input structure:\n");
    MRIdump(mri, stdout);
  }

  if (cmdargs.in_matrix_flag) {
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
  if (cmdargs.in_stats_flag) {
    MRIprintStats(mri, stdout);
  }

  // Load the transform
  if (cmdargs.transform_flag) {
    fmt::printf("INFO: Reading transformation from file %s...\n",
                cmdargs.transform_fname.data());
    transform_type = TransformFileNameType(cmdargs.transform_fname.data());
    if (transform_type == MNI_TRANSFORM_TYPE ||
        transform_type == TRANSFORM_ARRAY_TYPE ||
        transform_type == REGISTER_DAT || transform_type == FSLREG_TYPE) {
      fmt::printf("Reading transform with LTAreadEx()\n");
      // lta_transform = LTAread(transform_fname);
      lta_transform = LTAreadEx(cmdargs.transform_fname.data());
      if (lta_transform == nullptr) {
        fmt::fprintf(stderr, "ERROR: Reading transform from file %s\n",
                     cmdargs.transform_fname.data());
        exit(1);
      }
      if (transform_type == FSLREG_TYPE) {
        MRI *tmp = nullptr;
        if (!cmdargs.out_like_flag) {
          fmt::printf("ERROR: fslmat does not have the information "
                      "on the dst volume\n");
          fmt::printf(
              "ERROR: you must give option '--like volume' to specify the"
              " dst volume info\n");
          MRIfree(&mri);
          exit(1);
        }
        // now setup dst volume info
        tmp = MRIreadHeader(cmdargs.out_like_name.data(),
                            MRI_VOLUME_TYPE_UNKNOWN);
        // flsmat does not contain src and dst info
        LTAmodifySrcDstGeom(lta_transform, mri, tmp);
        // add src and dst information
        LTAchangeType(lta_transform, LINEAR_VOX_TO_VOX);
        MRIfree(&tmp);
      } // end FSLREG_TYPE

      if (cmdargs.DevXFM != 0) {
        fmt::printf("INFO: devolving XFM (%s)\n",
                    cmdargs.devolvexfm_subject.data());
        fmt::printf("-------- before ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        T = DevolveXFM(cmdargs.devolvexfm_subject.data(),
                       lta_transform->xforms[0].m_L, nullptr);
        if (T == nullptr) {
          exit(1);
        }
        fmt::printf("-------- after ---------\n");
        MatrixPrint(stdout, lta_transform->xforms[0].m_L);
        fmt::printf("-----------------\n");
      } // end DevXFM

      if (cmdargs.invert_transform_flag) {
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
          strcpy(buf, cmdargs.transform_fname.data());
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

  if (cmdargs.reslice_like_flag) {

    if (cmdargs.force_template_type_flag) {
      fmt::printf("reading template info from (type %s) volume %s...\n",
                  cmdargs.template_type_string.data(),
                  cmdargs.reslice_like_name.data());
      mri_template = MRIreadHeader(cmdargs.reslice_like_name.data(),
                                   cmdargs.forced_template_type);
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     cmdargs.reslice_like_name.data());
        exit(1);
      }
    } else {
      fmt::printf("reading template info from volume %s...\n",
                  cmdargs.reslice_like_name.data());
      mri_template = MRIreadInfo(cmdargs.reslice_like_name.data());
      if (mri_template == nullptr) {
        fmt::fprintf(stderr, "error reading from volume %s\n",
                     cmdargs.reslice_like_name.data());
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

    if (cmdargs.conform_flag) {
      conform_width = 256;
      if (cmdargs.conform_min_flag) {
        cmdargs.conform_size = MRIfindMinSize(mri, &conform_width);
      } else {
        if (cmdargs.conform_width_256_flag) {
          conform_width = 256; // force it
        } else {
          conform_width = MRIfindRightSize(mri, cmdargs.conform_size);
        }
      }
      mri_template = MRIconformedTemplate(
          mri, conform_width, cmdargs.conform_size, cmdargs.conf_keep_dc);
    } else if (!cmdargs.voxel_size.empty()) {
      mri_template = MRIallocHeader(mri->width, mri->height, mri->depth,
                                    mri->type, mri->nframes);
      MRIcopyHeader(mri, mri_template);

      mri_template->nframes = mri->nframes;

      mri_template->width = static_cast<int>(
          ceil(mri->xsize * mri->width / cmdargs.voxel_size[0]));
      mri_template->height = static_cast<int>(
          ceil(mri->ysize * mri->height / cmdargs.voxel_size[1]));
      mri_template->depth = static_cast<int>(
          ceil(mri->zsize * mri->depth / cmdargs.voxel_size[2]));

      mri_template->xsize = cmdargs.voxel_size[0];
      mri_template->ysize = cmdargs.voxel_size[1];
      mri_template->zsize = cmdargs.voxel_size[2];

      mri_template->xstart = -mri_template->width / 2;
      mri_template->xend = mri_template->width / 2;
      mri_template->ystart = -mri_template->height / 2;
      mri_template->yend = mri_template->height / 2;
      mri_template->zstart = -mri_template->depth / 2;
      mri_template->zend = mri_template->depth / 2;

    } else if (!cmdargs.downsample_factor.empty()) {
      mri_template = MRIallocHeader(mri->width, mri->height, mri->depth,
                                    mri->type, mri->nframes);
      MRIcopyHeader(mri, mri_template);

      mri_template->nframes = mri->nframes;

      mri_template->width =
          static_cast<int>(ceil(mri->width / cmdargs.downsample_factor[0]));
      mri_template->height =
          static_cast<int>(ceil(mri->height / cmdargs.downsample_factor[1]));
      mri_template->depth =
          static_cast<int>(ceil(mri->depth / cmdargs.downsample_factor[2]));

      mri_template->xsize *= cmdargs.downsample_factor[0];
      mri_template->ysize *= cmdargs.downsample_factor[1];
      mri_template->zsize *= cmdargs.downsample_factor[2];

      mri_template->xstart = -mri_template->xsize * mri_template->width / 2;
      mri_template->xend = mri_template->xsize * mri_template->width / 2;
      mri_template->ystart = -mri_template->ysize * mri_template->height / 2;
      mri_template->yend = mri_template->ysize * mri_template->height / 2;
      mri_template->zstart = -mri_template->zsize * mri_template->depth / 2;
      mri_template->zend = mri_template->zsize * mri_template->depth / 2;
    }
  }

  /* ----- apply command-line parameters ----- */
  if (cmdargs.out_i_size_flag) {
    float scale;
    scale = mri_template->xsize / cmdargs.out_i_size;
    mri_template->xsize = cmdargs.out_i_size;
    mri_template->width = nint(mri_template->width * scale);
  }
  if (cmdargs.out_j_size_flag) {
    float scale;
    scale = mri_template->ysize / cmdargs.out_j_size;
    mri_template->ysize = cmdargs.out_j_size;
    mri_template->height = nint(mri_template->height * scale);
  }
  if (cmdargs.out_k_size_flag) {
    float scale;
    scale = mri_template->zsize / cmdargs.out_k_size;
    mri_template->zsize = cmdargs.out_k_size;
    mri_template->depth = nint(mri_template->depth * scale);
  }
  if (cmdargs.out_n_i_flag) {
    mri_template->width = cmdargs.out_n_i;
  }
  if (cmdargs.out_n_j_flag) {
    mri_template->height = cmdargs.out_n_j;
  }
  if (cmdargs.out_n_k_flag) {
    mri_template->depth = cmdargs.out_n_k;
    mri_template->imnr1 = mri_template->imnr0 + cmdargs.out_n_k - 1;
  }
  if (cmdargs.out_i_direction_flag) {
    mri_template->x_r = cmdargs.out_i_directions[0];
    mri_template->x_a = cmdargs.out_i_directions[1];
    mri_template->x_s = cmdargs.out_i_directions[2];
  }
  if (cmdargs.out_j_direction_flag) {
    mri_template->y_r = cmdargs.out_j_directions[0];
    mri_template->y_a = cmdargs.out_j_directions[1];
    mri_template->y_s = cmdargs.out_j_directions[2];
  }
  if (cmdargs.out_k_direction_flag) {
    mri_template->z_r = cmdargs.out_k_directions[0];
    mri_template->z_a = cmdargs.out_k_directions[1];
    mri_template->z_s = cmdargs.out_k_directions[2];
  }
  if (!cmdargs.out_orientation_string.empty()) {
    fmt::printf("Setting output orientation to %s\n",
                cmdargs.out_orientation_string.data());
    MRIorientationStringToDircos(mri_template,
                                 cmdargs.out_orientation_string.data());
  }
  if (!cmdargs.out_center.empty()) {
    mri_template->c_r = cmdargs.out_center[0];
    mri_template->c_a = cmdargs.out_center[1];
    mri_template->c_s = cmdargs.out_center[2];
  }

  /* ----- correct starts, ends, and fov if necessary ----- */
  if ((cmdargs.out_i_size_flag) || (cmdargs.out_j_size_flag) ||
      (cmdargs.out_k_size_flag) || (cmdargs.out_n_i_flag) ||
      (cmdargs.out_n_j_flag) || (cmdargs.out_n_k_flag)) {

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
  if (cmdargs.out_data_type >= 0) {
    mri_template->type = cmdargs.out_data_type;
  }

  /* ----- catch the mri_template info flag ----- */
  if (cmdargs.template_info_flag) {
    fmt::printf("template structure:\n");
    MRIdump(mri_template, stdout);
  }

  /* ----- exit here if read only is desired ----- */
  if (cmdargs.read_only_flag) {
    exit(0);
  }

  /* ----- apply a transformation if requested ----- */
  if (cmdargs.transform_flag) {
    fmt::printf("INFO: Applying transformation from file %s...\n",
                cmdargs.transform_fname.data());
    transform_type = TransformFileNameType(cmdargs.transform_fname.data());
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
      if (cmdargs.out_like_flag) {
        MRI *tmp = nullptr;
        fmt::printf(
            "INFO: transform dst into the like-volume (resample_type %d)\n",
            cmdargs.resample_type_val);
        tmp = MRIreadHeader(cmdargs.out_like_name.data(),
                            MRI_VOLUME_TYPE_UNKNOWN);
        mri_transformed =
            MRIalloc(tmp->width, tmp->height, tmp->depth, mri->type);
        if (mri_transformed == nullptr) {
          ErrorExit(ERROR_NOMEMORY, "could not allocate memory");
        }
        MRIcopyHeader(tmp, mri_transformed);
        MRIfree(&tmp);
        tmp = nullptr;
        mri_transformed = LTAtransformInterp(
            mri, mri_transformed, lta_transform, cmdargs.resample_type_val);
      } else {
        if (!cmdargs.out_center.empty()) {
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
                    cmdargs.resample_type_val);
        if (Gdiag_no > 0) {
          fmt::printf(
              "Dumping LTA ---------++++++++++++-----------------------\n");
          LTAdump(stdout, lta_transform);
          fmt::printf(
              "========-------------++++++++++++-------------------=====\n");
        }
        mri_transformed = LTAtransformInterp(mri, nullptr, lta_transform,
                                             cmdargs.resample_type_val);
      } // end out_like_flag treatment

      if (mri_transformed == nullptr) {
        fmt::fprintf(stderr, "ERROR: applying transform to volume\n");
        exit(1);
      }
      if (!cmdargs.out_center.empty()) {
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
      TRANSFORM *tran = TransformRead(cmdargs.transform_fname.data());
      // check whether the volume to be morphed and the morph have the same
      // dimensions
      if (tran == nullptr) {
        ErrorExit(ERROR_NOFILE, "%s: could not read xform from %s\n", Progname,
                  cmdargs.transform_fname.c_str());
      }
      if (!cmdargs.invert_transform_flag) {
        fmt::printf("morphing to atlas with resample type %d\n",
                    cmdargs.resample_type_val);
        mri_transformed =
            GCAMmorphToAtlas(mri, static_cast<GCA_MORPH *>(tran->xform),
                             nullptr, 0, cmdargs.resample_type_val);
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
        else // when morphing atlas to subject using -ait <3d morph> must
             // resample atlas to 256 before applying morph
        {
          MRI *mri_tmp;
          mri_transformed = MRIalloc(gcam->image.width, gcam->image.height,
                                     gcam->image.depth, mri->type);
          useVolGeomToMRI(&gcam->image, mri_transformed);
          mri_tmp = MRIresample(mri, mri_transformed, resample_type_val);
          MRIfree(&mri);
          MRIfree(&mri_template);
          mri = mri_tmp;
        }
#endif
        fmt::printf("morphing from atlas with resample type %d\n",
                    cmdargs.resample_type_val);
        mri_transformed = GCAMmorphFromAtlas(mri, gcam, mri_transformed,
                                             cmdargs.resample_type_val);
        MRIwrite(mri_transformed, "t.mgz");
      }
      TransformFree(&tran);
      MRIfree(&mri);
      mri = mri_transformed;
    } else {
      fmt::fprintf(stderr, "unknown transform type in file %s\n",
                   cmdargs.transform_fname.data());
      exit(1);
    }
  } else if (((cmdargs.out_like_flag)) &&
             (!cmdargs.out_stats_table_flag)) // flag set but no transform
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
                cmdargs.out_like_name.data());
    tmp = MRIreadHeader(cmdargs.out_like_name.data(), MRI_VOLUME_TYPE_UNKNOWN);
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
  if (mri->type != mri_template->type && !cmdargs.nochange_flag) {
    fmt::printf("changing data type from %s to %s (noscale = %d)...\n",
                MRItype2str(mri->type), MRItype2str(mri_template->type),
                cmdargs.no_scale_flag);
    mri2 = MRISeqchangeType(mri, mri_template->type, 0.0, 0.999,
                            static_cast<int>(cmdargs.no_scale_flag));
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

#if 0
  printf("mri  vs.  mri_template\n");
  fmt::printf(" dim  ( %i , %i , %i ) vs ( %i , %i , %i)\n", mri->width,
              mri->height, mri->depth, mri_template->width,
              mri_template->height, mri_template->depth);
  fmt::printf(" size ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g)\n",
              mri->xsize, mri->ysize, mri->zsize, mri_template->xsize,
              mri_template->ysize, mri_template->zsize);
  fmt::printf(" xras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g)\n",
              mri->x_r, mri->x_a, mri->x_s, mri_template->x_r,
              mri_template->x_a, mri_template->x_s);
  fmt::printf(" yras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g)\n",
              mri->y_r, mri->x_a, mri->y_s, mri_template->y_r,
              mri_template->y_a, mri_template->y_s);
  fmt::printf(" zras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g)\n",
              mri->z_r, mri->x_a, mri->z_s, mri_template->z_r,
              mri_template->z_a, mri_template->z_s);
  fmt::printf(" cras ( %.9g , %.9g , %.9g ) vs ( %.9g , %.9g , %.9g)\n",
              mri->c_r, mri->c_a, mri->c_s, mri_template->c_r,
              mri_template->c_a, mri_template->c_s);
#endif

  /* ----- reslice if necessary and not performed during transform ----- */
  constexpr auto eps =
      static_cast<const float>(1e-05); /* (mr) do eps testing to avoid reslicing
                        due to tiny differences, e.g. from IO */
  if ((!cmdargs.out_like_flag) && (transform_type != MORPH_3D_TYPE) &&
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
    switch (cmdargs.resample_type_val) {
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
    mri2 = MRIresample(mri, mri_template, cmdargs.resample_type_val);
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
  if (!cmdargs.reorder_vals.empty()) {
    fmt::printf("reordering axes...\n");
    mri2 = MRIreorder(mri, nullptr, cmdargs.reorder_vals[0],
                      cmdargs.reorder_vals[1], cmdargs.reorder_vals[2]);
    if (mri2 == nullptr) {
      fmt::fprintf(stderr, "error reordering axes\n");
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- reorder if necessary ----- */
  if (!cmdargs.reorder4_vals.empty()) {
    fmt::printf("reordering all axes...\n");
    fmt::printf("reordering, values are %d %d %d %d\n",
                cmdargs.reorder4_vals[0], cmdargs.reorder4_vals[1],
                cmdargs.reorder4_vals[2], cmdargs.reorder4_vals[3]);
    mri2 = MRIreorder4(mri, cmdargs.reorder4_vals.data());
    if (mri2 == nullptr) {
      fmt::fprintf(stderr, "error reordering all axes\n");
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- store the gdf file stem ----- */
  strcpy(mri->gdf_image_stem, cmdargs.gdf_image_stem.data());

  /* ----- catch the out info flag ----- */
  if (cmdargs.out_info_flag) {
    fmt::printf("output structure:\n");
    MRIdump(mri, stdout);
  }

  /* ----- catch the out matrix flag ----- */
  if (cmdargs.out_matrix_flag) {
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
  if (cmdargs.out_stats_flag) {
    MRIprintStats(mri, stdout);
  }

  if (cmdargs.erode_seg_flag) {
    fmt::printf("Eroding segmentation %d\n", cmdargs.n_erode_seg);
    mri2 = MRIerodeSegmentation(mri, nullptr, cmdargs.n_erode_seg, 0);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (cmdargs.dil_seg_flag) {
    mritmp = nullptr;
    if (cmdargs.dil_seg_mask[0] == '\0') {
      fmt::printf("Dilating segmentation %d\n", cmdargs.n_dil_seg);
    } else {
      fmt::printf("Dilating segmentation, mask %s\n",
                  cmdargs.dil_seg_mask.data());
      mritmp = MRIread(cmdargs.dil_seg_mask.data());
      if (mritmp == nullptr) {
        fmt::printf("ERROR: could not read %s\n", cmdargs.dil_seg_mask.data());
        exit(1);
      }
    }
    mri2 = MRIdilateSegmentation(mri, nullptr, cmdargs.n_dil_seg, mritmp, 0,
                                 0.5, &i);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cmdargs.frame_flag) {
    if (cmdargs.mid_frame_flag) {
      nframes = 1;
      cmdargs.frames[0] = nint(mri->nframes / 2);
    }
    if (nframes == 1) {
      fmt::printf("keeping frame %d\n", cmdargs.frames[0]);
    } else {
      fmt::printf("keeping frames");
      for (f = 0; f < nframes; f++) {
        fmt::printf(" %d", cmdargs.frames[f]);
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
      if (cmdargs.frames[f] < 0 || cmdargs.frames[f] >= mri->nframes) {
        fmt::printf("   ERROR: valid frame numbers are between 0 and %d\n",
                    mri->nframes - 1);
        exit(1);
      } else {
        for (s = 0; s < mri2->depth; s++) {
          for (r = 0; r < mri2->height; r++) {
            for (c = 0; c < mri2->width; c++) {
              MRIsetVoxVal(mri2, c, r, s, f,
                           MRIgetVoxVal(mri, c, r, s, cmdargs.frames[f]));
            }
          }
        }
      }
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- drop the last ndrop frames (drop before skip) ------ */
  if (cmdargs.ndrop > 0) {
    fmt::printf("Dropping last %d frames\n", cmdargs.ndrop);
    if (mri->nframes <= cmdargs.ndrop) {
      fmt::printf("   ERROR: can't drop, volume only has %d frames\n",
                  mri->nframes);
      exit(1);
    }
    mri2 = fMRIndrop(mri, cmdargs.ndrop, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- skip the first nskip frames ------ */
  if (cmdargs.nskip > 0) {
    fmt::printf("Skipping %d frames\n", cmdargs.nskip);
    if (mri->nframes <= cmdargs.nskip) {
      fmt::printf("   ERROR: can't skip, volume only has %d frames\n",
                  mri->nframes);
      exit(1);
    }
    mri2 = fMRInskip(mri, cmdargs.nskip, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }
  if (cmdargs.subsample_flag) {
    fmt::printf("SubSample: Start = %d  Delta = %d, End = %d\n",
                cmdargs.SubSampStart, cmdargs.SubSampDelta, cmdargs.SubSampEnd);
    mri2 = fMRIsubSample(mri, cmdargs.SubSampStart, cmdargs.SubSampDelta,
                         cmdargs.SubSampEnd, nullptr);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  if (cmdargs.cutends_flag) {
    fmt::printf("Cutting ends: n = %d\n", cmdargs.ncutends);
    mri2 = MRIcutEndSlices(mri, cmdargs.ncutends);
    if (mri2 == nullptr) {
      exit(1);
    }
    MRIfree(&mri);
    mri = mri2;
  }

  /* ----- crops ---------*/
  if (cmdargs.crop_flag) {
    MRI *mri_tmp;
    int x0;
    int y0;
    int z0;

    x0 = cmdargs.crop_center[0];
    y0 = cmdargs.crop_center[1];
    z0 = cmdargs.crop_center[2];

    x0 -= cmdargs.cropsize[0] / 2;
    y0 -= cmdargs.cropsize[1] / 2;
    z0 -= cmdargs.cropsize[2] / 2;
    mri_tmp = MRIextract(mri, nullptr, x0, y0, z0, cmdargs.cropsize[0],
                         cmdargs.cropsize[1], cmdargs.cropsize[2]);
    MRIfree(&mri);
    mri = mri_tmp;
  }

  if (!FEQUAL(cmdargs.out_scale_factor, 1.0)) {
    fmt::printf("scaling output intensities by %2.3f\n",
                cmdargs.out_scale_factor);
    MRIscalarMul(mri, mri, cmdargs.out_scale_factor);
  }

  if (cmdargs.sphinx_flag) {
    fmt::printf("Changing orientation information to Sphinx\n");
    MRIhfs2Sphinx(mri);
  }

  if (cmdargs.AutoAlign != nullptr) {
    mri->AutoAlign = cmdargs.AutoAlign;
  }

  // ----- modify color lookup table (specified by --ctab option) -----
  if (cmdargs.colortablefile == "remove") {
    // remove an embedded ctab
    if (mri->ct != nullptr) {
      std::cout << "removing color lookup table" << std::endl;
      CTABfree(&mri->ct);
    }
  } else if (!cmdargs.colortablefile.empty()) {
    // add a user-specified ctab
    std::cout << "embedding color lookup table" << std::endl;
    if (mri->ct != nullptr) {
      CTABfree(&mri->ct);
    }
    mri->ct = CTABreadASCII(cmdargs.colortablefile.data());
    if (mri->ct == nullptr) {
      fs::fatal() << "could not read lookup table from "
                  << cmdargs.colortablefile.data();
    }
  }

  /*------ Finally, write the output -----*/

  if (cmdargs.out_stats_table_flag) {
    fmt::printf("Writing as Stats-Table to %s using template %s\n",
                cmdargs.out_name.data(), cmdargs.out_like_name.data());

    OutStatTable = InitStatTableFromMRI(mri, cmdargs.out_like_name.data());
    WriteStatTable(cmdargs.out_name.data(), OutStatTable);

    fmt::printf("done\n");
    exit(0);
  }

  if (cmdargs.ascii_flag != 0) {
    fmt::printf("Writing as ASCII to %s\n", cmdargs.out_name.data());
    fptmp = fopen(cmdargs.out_name.data(), "w");
    if (cmdargs.ascii_flag == 1 || cmdargs.ascii_flag == 2) {
      for (f = 0; f < mri->nframes; f++) {
        for (s = 0; s < mri->depth; s++) {
          for (r = 0; r < mri->height; r++) {
            for (c = 0; c < mri->width; c++) {
              if (cmdargs.ascii_flag == 1) {
                fmt::fprintf(fptmp, "%lf\n", MRIgetVoxVal(mri, c, r, s, f));
              }
              if (cmdargs.ascii_flag == 2) {
                fmt::fprintf(fptmp, "%3d %3d %3d %3d %lf\n", c, r, s, f,
                             MRIgetVoxVal(mri, c, r, s, f));
              }
            }
          }
        }
      }
    }
    if (cmdargs.ascii_flag == 3) {
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
  if (!cmdargs.no_write_flag) {
    if (cmdargs.split_frames_flag) {
      fmt::printf("writing to %s...\n", cmdargs.out_name.data());
      if (cmdargs.force_out_type_flag) {
        err = MRIwriteType(mri, cmdargs.out_name.data(), out_volume_type);
        if (err != NO_ERROR) {
          fmt::printf("ERROR: failure writing %s as volume type %d\n",
                      cmdargs.out_name.data(), out_volume_type);
          exit(1);
        }
      } else {
        err = MRIwrite(mri, cmdargs.out_name.data());
        if (err != NO_ERROR) {
          fmt::printf("ERROR: failure writing %s\n", cmdargs.out_name.data());
          exit(1);
        }
      }
    } else {
      stem = IDstemFromName(cmdargs.out_name.data());
      ext = IDextensionFromName(cmdargs.out_name.data());

      fmt::printf("splitting frames, stem = %s, ext = %s\n", stem, ext);
      mri2 = nullptr;
      for (i = 0; i < mri->nframes; i++) {
        mri2 = MRIcopyFrame(mri, mri2, i, 0);
        tmpstr = fmt::sprintf("%s%04d.%s", stem, i, ext);
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
                           .style(fs::util::cli::po_style)
                           .run();

    po::store(parsed_opts, vm);
    fs::util::cli::print_parsed_tokens(parsed_opts);

    po::notify(vm);

    cmdargs->check_conflicts(vm);
    cmdargs->check_dependencies(vm);
  } catch (std::exception const &e) {
    spdlog::get("stderr")->critical(e.what());
    return false;
  }

  return true;
}

void initArgDesc(boost::program_options::options_description *desc,
                 CMDARGS *cmdargs, ENV *env) {

  namespace po = boost::program_options;
  namespace cli = fs::util::cli;
  namespace io = fs::util::io;
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
               handle_version_option(false, cmdargs->raw, env->vcid,
                                     "$Name:  $");
               exit(0);
             }
           }),

       "print out version and exit")

      //

      ("all-info",

       po::bool_switch() //
           ->notifier([cmdargs, env](auto v) {
             if (v) {
               handle_version_option(true, cmdargs->raw, env->vcid,
                                     "$Name:  $");
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
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->conform_flag = true;
           }),

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
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->conform_flag = true;
           }),

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
             cmdargs->transform_flag = true;
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
             cmdargs->slice_crop_flag = true;
             cmdargs->slice_crop_start = cmdargs->slice_crop[0];
             cmdargs->slice_crop_stop = cmdargs->slice_crop[1];
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
             cmdargs->transform_flag = true;
             cmdargs->invert_transform_flag = true;
           }),

       "Apply inverse transform given by xfm or m3z files.")

      //

      ("upsample",

       po::value(&cmdargs->upsample_factor) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->upsample_flag = true;
           }),

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
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->erode_seg_flag = true;
           }),

       "Erode segmentation boundaries Nerode times (based on 6 nearest "
       "neighbors)")

      //

      ("dil-seg",

       po::value(&cmdargs->n_dil_seg) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->dil_seg_flag = true;
           }),

       "Dilate segmentation boundaries Ndilate times (based on 6 nearest "
       "neighbors) to fill seg=0 voxels")

      //

      ("cutends",

       po::value(&cmdargs->ncutends) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->cutends_flag = true;
           }),

       "Remove ncut slices from the ends")

      //

      ("out_i_count,oni,oic",

       po::value(&cmdargs->out_n_i) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->out_n_i_flag = true;
           }),

       "out_i_count")

      //

      ("out_j_count,onj,ojc",

       po::value(&cmdargs->out_n_j) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->out_n_j_flag = true;
           }),

       "out_i_count")

      //

      ("out_k_count,onk,okc",

       po::value(&cmdargs->out_n_k) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->out_n_k_flag = true;
           }),

       "out_i_count")

      //

      ("downsample2,ds2",

       po::bool_switch(&cmdargs->downsample2_flag),

       "downsample2")

      //

      ("in_i_count,ini,iic",

       po::value(&cmdargs->in_n_i) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_n_i_flag = true;
           }),

       "in_i_count")

      //

      ("in_j_count,inj,ijc",

       po::value(&cmdargs->in_n_j) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_n_j_flag = true;
           }),

       "in_j_count")

      //

      ("in_k_count,ink,ikc",

       po::value(&cmdargs->in_n_k) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_n_k_flag = true;
           }),

       "in_k_count")

      //

      ("tr",

       po::value(&cmdargs->in_tr) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_tr_flag = true;
           }),

       "TR in msec")

      //

      ("TI",

       po::value(&cmdargs->in_ti) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_ti_flag = true;
           }),

       "TI in msec (note uppercase flag)")

      //

      ("te",

       po::value(&cmdargs->in_te) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_te_flag = true;
           }),

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
           ->notifier(cli::addConflicts({"read_only", "no_write"}, "out_name",
                                        cmdargs)),

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
           ->notifier([](auto /*unused*/) {
             setenv("FS_MOSAIC_FIX_NOASCII", "1", 1);
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
             std::tie(cmdargs->SubSampStart, cmdargs->SubSampDelta,
                      cmdargs->SubSampEnd) = std::array<int, 3>{
                 cmdargs->fsubsample[0], cmdargs->fsubsample[1],
                 cmdargs->fsubsample[2]};
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
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->SliceBias = true;
           }),

       "Apply half-cosine bias field")

      //

      ("in_like,il",

       po::value(&cmdargs->in_like_name) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->in_like_flag = true;
           }),

       "in_like")

      //

      ("color_file,cf",

       po::value(&cmdargs->color_file_name) //
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->color_file_flag = true;
           }),

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
           ->notifier([cmdargs](auto /*unused*/) {
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
           ->notifier([cmdargs](auto /*unused*/) {
             cmdargs->frame_flag = true;
           }),

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
               cmdargs->ascii_flag = 1;
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
               cmdargs->ascii_flag = 2;
               cmdargs->force_in_type_flag = true;
             }
           }),

       "Same as --ascii but includes col, row, slice, and frame")

      //

      ("ascii-fcol",

       po::bool_switch() //
           ->notifier([cmdargs](auto v) {
             if (v) {
               cmdargs->ascii_flag = 3;
               cmdargs->force_in_type_flag = true;
               fmt::printf("blowme please");
             }
           }),

       "ascii-fcol");
}
