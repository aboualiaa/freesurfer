/**
 * @file  mris_find_flat_regions.c
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:32 $
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

#include "diag.h"
#include "mrisurf.h"
#include "version.h"
#include "mri_identify.h"

static char vcid[] =
    "$Id: mris_find_flat_regions.c,v 1.4 2011/03/02 00:04:32 nicks Exp $";

int main(int argc, char *argv[]);

static int get_option(int argc, char *argv[]);
static void print_usage();
static void print_help();
static void print_version();

const char *Progname;

static float thresh = 0.99;

int main(int argc, char *argv[]) {
  char **av, fname[STRLEN], *surf_name, *wfile_name;
  int ac, nargs, vno;
  MRI_SURFACE *mris;
  VERTEX *v;

  /* rkt: check for and handle version tag */
  nargs = handle_version_option(
      argc, argv,
      "$Id: mris_find_flat_regions.c,v 1.4 2011/03/02 00:04:32 nicks Exp $",
      "$Name:  $");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;

  Progname = argv[0];
  ErrorInit(NULL, NULL, NULL);
  DiagInit(NULL, NULL, NULL);

  ac = argc;
  av = argv;
  for (; argc > 1 && ISOPTION(*argv[1]); argc--, argv++) {
    nargs = get_option(argc, argv);
    argc -= nargs;
    argv += nargs;
  }

  if (argc < 2)
    print_help();

  surf_name = argv[1];
  wfile_name = argv[2];

  mris = MRISread(surf_name);
  if (!mris)
    ErrorExit(ERROR_NOFILE, "%s: could not read surface file %s", Progname,
              fname);

  MRIScomputeMetricProperties(mris);

  for (vno = 0; vno < mris->nvertices; vno++) {
    v = &mris->vertices[vno];
    if (v->ripflag)
      continue;
    if ((fabs(v->nx) > thresh) || (fabs(v->ny) > thresh) ||
        (fabs(v->nz) > thresh))
      v->val = 1;
  }

  if (mri_identify(wfile_name) == MGH_LABEL_FILE) {
    LABEL *area;
    area = LabelFromSurface(mris, VERTEX_VALS, .9);
    LabelWrite(area, wfile_name);
  } else
    MRISwriteValues(mris, wfile_name);

  exit(0);
  return (0); /* for ansi */
}

/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int get_option(int argc, char *argv[]) {
  int nargs = 0;
  char *option;

  option = argv[1] + 1; /* past '-' */
  if (!stricmp(option, "-help"))
    print_help();
  else if (!stricmp(option, "-version"))
    print_version();
  else
    switch (toupper(*option)) {
    case '?':
    case 'U':
      print_usage();
      exit(1);
      break;
    case 'T':
      thresh = atof(argv[2]);
      nargs = 1;
      fprintf(stderr, "using threshold = %2.3f\n", thresh);
      break;
    default:
      fprintf(stderr, "unknown option %s\n", argv[1]);
      exit(1);
      break;
    }

  return (nargs);
}

static void print_usage() {
  fprintf(stderr, "usage: %s [options] <surface> <wfile>\n", Progname);
}

static void print_help() {
  print_usage();
  fprintf(stderr,
          "\nThis program computed regions in  which the surface is almost "
          "perpindicular to one\n"
          "of  the cardinal axes, and writes  the results  to  a label file\n");
  fprintf(stderr,
          "-t <thresh>  "
          "specify the threshold to use  (default=%2.3f)\n",
          thresh);
  exit(1);
}

static void print_version() {
  fprintf(stderr, "%s\n", vcid);
  exit(1);
}
