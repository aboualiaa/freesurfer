/**
 * @file  mri_uchar.cpp
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:55 $
 *    $Revision: 1.4 $
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

//
// mri_uchar.cpp
//
// purpose: convert data to uchar value without scaling
//

#include <iomanip>
#include <iostream>

extern "C" {
#include "mri.h"

const char *Progname = "mri_uchar";
}

int main(int argc, char *argv[]) {
  if (argc < 1) {
    std::cerr << "Usage: mri_uchar <involume> <outvolume>" << std::endl;
    std::cerr << "       any involume val is set to uchar volume " << std::endl;
    std::cerr << "       i.e. -0.5 <= val < 0.5 becomes 0. " << std::endl;
    std::cerr
        << "       make sure that involume does not have values more than 255 "
        << std::endl;
    return -1;
  }
  MRI *src = MRIread(argv[1]);
  if (!src) {
    std::cerr << "could not open " << argv[1] << std::endl;
    return -1;
  }
  MRI *dst = MRIalloc(src->width, src->height, src->depth, MRI_UCHAR);
  if (!dst) {
    std::cerr << "could not allocate memory for the target" << std::endl;
    return -1;
  }
  // copy geometry information
  MRIcopyHeader(src, dst);

  int count = 0;
  for (int f = 0; f < src->nframes; f++)
    for (int k = 0; k < src->depth; k++)
      for (int j = 0; j < src->height; j++)
        for (int i = 0; i < src->width; i++) {
          float val = MRIgetVoxVal(src, i, j, k, f);
          // -0.5 up to 0.5(not including) becomes 0
          float fapp = floorf(val + 0.5);
          if (fapp > 0)
            count++;
          MRIsetVoxVal(dst, i, j, k, f, fapp);
        }
  std::cout << "non-zero value count = " << count << std::endl;
  MRIwrite(dst, argv[2]);
}
