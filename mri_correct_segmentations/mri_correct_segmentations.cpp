/**
 * @brief 
 *
 */

#include "mri2.h"
#include "version.h"

#include "fastmarching.h"

static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();

MRI *correct_gmwm_boundaries(MRI *segmri, MRI *outmri);
MRI *correct_gmwm_boundaries_2(MRI *segmri, MRI *outmri);
MRI *correct_putamen_pallidum_boundaries(MRI *segmri, MRI *outmri);
MRI *correct_largestCC_and_fill_holes(MRI *segmri, MRI *outmri);
MRI *fill_leftover_voxels(MRI *segmri, MRI *inmri, MRI *outmri);

static int get_option(int argc, char *argv[]);

const char *Progname;
// int use_orig_value = 0;
int noGMWM = 0;

/***-------------------------------------------------------****/
int main(int argc, char *argv[]) {
  int    nargs, ac, nvolumes;
  char **av;
  MRI *  outmri0 = nullptr, *outmri1 = nullptr, *outmri2 = nullptr,
      *outmri3 = nullptr, *outmri4 = nullptr, *segmri;

  nargs = handleVersionOption(argc, argv, "mri_correct_segmentations");
  if (nargs && argc - nargs == 1)
    exit(0);
  Progname = argv[0];
  argc -= nargs;
  ac = argc;
  av = argv;
  for (; argc > 1 && ISOPTION(*argv[1]); argc--, argv++) {
    nargs = get_option(argc, argv);
    argc -= nargs;
    argv += nargs;
  }

  nvolumes = argc - 1;
  printf("processing %d input files\n", nvolumes);
  if (nvolumes != 2)
    usage_exit();
  printf("processing %d input files\n", nvolumes);

  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  char *fname = argv[1];
  printf("processing segmentation input volume %s\n", fname);
  segmri = MRIread(fname);
  // int width  = segmri->width ;
  // int height = segmri->height ;
  // int depth  = segmri->depth ;

  char *outputfname = argv[2];
  printf("output fname %s\n", outputfname);

  // GM/WM
  outmri0 = MRIcopy(segmri, nullptr);
  // MRIwrite(outmri0, "/tmp/segmri.mgz") ;
  correct_gmwm_boundaries(segmri, outmri0);

  // MRIwrite(outmri0, "/tmp/outmri0.mgz") ;
  // putamen / pallidum
  outmri1 = MRIcopy(outmri0, nullptr);
  if (noGMWM == 0)
    correct_putamen_pallidum_boundaries(outmri0, outmri1);
  // MRIwrite(outmri1, "/tmp/outmri1.mgz") ;
  // GM / WM
  outmri2 = MRIcopy(outmri1, nullptr);
  correct_gmwm_boundaries_2(outmri1, outmri2);

  // MRIwrite(outmri2, "/tmp/outmri2.mgz") ;
  // find largest connected components and close holes
  outmri3 = MRIcopy(segmri, nullptr);
  MRIvalueFill(outmri3, 0);
  correct_largestCC_and_fill_holes(outmri2, outmri3);
  // MRIwrite(outmri3, "/tmp/outmri3.mgz") ;
  // fill leftover voxels in original mask
  outmri4 = MRIcopy(outmri3, nullptr);
  fill_leftover_voxels(segmri, outmri3, outmri4);
  // MRIwrite(outmri4, "/tmp/outmri4.mgz") ;

  //
  outmri0 = MRIcopy(outmri4, nullptr);
  correct_gmwm_boundaries(outmri4, outmri0);

  // MRIwrite(outmri0, "/tmp/redone-outmri0.mgz") ;
  outmri1 = MRIcopy(outmri0, nullptr);
  if (noGMWM == 0)
    correct_putamen_pallidum_boundaries(outmri0, outmri1);
  // MRIwrite(outmri1, "/tmp/redone-outmri1.mgz") ;
  outmri2 = MRIcopy(outmri1, nullptr);
  correct_gmwm_boundaries_2(outmri1, outmri2);

  // MRIwrite(outmri2, "/tmp/redone-outmri2.mgz") ;
  //

  printf("writing output to %s\n", outputfname);
  MRIwrite(outmri2, outputfname);

  MRIfree(&segmri);
  MRIfree(&outmri0);
  MRIfree(&outmri1);
  MRIfree(&outmri2);
  MRIfree(&outmri3);
  MRIfree(&outmri4);

  exit(0);

} /* end main() */

