/**
 * @brief include file for ROI utilies.
 *
 * structures, macros, constants and prototypes for the
 * manipulation, creation and
 * I/O for labels (lists of vertices and/or voxels)
 */
/*
 * Original Author: Bruce Fischl
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

#ifndef LABEL_H
#define LABEL_H

#include "minc.h"

#include "const.h"
#include "matrix.h"
#include "mri.h"

#include "mrisurf_aaa.h" // incomplete MRIS, MHT
#include "transform.h"

struct LABEL_VERTEX {
  int   vno;
  float x;
  float y;
  float z;
  int   xv; // voxel coords into label->mri struct (may not be initialzed if
            // label->mri == NULL)
  int           yv;
  int           zv;
  unsigned char deleted;
  float         stat; /* statistic (might not be used) */
};

struct LABEL {
  int               max_points;   /* # of points allocated */
  int               coords;       // one of the LABEL_COORDS* constants below
  int               n_points;     /* # of points in area */
  char              name[STRLEN]; /* name of label file */
  char              subject_name[STRLEN]; /* name of subject */
  LV *              lv;                   /* labeled vertices */
  General_transform transform; /* the next two are from this struct */
  Transform *       linear_transform;
  Transform *       inverse_linear_transform;
  char              space[100]; /* space description of the coords */
  double            avg_stat;
  int *vertex_label_ind; // mris->nvertices long - < 0 means it isn't in the
                         // label
  MRI * mri_template;
  MHT * mht;
  MRIS *mris;
};

LABEL *LabelfromASeg(MRI *aseg, int segcode);
int    LabelFillVolume(MRI *mri, LABEL *label, int fillval);

MATRIX *LabelFitXYZ(LABEL *label, int order);
LABEL * LabelApplyMatrix(LABEL *lsrc, MATRIX *m, LABEL *ldst);
LABEL * LabelBoundary(LABEL *label, MRIS *surf);
int     VertexIsInLabel(int vtxno, LABEL *label);
LABEL * LabelInFOV(MRI_SURFACE *mris, MRI *mri, float pad);
int     LabelUnassign(LABEL *area);
LABEL * MRISlabelInvert(MRIS *surf, LABEL *label);
int     LabelMaskSurface(LABEL *label, MRI_SURFACE *mris);
int     LabelMaskSurfaceValues(LABEL *label, MRI_SURFACE *mris);
int     LabelMaskSurfaceCurvature(LABEL *label, MRI_SURFACE *mris);
int     LabelMaskSurfaceVolume(LABEL *label, MRI *mri, float nonmask_val);

LABEL *LabelSphericalCombine(MRI_SURFACE *mris, LABEL *area,
                             MRIS_HASH_TABLE *mht, MRI_SURFACE *mris_dst,
                             LABEL *area_dst);

LABEL *LabelClone(LABEL *a);
int    LabelCropPosterior(LABEL *area, float anterior_dist);
int    LabelCropAnterior(LABEL *area, float anterior_dist);
int    LabelCentroid(LABEL *area, MRI_SURFACE *mris, double *px, double *py,
                     double *pz, int *pvno);
int    LabelSetVals(MRI_SURFACE *mris, LABEL *area, float fillval);
int    LabelAddToMark(LABEL *area, MRI_SURFACE *mris, int val_to_add);
LABEL *LabelTransform(LABEL *area_in, TRANSFORM *xform, MRI *mri,
                      LABEL *area_out);
LABEL *LabelFromScannerRAS(LABEL *lsrc, MRI *mri, LABEL *ldst);
LABEL *LabelBaryFill(MRIS *mris, LABEL *srclabel, double delta);

LABEL *LabelSampleToSurface(MRI_SURFACE *mris, LABEL *area, MRI *mri_template,
                            int coords);
int    LabelInit(LABEL *lsrc, MRI *mri_template, MRI_SURFACE *mris, int coords);
int    LabelAddVoxel(LABEL *area, int xv, int yv, int zv, int coords,
                     int *vertices, int *pnvertices);
int    LabelDeleteVoxel(LABEL *area, int xv, int yv, int zv, int *vertices,
                        int *pnvertices);
LABEL *LabelAddPoint(LABEL *label, LV *lv);
int    LabelAddVertex(LABEL *area, int vno, int coords);
int    LabelDeleteVertex(LABEL *area, int vno, int coords);
double LabelAverageVal(LABEL *area, MRI_SURFACE *mris);
LABEL *LabelFromSurface(MRI_SURFACE *mris, int which, double thresh);
LABEL *LabelRemoveIslandsSurf(MRIS *surf, LABEL *lb);
LABEL *LabelRemoveHolesSurf(MRIS *surf, LABEL *lb);
LABEL *LabelRemoveHolesAndIslandsSurf(MRIS *surf, LABEL *lb);

#endif
