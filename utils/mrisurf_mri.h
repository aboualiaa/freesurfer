#pragma once
/*
 *
 */
/*
 * surfaces Author: Bruce Fischl, extracted from mrisurf.c by Bevin Brett
 *
 * $ © copyright-2014,2018 The General Hospital Corporation (Boston, MA) "MGH"
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
// Configurable support for checking that some of the parallel loops get the
// same results regardless of thread count
//
// Two common causes of this being false are
//          floating point reductions
//          random numbers being generated by parallel code
//

#include "mrisurf_vals.h"

int mrisFillFace(MRI_SURFACE *mris, MRI *mri, int fno);
int mrisHatchFace(MRI_SURFACE *mris, MRI *mri, int fno, int on);
int MRIScomputeMaxGradBorderValues(MRI_SURFACE *mris, MRI *mri_brain,
                                   MRI *mri_smooth, double sigma,
                                   float max_thickness, float dir, FILE *log_fp,
                                   MRI *mri_wm, int callno);
int MRIScomputeMaxGradBorderValuesPial(MRI_SURFACE *mris, MRI *mri_brain,
                                       MRI *mri_smooth, double sigma,
                                       float max_thickness, float dir,
                                       FILE *log_fp, int callno, MRI *mri_mask);