/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int get_option(int argc, char *argv[]) {
  int   nargs = 0;
  char *option;

  option = argv[1] + 1; /* past '-' */
  if (!stricmp(option, "-help")) {
    print_help();
  } else
    switch (toupper(*option)) {
    case '?':
    case 'U':
      nargs = 0;
      print_usage();
      exit(1);
      break;
    case 'V':
      print_version();
      break;
    case 'N':
      noGMWM = 1;
      break;
    default:
      fprintf(stderr, "unknown option %s\n", argv[1]);
      exit(1);
      break;
    }

  return (nargs);
}

/* --------------------------------------------- */
static void print_usage() {
  printf("USAGE: %s [options] fname1 fname2 \n", Progname);
  printf("\n");
}

/* --------------------------------------------- */
static void print_help() {
  print_usage();
  printf("\n"
         "Correcting automated infant segmentation\n");
  exit(1);
}

/* --------------------------------------------- */
static void print_version(void) {
  std::cout << getVersion() << std::endl;
  exit(1);
}

/* ------------------------------------------------------ */
static void usage_exit() {
  print_usage();
  exit(1);
}

/* ------------------------------------------------------ */

MRI *correct_gmwm_boundaries(MRI *segmri, MRI *outmri) {
  int    x, y, z, width, height, depth = 0;
  double val;
  MRI *  tmpvol = nullptr;
  MRI *allmaskdist, *label2, *label2distmap, *label3, *label3distmap, *label41,
      *label41distmap, *label42, *label42distmap = nullptr;

  width  = segmri->width;
  height = segmri->height;
  depth  = segmri->depth;

  // all lables
  tmpvol = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val > 0)
          MRIsetVoxVal(tmpvol, x, y, z, 0, 1);
      }
  allmaskdist = MRIalloc(width, height, depth, MRI_FLOAT);
  allmaskdist = MRIextractDistanceMap(tmpvol, allmaskdist, 1, 3, 3, nullptr);

  // label 2
  label2 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 2)
          MRIsetVoxVal(label2, x, y, z, 0, 1);
      }
  label2distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label2distmap =
      MRIextractDistanceMap(label2, label2distmap, 1, 3, 3, nullptr);

  // label 3
  label3 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 3)
          MRIsetVoxVal(label3, x, y, z, 0, 1);
      }
  label3distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label3distmap =
      MRIextractDistanceMap(label3, label3distmap, 1, 3, 3, nullptr);

  // label 41
  label41 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 41)
          MRIsetVoxVal(label41, x, y, z, 0, 1);
      }
  label41distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label41distmap =
      MRIextractDistanceMap(label41, label41distmap, 1, 3, 3, nullptr);

  // label 42
  label42 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 42)
          MRIsetVoxVal(label42, x, y, z, 0, 1);
      }
  label42distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label42distmap =
      MRIextractDistanceMap(label42, label42distmap, 1, 3, 3, nullptr);

  if (!outmri)
    outmri = MRIclone(segmri, nullptr);

  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        if (MRIgetVoxVal(label2, x, y, z, 0) == 1) {
          double tmpval  = MRIgetVoxVal(label2distmap, x, y, z, 0);
          double tmpval2 = MRIgetVoxVal(label3distmap, x, y, z, 0);
          // if (-0.5 <= lhwmvoldist(WM_L_ind(i)) && lhwmvoldist(WM_L_ind(i)) <=
          // 0 && abs(lhwmvoldist(WM_L_ind(i))) <= lhgmvoldist(WM_L_ind(i)) &&
          // lhgmvoldist(WM_L_ind(i)) >= .5 && segvol(WM_L_ind(i)) == 2 &&
          // abs(maskvol(WM_L_ind(i))) <= 0.5)
          if (-0.5 <= tmpval && tmpval <= 0 && fabs(tmpval) <= tmpval2 &&
              tmpval2 >= .5 &&
              fabs(MRIgetVoxVal(allmaskdist, x, y, z, 0)) <= 0.5)
            MRIsetVoxVal(outmri, x, y, z, 0, 3);
        } else if (MRIgetVoxVal(label41, x, y, z, 0) == 1) {
          double tmpval  = MRIgetVoxVal(label41distmap, x, y, z, 0);
          double tmpval2 = MRIgetVoxVal(label42distmap, x, y, z, 0);
          // if (-0.5 <= rhwmvoldist(WM_R_ind(i)) && rhwmvoldist(WM_R_ind(i)) <=
          // 0 && abs(rhwmvoldist(WM_R_ind(i))) <= rhgmvoldist(WM_R_ind(i)) &&
          // rhgmvoldist(WM_R_ind(i)) >= .5 && segvol(WM_R_ind(i)) == 41 &&
          // abs(maskvol(WM_R_ind(i))) <= 0.5)
          if (-0.5 <= tmpval && tmpval <= 0 && fabs(tmpval) <= tmpval2 &&
              tmpval2 >= .5 &&
              fabs(MRIgetVoxVal(allmaskdist, x, y, z, 0)) <= 0.5)
            MRIsetVoxVal(outmri, x, y, z, 0, 42);
        }
      }

  return (outmri);
}

