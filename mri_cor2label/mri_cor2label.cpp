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
/*----------------------------------------------------------
  Name: mri_cor2label.c
  $Id: mri_cor2label.c,v 1.12 2011/03/02 00:04:14 nicks Exp $
  Author: Douglas Greve
  Purpose: Converts values in any volume file (not just cor)
  to a label.
  -----------------------------------------------------------*/

/*
BEGINUSAGE --------------------------------------------------------------

mri_cor2label
   --i  input     : vol or surface overlay
   --id labelid   : value to match in the input
   --l  labelfile : name of output file
   --v  volfile   : write label volume in file
   --surf subject hemi <surf> : interpret input as surface overlay
   --opt target delta valmap
   --remove-holes-islands  : remove holes in label and islands (with --surf only)
   --dilate ndilations : dilate label (with --surf only)
   --erode  nerosions  : erode label (with --surf only)
     Note: dilation is done first, then erode
   --help         :print out help information

ENDUSAGE --------------------------------------------------------------
*/

/*
BEGINHELP --------------------------------------------------------------

Converts values in a volume or surface overlay to a label. The program
searches the input for values equal to labelid. The xyz values for
each point are then computed based on the tkregister voxel-to-RAS
matrix (volume) or from the xyz of the specified surface.  The xyz
values are then stored in labelfile in the label file format. the
statistic value is set to 0.  While this program can be used with any
mri volume, it was designed to convert parcellation volumes, which
happen to be stored in mri format.  See tkmedit for more information
on parcellations. For volumes, the volume of the labvel in mm^3 can be
written to the text file designated in by the --v flag.

If --opt is used, then treats input as a probability map. It
thresholds the probability map such that the sum of the suprathreshold
voxels in the valmap will be closest to target. The application is
where one is trying to create a label where the total area is as
close to the mean of the individual labels as possible, eg, 
   mri_cor2label --i label.prob.mgz --id 1 --l ./lh.junk.label \
     --surf fsaverage lh --opt 922.18 .001 \
    $SUBJECTS_DIR/fsaverage/surf/lh.white.avg.area.mgh
Performs a brute-force seach of thresholds from 0-1 step delta.

Note: the name of this program is a bit misleading as it will operate
on anything readble by mri_convert (eg, mgz, mgh, nifti, bhdr,
analyze, etc).

Bugs:

  If the name of the label does not include a forward slash (ie, '/')
  then the program will attempt to put the label files in
  $SUBJECTS_DIR/subject/label.  So, if you want the labels to go into
  the current directory, make sure to put a './' in front of the label.

Example 1: Create a label of the left putamen (12) from the automatic
segmentation

mri_cor2label --c $SUBJECTS_DIR/subject/mri/aseg.mgz
   --id 12 --l ./left-putamen.label

The value 12 is from $FREESURFER_HOME/FreeSurferColorLUT.txt

Example 2: Create a label of the left hemi surface vertices whose
thickness is greater than 2mm.

# First, create a binarized map of the thickness
mri_binarize --i $SUBJECTS_DIR/subject/surf/lh.thickness
  --min 2 --o lh.thickness.thresh.mgh

mri_cor2label --i lh.thickness.thresh.mgh
  --surf subject lh  --id 1 --l ./lh.thickness.thresh.label


ENDHELP --------------------------------------------------------------
*/

#include "cmdargs.h"
#include "diag.h"
#include "mrisurf.h"
#include "version.h"

static int  parse_commandline(int argc, char **argv);
static void check_options();
static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();
static void argnerr(char *option, int n);

static char vcid[] =
    "$Id: mri_cor2label.c,v 1.12 2011/03/02 00:04:14 nicks Exp $";
const char *Progname;
int         main(int argc, char *argv[]);

char * infile;
char * labelfile;
char * volfile;
int    labelid;
int    DoStat;
char * hemi, *SUBJECTS_DIR;
char * surfname = "white";
MRI *  mri;
LABEL *lb;

int    xi, yi, zi, c, nlabel;
float  x, y, z;
char * subject_name = nullptr;
int    doit;
int    synthlabel = 0;
int    verbose    = 0;
float  xsum, ysum, zsum;
char   tmpstr[2000];
MRIS * surf = NULL;
double optTargetSum;  //eg, target area of label
double optDelta;      //eg, step size in brute force search
char * optValMapFile; // eg lh.area
int    DoOptThresh = 0;
double GetOptThresh(double targetSumVal, MRI *valmap, MRI *pmap, double delta);
int    label_erode           = 0; // requires surface
int    label_dilate          = 0; // requires surface
int    RemoveHolesAndIslands = 0; // requires surface

