#pragma once
/*
 * @file utilities common to mrisurf*.c but not used outside them
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

#include "mrisurf_metricProperties.h"

#define SAMPLE_DIST 0.25

#define MAX_CSF   55.0f
#define STEP_SIZE 0.1

int mrisClearMomentum(MRI_SURFACE *mris);
int mrisClearGradient(MRI_SURFACE *mris);
int mrisClearExtraGradient(MRI_SURFACE *mris);

void mrisSetVal(MRIS *mris, float val);
void mrisSetValAndClearVal2(MRIS *mris, float val);

void MRISsetCurvaturesToValues(MRIS *mris, int fno);     //  val = curv
void MRISsetCurvaturesToOrigValues(MRIS *mris, int fno); //  etc.
void MRISsetOrigValuesToCurvatures(MRIS *mris, int fno);
void MRISsetOrigValuesToValues(MRIS *mris, int fno);
void MRISsetValuesToCurvatures(MRIS *mris, int fno);

int detectContrast(MRIS *mris);

int mrisUpdateTargetLocations(MRI_SURFACE *mris, MRI *mri,
                              double target_intensity);

void MRISclearWhichAndVal2(MRIS *mris, int which);