MRI *correct_putamen_pallidum_boundaries(MRI *segmri, MRI *outmri) {
  int    x, y, z, width, height, depth = 0;
  double val;
  MRI *  label42, *label3, *label12, *label12distmap, *label13, *label13distmap,
      *label51, *label51distmap, *label52, *label52distmap = nullptr;

  width  = segmri->width;
  height = segmri->height;
  depth  = segmri->depth;

  // label 42
  label42 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 42)
          MRIsetVoxVal(label42, x, y, z, 0, 1);
      }

  // label 3
  label3 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 3)
          MRIsetVoxVal(label3, x, y, z, 0, 1);
      }

  // label 12
  label12 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 12)
          MRIsetVoxVal(label12, x, y, z, 0, 1);
      }
  label12distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label12distmap =
      MRIextractDistanceMap(label12, label12distmap, 1, 3, 3, nullptr);

  // label 13
  label13 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 13)
          MRIsetVoxVal(label13, x, y, z, 0, 1);
      }
  label13distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label13distmap =
      MRIextractDistanceMap(label13, label13distmap, 1, 3, 3, nullptr);

  // label 51
  label51 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 51)
          MRIsetVoxVal(label51, x, y, z, 0, 1);
      }
  label51distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label51distmap =
      MRIextractDistanceMap(label51, label51distmap, 1, 3, 3, nullptr);

  // label 52
  label52 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 52)
          MRIsetVoxVal(label52, x, y, z, 0, 1);
      }
  label52distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label52distmap =
      MRIextractDistanceMap(label52, label52distmap, 1, 3, 3, nullptr);

  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        if (MRIgetVoxVal(label42, x, y, z, 0) == 1) {
          // if (rhputvoldist(Rcort_ind(i))<=.5 ||  rhpalvoldist(Rcort_ind(i))
          // <= .5)
          if (MRIgetVoxVal(label51distmap, x, y, z, 0) <= .5 ||
              MRIgetVoxVal(label52distmap, x, y, z, 0) <= .5)
            MRIsetVoxVal(outmri, x, y, z, 0, 41);
        }
        if (MRIgetVoxVal(label3, x, y, z, 0) == 1) {
          // if (lhputvoldist(Lcort_ind(i))<=.5 ||  lhpalvoldist(Lcort_ind(i))
          // <= .5)
          if (MRIgetVoxVal(label12distmap, x, y, z, 0) <= .5 ||
              MRIgetVoxVal(label13distmap, x, y, z, 0) <= .5)
            MRIsetVoxVal(outmri, x, y, z, 0, 2);
        }
      }

  return (outmri);
}

MRI *correct_gmwm_boundaries_2(MRI *segmri, MRI *outmri) {
  int    x, y, z, width, height, depth = 0;
  double val;
  MRI *  allvol = nullptr;
  MRI *  allmaskdist, *label2, *label2distmap, *label41,
      *label41distmap = nullptr;

  width  = segmri->width;
  height = segmri->height;
  depth  = segmri->depth;

  // all lables
  allvol = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val > 0)
          MRIsetVoxVal(allvol, x, y, z, 0, 1);
      }
  allmaskdist = MRIalloc(width, height, depth, MRI_FLOAT);
  allmaskdist = MRIextractDistanceMap(allvol, allmaskdist, 1, 3, 3, nullptr);

  // label 2
  label2 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 2)
          MRIsetVoxVal(label2, x, y, z, 0, 1);
      }
  label2distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label2distmap =
      MRIextractDistanceMap(label2, label2distmap, 1, 3, 3, nullptr);

  // label 41
  label41 = MRIclone(segmri, nullptr);
  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        val = MRIgetVoxVal(segmri, x, y, z, 0);
        if (val == 41)
          MRIsetVoxVal(label41, x, y, z, 0, 1);
      }
  label41distmap = MRIalloc(width, height, depth, MRI_FLOAT);
  label41distmap =
      MRIextractDistanceMap(label41, label41distmap, 1, 3, 3, nullptr);

  if (!outmri)
    outmri = MRIclone(segmri, nullptr);

  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        if (MRIgetVoxVal(allvol, x, y, z, 0) == 1) {
          if ((fabs(MRIgetVoxVal(allmaskdist, x, y, z, 0)) <= .5) &&
              (MRIgetVoxVal(label41distmap, x, y, z, 0) <= 0))
            MRIsetVoxVal(outmri, x, y, z, 0, 42);
          else if ((fabs(MRIgetVoxVal(allmaskdist, x, y, z, 0)) <= .5) &&
                   (MRIgetVoxVal(label2distmap, x, y, z, 0) <= 0))
            MRIsetVoxVal(outmri, x, y, z, 0, 3);
        }
      }

  return (outmri);
}