/*----------------------------------------------------*/
int main(int argc, char **argv) {
  FILE *  fp;
  int     nargs, nv, nth;
  MATRIX *vox2rastkr = NULL, *crs = NULL, *xyz = NULL;
  double  voxval;

  nargs = handleVersionOption(argc, argv, "mri_cor2label");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;

  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  if (argc == 0)
    usage_exit();

  infile    = nullptr;
  labelfile = nullptr;
  labelid   = -1;

  parse_commandline(argc, argv);
  check_options();

  printf("%s\n", vcid);

  fprintf(stderr, "Loading mri %s\n", infile);
  mri = MRIread(infile);
  nv  = mri->width * mri->height * mri->depth;

  if (DoOptThresh) {
    printf("Computing optimal threshold\n");
    MRI *valmap = MRIread(optValMapFile);
    if (valmap == NULL)
      exit(1);
    double OptThresh = GetOptThresh(optTargetSum, valmap, mri, optDelta);
    printf("Binarizing at %g\n", OptThresh);
    MRI *tmp = MRIbinarize(mri, NULL, OptThresh, 0, 1);
    MRIfree(&mri);
    mri = tmp;
    MRIfree(&valmap);
  }

  if (hemi == NULL) {
    vox2rastkr = MRIxfmCRS2XYZtkreg(mri);
    printf("------- Vox2RAS of input volume -----------\n");
    MatrixPrint(stdout, vox2rastkr);
    crs             = MatrixAlloc(4, 1, MATRIX_REAL);
    crs->rptr[4][1] = 1;
    xyz             = MatrixAlloc(4, 1, MATRIX_REAL);
  }

  if (hemi != nullptr) {
    SUBJECTS_DIR = getenv("SUBJECTS_DIR");
    sprintf(tmpstr, "%s/%s/surf/%s.%s", SUBJECTS_DIR, subject_name, hemi,
            surfname);
    printf("Loading %s\n", tmpstr);
    surf = MRISread(tmpstr);
    if (surf == nullptr)
      exit(1);
    if (nv != surf->nvertices) {
      printf("ERROR: dim mismatch between surface (%d) and input (%d)\n",
             surf->nvertices, nv);
      exit(1);
    }
  }

  /* allocate a label as big as the mri itself */
  lb = LabelAlloc(nv, subject_name, labelfile);

  fprintf(stderr, "Scanning the volume\n");
  nlabel = 0;
  xsum   = 0.0;
  ysum   = 0.0;
  zsum   = 0.0;
  nth    = -1;
  for (zi = 0; zi < mri->depth; zi++) {
    for (yi = 0; yi < mri->height; yi++) {
      for (xi = 0; xi < mri->width; xi++) {
        nth++;

        voxval = MRIgetVoxVal(mri, xi, yi, zi, 0);

        if (DoStat == 0) {
          c = (int)voxval;
          if (c != labelid)
            continue;
        } else {
          if (voxval == 0)
            continue;
          lb->lv[nlabel].stat = voxval;
        }

        if (surf == NULL) {
          crs->rptr[1][1]    = xi;
          crs->rptr[2][1]    = yi;
          crs->rptr[3][1]    = zi;
          crs->rptr[4][1]    = 1;
          xyz                = MatrixMultiply(vox2rastkr, crs, xyz);
          x                  = xyz->rptr[1][1];
          y                  = xyz->rptr[2][1];
          z                  = xyz->rptr[3][1];
          lb->lv[nlabel].vno = -1;
        } else {
          x                  = surf->vertices[nth].x;
          y                  = surf->vertices[nth].y;
          z                  = surf->vertices[nth].z;
          lb->lv[nlabel].vno = nth;
        }
        if (verbose)
          printf("%5d   %3d %3d %3d   %6.2f %6.2f %6.2f \n", nlabel, xi, yi, zi,
                 x, y, z);

        lb->lv[nlabel].x = x;
        lb->lv[nlabel].y = y;
        lb->lv[nlabel].z = z;
        nlabel++;
        xsum += x;
        ysum += y;
        zsum += z;
      }
    }
  }
  lb->n_points = nlabel;
  printf("Found %d label voxels\n", nlabel);

  if (volfile != nullptr) {
    printf("Writing volume to  %s\n", volfile);
    fp = fopen(volfile, "w");
    if (fp == nullptr) {
      printf("ERROR: could not open  %s\n", volfile);
      exit(1);
    }
    fprintf(fp, "%f\n", nlabel * mri->xsize * mri->ysize * mri->zsize);
    fclose(fp);
  }

  if (labelfile == nullptr)
    exit(0);

  if (nlabel == 0) {
    printf("ERROR: found no voxels matching id %d \n", labelid);
    exit(1);
  }

  if (RemoveHolesAndIslands) {
    printf("Removing holes and islands\n");
    LABEL *tmplabel = LabelRemoveHolesAndIslandsSurf(surf, lb);
    LabelFree(&lb);
    lb = tmplabel;
  }

  if (label_dilate && surf) {
    printf("Dilating label %d times\n", label_dilate);
    LabelDilate(lb, surf, label_dilate, CURRENT_VERTICES);
  }
  if (label_erode) {
    printf("Eroding label %d times\n", label_erode);
    LabelErode(lb, surf, label_erode);
  }

  printf("Writing label file %s\n", labelfile);
  LabelWrite(lb, labelfile);

  fprintf(stderr, "Centroid: %6.2f  %6.2f  %6.2f \n", xsum / nlabel,
          ysum / nlabel, zsum / nlabel);

  fprintf(stderr, "mri_cor2label completed SUCCESSFULLY\n");

  return (0);
  exit(0);
}
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static int parse_commandline(int argc, char **argv) {
  int    nargc = argc, nargs = 0;
  char **pargv = argv, *option;

  while (nargc > 0) {

    option = pargv[0];
    if (!stricmp(option, "--help"))
      print_help();
    else if (!stricmp(option, "--version"))
      print_version();

    /* ---- mri directory ------------ */
    else if (!strcmp(option, "--c") || !strcmp(option, "--i")) {
      if (nargc < 2)
        argnerr(option, 1);
      infile = pargv[1];
      nargs  = 2;
    }

    /* ---- label file ---------- */
    else if (!strcmp(option, "--l")) {
      if (nargc < 2)
        argnerr(option, 1);
      labelfile = pargv[1];
      nargs     = 2;
    }

    /* ---- count file ---------- */
    else if (!strcmp(option, "--v")) {
      if (nargc < 2)
        argnerr(option, 1);
      volfile = pargv[1];
      nargs   = 2;
    }

    /* ---- label id ---------- */
    else if (!strcmp(option, "--id")) {
      if (nargc < 2)
        argnerr(option, 1);
      sscanf(pargv[1], "%d", &labelid);
      nargs = 2;
    }
    /* ---- copy voxel value to stat field ---------- */
    else if (!strcmp(option, "--stat")) {
      DoStat = 1;
      nargs  = 1;
    }

    /* ---- label id ---------- */
    else if (!strcmp(option, "--surf")) {
      if (nargc < 2)
        argnerr(option, 1);
      subject_name = pargv[1];
      hemi         = pargv[2];
      nargs        = 3;
      if (nargc > 3 && !CMDisFlag(pargv[3])) {
        surfname = pargv[3];
        nargs++;
      }
    }

    /* ---- label id ---------- */
    else if (!strcmp(option, "--sd")) {
      if (nargc < 2)
        argnerr(option, 1);
      setenv("SUBJECTS_DIR", pargv[1], 1);
      nargs = 2;
    }

    /* ---- opt-thresholded label ---------- */
    else if (!strcmp(option, "--opt")) {
      if (nargc < 4)
        argnerr(option, 3); // target delta valmap
      // Will interpet the input as a p-map
      DoOptThresh = 1;
      sscanf(pargv[1], "%lf", &optTargetSum); //eg, target area of label
      sscanf(pargv[2], "%lf", &optDelta);     //eg, target area of label
      optValMapFile = pargv[3];               // eg, lh.area
      labelid       = 1;
      nargs         = 4;
    } else if (!strcmp(option, "--dilate")) {
      if (nargc < 2)
        argnerr(option, 1);
      sscanf(pargv[1], "%d", &label_dilate);
      nargs = 2;
    } else if (!strcmp(option, "--erode")) {
      if (nargc < 2)
        argnerr(option, 1);
      sscanf(pargv[1], "%d", &label_erode);
      nargs = 2;
    } else if (!strcmp(option, "--remove-holes-islands")) {
      RemoveHolesAndIslands = 1; // requires surface
      nargs                 = 1;
    }
    /* ---- synthesize the label ---------- */
    else if (!strcmp(option, "--synthlabel")) {
      synthlabel = 1;
      nargs      = 1;
    }

    /* ---- verbose ---------- */
    else if (!strcmp(option, "--verbose")) {
      verbose = 1;
      nargs   = 1;
    } else {
      fprintf(stderr, "ERROR: Option %s unknown\n", option);
      exit(-1);
    }
    nargc -= nargs;
    pargv += nargs;
  }
  return (0);
}
/* ------------------------------------------------------ */
static void usage_exit() {
  print_usage();
  exit(1);
}
/* --------------------------------------------- */
static void print_usage(void) {
  printf("\n");
  printf("mri_cor2label \n");
  printf("   --i  input     : vol or surface overlay\n");
  printf("   --id labelid   : value to match in the input\n");
  printf("   --l  labelfile : name of output file\n");
  printf("   --v  volfile   : write label volume in file\n");
  printf(
      "   --surf subject hemi <surf> : interpret input as surface overlay\n");
  printf("   --opt target delta valmap\n");
  printf("   --remove-holes-islands  : remove holes in label and islands (with "
         "--surf only)\n");
  printf("   --dilate ndilations : dilate label (with --surf only)\n");
  printf("   --erode  nerosions  : erode label (with --surf only)\n");
  printf("     Note: dilation is done first, then erode\n");
  printf("   --help         :print out help information\n");
  printf("\n");
}
/* --------------------------------------------- */
static void print_help(void) {
  print_usage();
  printf("\n%s\n\n", vcid);
  printf("\n");
  printf("Converts values in a volume or surface overlay to a label. The "
         "program\n");
  printf(
      "searches the input for values equal to labelid. The xyz values for\n");
  printf("each point are then computed based on the tkregister voxel-to-RAS\n");
  printf(
      "matrix (volume) or from the xyz of the specified surface.  The xyz\n");
  printf("values are then stored in labelfile in the label file format. the\n");
  printf("statistic value is set to 0.  While this program can be used with "
         "any\n");
  printf(
      "mri volume, it was designed to convert parcellation volumes, which\n");
  printf(
      "happen to be stored in mri format.  See tkmedit for more information\n");
  printf("on parcellations. For volumes, the volume of the labvel in mm^3 can "
         "be\n");
  printf("written to the text file designated in by the --v flag. \n");
  printf("\n");
  printf("If --opt is used, then treats input as a probability map. It\n");
  printf("thresholds the probability map such that the sum of the "
         "suprathreshold\n");
  printf(
      "voxels in the valmap will be closest to target. The application is\n");
  printf("where one is trying to create a label where the total area is as\n");
  printf("close to the mean of the individual labels as possible, eg, \n");
  printf("   mri_cor2label --i label.prob.mgz --id 1 --l ./lh.junk.label \\n");
  printf("     --surf fsaverage lh --opt 922.18 .001 \\n");
  printf("    $SUBJECTS_DIR/fsaverage/surf/lh.white.avg.area.mgh\n");
  printf("Performs a brute-force seach of thresholds from 0-1 step delta.\n");
  printf("\n");
  printf("Note: the name of this program is a bit misleading as it will "
         "operate\n");
  printf("on anything readble by mri_convert (eg, mgz, mgh, nifti, bhdr,\n");
  printf("analyze, etc).\n");
  printf("\n");
  printf("Bugs:\n");
  printf("\n");
  printf("  If the name of the label does not include a forward slash (ie, "
         "'/')\n");
  printf("  then the program will attempt to put the label files in\n");
  printf("  $SUBJECTS_DIR/subject/label.  So, if you want the labels to go "
         "into\n");
  printf("  the current directory, make sure to put a './' in front of the "
         "label.\n");
  printf("\n");
  printf("Example 1: Create a label of the left putamen (12) from the "
         "automatic\n");
  printf("segmentation\n");
  printf("\n");
  printf("mri_cor2label --c $SUBJECTS_DIR/subject/mri/aseg.mgz \n");
  printf("   --id 12 --l ./left-putamen.label\n");
  printf("\n");
  printf("The value 12 is from $FREESURFER_HOME/FreeSurferColorLUT.txt\n");
  printf("\n");
  printf("Example 2: Create a label of the left hemi surface vertices whose\n");
  printf("thickness is greater than 2mm.\n");
  printf("\n");
  printf("# First, create a binarized map of the thickness\n");
  printf("mri_binarize --i $SUBJECTS_DIR/subject/surf/lh.thickness \n");
  printf("  --min 2 --o lh.thickness.thresh.mgh\n");
  printf("\n");
  printf("mri_cor2label --i lh.thickness.thresh.mgh \n");
  printf("  --surf subject lh  --id 1 --l ./lh.thickness.thresh.label\n");
  printf("\n");
  printf("\n");
  exit(1);
}
/* --------------------------------------------- */
static void print_version() {
  fprintf(stderr, "%s\n", vcid);
  exit(1);
}
/* --------------------------------------------- */
static void argnerr(char *option, int n) {
  if (n == 1)
    fprintf(stderr, "ERROR: %s flag needs %d argument\n", option, n);
  else
    fprintf(stderr, "ERROR: %s flag needs %d arguments\n", option, n);
  exit(-1);
}
/* --------------------------------------------- */
static void check_options() {
  if (infile == nullptr) {
    fprintf(stderr, "ERROR: must supply an input\n");
    exit(1);
  }
  if (labelfile == nullptr && volfile == nullptr) {
    fprintf(stderr, "ERROR: must be supply a label or volume file\n");
    exit(1);
  }
  if (labelid == -1 && DoStat == 0) {
    fprintf(stderr, "ERROR: must supply a label id or --stat\n");
    exit(1);
  }
  if (labelid != -1 && DoStat != 0) {
    fprintf(stderr, "ERROR: cannot supply both label id and --stat\n");
    exit(1);
  }
  if (label_dilate && subject_name == NULL) {
    printf("ERROR: --dilate requires a surface\n");
    exit(1);
  }
  if (label_erode && subject_name == NULL) {
    printf("ERROR: --erode requires a surface\n");
    exit(1);
  }
  if (RemoveHolesAndIslands && subject_name == NULL) {
    printf("ERROR: --remove-holes-islands requires a surface\n");
    exit(1);
  }

  return;
}

