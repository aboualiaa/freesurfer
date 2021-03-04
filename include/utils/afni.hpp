/*
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

#ifndef UTILS_AFNI_HPP
#define UTILS_AFNI_HPP

#include "mri.h"

namespace fs::utils {

struct AFNI_HEADER {
  // mandatory attributes
  int   dataset_rank[2];
  int   dataset_dimensions[3];
  char  typestring[16];
  int   scene_data[3];
  int   orient_specific[3];
  float origin[3];
  float delta[3];

  // almost mandatory attributes
  char * idcode_string;
  int    numchars;
  char   byteorder_string[10];
  float *brick_stats;
  int    numstats;
  int *  brick_types;
  int    numtypes;
  float *brick_float_facs;
  int    numfacs;
};

using AF = struct AFNI_HEADER;

auto afniRead(const char *fname, int read_volume) -> MRI *;
auto afniWrite(MRI *mri, const char *fname) -> int;
auto readAFNIHeader(FILE *fp, AF *paf) -> int;
void AFinit(AF &pAF);
void AFclean(AF *pAF);
void printAFNIHeader(AF &pAF);

} // namespace fs::utils

#endif