MRI *correct_largestCC_and_fill_holes(MRI *segmri, MRI *outmri) {
  int i, x, y, z, width, height, depth = 0, val;
  width  = segmri->width;
  height = segmri->height;
  depth  = segmri->depth;
  // char fname[STR_LEN];

  if (!outmri)
    printf("no outmri volume exists!\n");
  // outmri = MRIclone(segmri, NULL) ;
  // outmri = MRIalloc(width, height, depth, MRI_INT);

  int *segidlist;
  int  nsegids = 0;
  segidlist    = MRIsegIdListNot0(segmri, &nsegids, 0);

  MRI *currlabelvol = MRIalloc(width, height, depth, MRI_INT);
  int  currlabel    = 0;

  for (i = 0; i < nsegids; i++) {
    currlabel = segidlist[i];

    if (currlabel > 0) {
      // label volume
      for (x = 0; x < width; x++)
        for (y = 0; y < height; y++)
          for (z = 0; z < depth; z++)
            if (MRIgetVoxVal(segmri, x, y, z, 0) == currlabel)
              MRIsetVoxVal(currlabelvol, x, y, z, 0, 1);
            else
              MRIsetVoxVal(currlabelvol, x, y, z, 0, 0);

      // choose largest connected components
      // sprintf(fname, "/tmp/%d.%s", currlabel,
      // "before-largest-conn-component.mgz") ; MRIwrite(currlabelvol, fname) ;
      GetLargestCC6(
          currlabelvol); // Note: how to fill the background???!! -- WM
      // sprintf(fname, "/tmp/%d.%s", currlabel, "largest-conn-component.mgz") ;
      // MRIwrite(currlabelvol, fname) ;
      // remove holes on that component
      RemoveHoles(currlabelvol);
      // sprintf(fname, "/tmp/%d.%s", currlabel, "holes-removed.mgz") ;
      // MRIwrite(currlabelvol, fname) ;

      // fill ouput volume with new labels
      for (x = 0; x < width; x++)
        for (y = 0; y < height; y++)
          for (z = 0; z < depth; z++) {
            val = MRIgetVoxVal(currlabelvol, x, y, z, 0);
            if (val == 1)
              MRIsetVoxVal(outmri, x, y, z, 0, currlabel);
          }
    }
  }

  return (outmri);
}

// segmri used as a mask for all voxels that need to be labeled in the final
// output
MRI *fill_leftover_voxels(MRI *segmri, MRI *inmri, MRI *outmri) {
  int x, y, z, i, ncount;
  int val = 0;

  int width  = inmri->width;
  int height = inmri->height;
  int depth  = inmri->depth;

  int  nsegid    = 0;
  int *segidlist = MRIsegIdListNot0(inmri, &nsegid, 0);

  for (z = 0; z < depth; z++)
    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        if (MRIgetVoxVal(segmri, x, y, z, 0) &&
            !MRIgetVoxVal(inmri, x, y, z, 0)) {
          // find all neighboring labels and assign based upon majority
          int nmax = 0;
          val      = 0;
          for (i = 0; i < nsegid; i++) {
            ncount = MRIlabelsInNbhd6(inmri, x, y, z, segidlist[i]);
            if (nmax < ncount) {
              nmax = ncount;
              val  = segidlist[i];
            }
          }
          MRIsetVoxVal(outmri, x, y, z, 0, val);
        }
      }

  return outmri;
}
