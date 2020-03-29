/**
 * @file  i2rtest.cpp
 * @brief Testing routine to verify extract_i_to_r
 *
 */
/*
 * Original Author: Y. Tosa
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:55 $
 *    $Revision: 1.8 $
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

#include <iostream>
#if (__GNUC__ < 3)
#include "/usr/include/g++-3/alloc.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <matrix.h>
#include <mri.h>

#ifdef __cplusplus
}
#endif

// libutils needs Progname defined
const char *Progname = "i2rtest";

int main(int argc, char *argv[]) {
  std::string command;

  int    width, height, depth;
  double c_r, c_a, c_s;
  double xsize, ysize, zsize;

  std::string filename;
  if (argc < 2)
    filename = "./rot0.mgh";
  else
    filename = argv[1];

  MRI *mri;
  mri = MRIread(const_cast<char *>(filename.c_str()));

  width  = mri->width;
  height = mri->height;
  depth  = mri->depth;

  xsize = mri->xsize;
  ysize = mri->ysize;
  zsize = mri->zsize;

  c_r = mri->c_r;
  c_a = mri->c_a;
  c_s = mri->c_s;

  std::cout << "input file = " << filename.c_str() << std::endl;
  std::cout << "Input data ------------------------------------------------"
            << std::endl;
  std::cout << "width= " << width << "  height= " << height
            << "  depth= " << depth << std::endl;
  std::cout << "xsize= " << xsize << "  ysize = " << ysize
            << "  zsize= " << zsize << std::endl;
  std::cout << "c_r  = " << c_r << "  c_a   = " << c_a << "  c_s  = " << c_s
            << std::endl;
  std::cout << "ras_good_flag = " << mri->ras_good_flag << std::endl;
  std::cout << std::endl;

  MATRIX *m     = extract_i_to_r(mri);
  VECTOR *c     = VectorAlloc(4, MATRIX_REAL);
  c->rptr[1][1] = ((double)width) / 2.;
  c->rptr[2][1] = ((double)height) / 2.;
  c->rptr[3][1] = ((double)depth) / 2.;
  c->rptr[4][1] = 1.;

  // check the definition
  std::cout << "Check   C = M * c with c = (width/2, height/2, depth/2) "
            << std::endl;
  std::cout << "--------------------------------------------------------"
            << std::endl;
  VECTOR *C = MatrixMultiply(m, c, NULL);

  double C_r = C->rptr[1][1];
  double C_a = C->rptr[2][1];
  double C_s = C->rptr[3][1];
  std::cout << "Calculated values are" << std::endl;
  std::cout << "C_r  = " << C_r << "  C_a   = " << C_a << "  C_s  = " << C_s
            << std::endl;

  MatrixFree(&m);
  VectorFree(&c);
  VectorFree(&C);
  MRIfree(&mri);

  double tolerance = 0.000001;
  double crdiff    = C_r - c_r;
  double cadiff    = C_a - c_a;
  double csdiff    = C_s - c_s;
  if (crdiff > tolerance || cadiff > tolerance || csdiff > tolerance) {
    std::cout << "***********************Error in "
                 "extract_i_to_r()*********************** "
              << std::endl;
    std::cout << "cr diff = " << crdiff << "  ca diff = " << cadiff
              << "   cs diff = " << csdiff << std::endl;
    return -1;
  }
  std::cout << "No problem found." << std::endl;
  return 0;
}
