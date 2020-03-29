/**
 * @file  mris_rescale.c
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:33 $
 *    $Revision: 1.5 $
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

static char vcid[] =
    "$Id: mris_rescale.c,v 1.5 2011/03/02 00:04:33 nicks Exp $";

int main(int argc, char *argv[]);

static int  get_option(int argc, char *argv[]);
static void usage_exit();
static void print_usage();
static void print_help();
static void print_version();

const char *Progname;

int main(int argc, char *argv[]) {
  char **      av, *in_fname, *out_fname;
  int          ac, nargs;
  MRI_SURFACE *mris;
  float        radius, scale;

  nargs = handleVersionOption(argc, argv, "mris_rescale");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;

  Progname = argv[0];
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  ac = argc;
  av = argv;
  for (; argc > 1 && ISOPTION(*argv[1]); argc--, argv++) {
    nargs = get_option(argc, argv);
    argc -= nargs;
    argv += nargs;
  }

  if (argc < 3)
    usage_exit();

  in_fname  = argv[1];
  out_fname = argv[2];

  mris = MRISread(in_fname);
  if (!mris)
    ErrorExit(ERROR_NOFILE, "%s: could not read surface file %s", Progname,
              in_fname);
  radius = MRISaverageRadius(mris);
  scale  = DEFAULT_RADIUS / radius;
  MRISscaleBrain(mris, mris, scale);
  MRISwrite(mris, out_fname);

  exit(0);
  return (0); /* for ansi */
}

/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int get_option(int argc, char *argv[]) {
  int   nargs = 0;
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
    default:
      fprintf(stderr, "unknown option %s\n", argv[1]);
      exit(1);
      break;
    }

  return (nargs);
}

static void usage_exit() {
  print_usage();
  exit(1);
}

static void print_usage() {
  fprintf(stderr, "usage: %s [options] <input surface file> <output surf>\n",
          Progname);
}

static void print_help() {
  print_usage();
  fprintf(stderr, "\nThis program will rescale a surface representation\n");
  fprintf(stderr, "\nvalid options are:\n\n");
  exit(1);
}

static void print_version() {
  fprintf(stderr, "%s\n", vcid);
  exit(1);
}