/*!
  \fn double GetOptThresh(double targetSumVal, MRI *valmap, MRI *pmap, double delta)
  \brief Determines a threshold for pmap where the sum of valmap over the suprathreshold
  voxels in pmap is closest to targetSumVal. The application here is when trying to 
  threshold a probablistic label map so that the total area of the label is the same
  as the average areas of the individual. Performs a brute-force search for threshold
  from 0-1 step delta.
  \param targetSumVal - target value (eg, average area of the label)
  \param valmap - map to get the values to sum (eg, lh.white.avg.area.mgh)
  \param pmap - probability of seeing a label at a given vertex
  \param delta - step size when performing brute force search.
 */
double GetOptThresh(double targetSumVal, MRI *valmap, MRI *pmap, double delta) {
  int    c, r, s, nhits;
  double thresh, p, val, SumVal, optThresh, err, errmin, optSumVal;

  errmin    = 10e10;
  optThresh = 0;
  optSumVal = 0;
  for (thresh = delta; thresh < 1; thresh += delta) {
    SumVal = 0;
    nhits  = 0;
    for (c = 0; c < pmap->width; c++) {
      for (r = 0; r < pmap->height; r++) {
        for (s = 0; s < pmap->depth; s++) {
          p = MRIgetVoxVal(pmap, c, r, s, 0);
          if (p < thresh)
            continue;
          nhits++;
          val = MRIgetVoxVal(valmap, c, r, s, 0);
          SumVal += val;
        }
      }
    }
    err = fabs(SumVal - targetSumVal);
    //printf("%6.4f %8.4f %8.4f %8.4f\n",thresh,SumVal,targetSumVal,err);
    if (errmin > err) {
      errmin    = err;
      optThresh = thresh;
      optSumVal = SumVal;
    }
    if (nhits == 0)
      break;
  }
  printf(
      "GetOptThresh: optThresh=%g, optSumVal=%g, targetSumVal=%g, errmin=%g\n",
      optThresh, optSumVal, targetSumVal, errmin);

  return (optThresh);
}
