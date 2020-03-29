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

#include "mrisurf_deform.h"

#define KEEP_VERTEX    0
#define DISCARD_VERTEX 1
#define FINAL_VERTEX   2

#define ET_OVERLAP_LIST_INCOMPLETE 0x0001

typedef struct {
  int            nedges;
  EDGE *         edges;
  int **         overlapping_edges; /* indices of all edges
                              overlapping this one (i.e. [i][j]) */
  int *          noverlap;
  unsigned char *flags;
  int            use_overlap;
} EDGE_TABLE;

typedef struct {
  double      fitness;
  int         nedges;
  int *       ordering; /* order of edges in EDGE_TABLE
                    for retessellation */
  DEFECT *    defect;
  EDGE_TABLE *etable;
  int         rank;

  TP  tp; /* list of vertices,edges,faces
            used in the tessellated defect */
  int retessellation_mode;

  MRI *mri;
  MRI *mri_defect; /* volume constituted by
                      the potential edges - just a pointer */
  MRI *mri_defect_white;
  MRI *mri_defect_gray;
  MRI *mri_defect_sign;
  MRI *mri_defect_initial_sign;
  MRI *mri_defect_wm;

  int verbose_mode; /* verbose mode for debugging */

} DEFECT_PATCH, DP;

typedef struct {
  int vno; // vertex # in surface

  // copied from VERTEX (malloc used for pointers)
  unsigned char nsizeMax, nsizeCur;
  int           vnum, v2num, v3num;
  int           vtotal;
  int *         v;
  float         origx, origy, origz;
  float         nx, ny, nz;
  int *         f;
  uchar *       n;
  uchar         num;

  // A hash that can be checked after restore to test for other changes
  //
  MRIS_HASH hash;
} VERTEX_STATE, VS;

typedef struct {
  DEFECT *      defect;
  VERTEX_STATE *vs;
  int           nvertices;
  int *         vertex_trans; /* not allocated - pointer to preexisting table */
  int           nfaces;       /* number of used faces before retessellation */
} DEFECT_VERTEX_STATE, DVS;

/* this structure is used in the VERTEX structure (*vp) */
typedef struct {
  int  nedges;
  int *edges;
} EDGE_POINTER, EP;

typedef struct {
  /* edges constituting the cluster */
  int *edges;
  int  nedges;
  int  max_edges;
  /* edge intersecting the edges of the cluster */
  int *xedges;
  int  nxedges;
  int  max_xedges;
} SEGMENT;

// for the clustering of overlapping edges
typedef struct {
  int n;          /* position of this edge in the table */
  int vno1, vno2; /* vertices in the ORIGINAL tessellation */
  int segment;    /* cluster number */

  int *xedges; /* intersecting edges */
  int  nxedges;
} EDGE_STRUCTURE, ES;

typedef struct {
  SEGMENT *segments;
  int      nsegments;
  int      max_segments;
  MRIS *   mris;
  /* the structure containing the edges */
  ES *edges;
  int nedges;
} SEGMENTATION;

typedef struct {
  /* save the best configuration */
  int * best_ordering;
  float best_fitness;
  char *status; // kept or discarded

  /* keep track of the result for the past iterations */
  int *  nused;
  float *vertex_fitness;
} RANDOM_PATCH, RP;

typedef struct {
  float c_x, c_y, c_z;    /* canonical coordinates */
  float oc_x, oc_y, oc_z; /* orig coordinates in optimized configuations */
  float status;           /* DISCARDED VERTEX OR KEPT VERTEX */
} FS_VERTEX_INFO;

typedef struct {
  int             nvertices;
  int             ninside; /* index of the first inside vertex */
  FS_VERTEX_INFO *vertices;
  float           fitness; /* fitness associated with this patch */
  int             status;  /* valid patch or not */
} MAPPING;

/* structure which encodes the information for the optimal mapping */
typedef struct {
  MRIS *mris; /* structure encoding the original mapping */

  int *vertex_trans;
  int *face_trans;

  int     nmappings; /* number of generated mappings */
  MAPPING orig_mapping;
  MAPPING mappings[10]; /* mapping (mapping 0 corresponds
                           to the initial mapping) */
} OPTIMAL_DEFECT_MAPPING;

double mrisComputeDefectLogLikelihood(
    ComputeDefectContext *computeDefectContext, MRI_SURFACE *mris, MRI *mri,
    DEFECT_PATCH *dp, HISTOGRAM *h_k1, HISTOGRAM *h_k2, MRI *mri_k1_k2,
    HISTOGRAM *h_white, HISTOGRAM *h_gray, HISTOGRAM *h_border,
    HISTOGRAM *h_grad, MRI *mri_gray_white, HISTOGRAM *h_dot,
    TOPOLOGY_PARMS *parms);

double mrisComputeDefectNormalDotLogLikelihood(MRI_SURFACE *mris, TP *tp,
                                               HISTOGRAM *h_dot);
double mrisComputeDefectCurvatureLogLikelihood(MRI_SURFACE *mris, TP *tp,
                                               HISTOGRAM *h_k1, HISTOGRAM *h_k2,
                                               MRI *mri_k1_k2);
double
mrisComputeDefectMRILogUnlikelihood(ComputeDefectContext *computeDefectContext,
                                    MRI_SURFACE *mris, DP *dp,
                                    HISTOGRAM *h_border);
