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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "diag.h"
#include "error.h"
#include "gcamorph.h"
#include "macros.h"
#include "mri.h"
#include "timer.h"
#include "utils.h"
#include "version.h"

int        main(int argc, char *argv[]);
static int get_option(int argc, char *argv[]);

const char *Progname;
static void usage_exit(int code);

int main(int argc, char *argv[]) {
  char **    av;
  int        ac, nargs;
  int        msec, minutes, seconds;
  Timer      start;
  GCA_MORPH *gcam;
  MRI *      mri_lame;

  nargs = handleVersionOption(argc, argv, "mri_elastic_energy");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;

  Progname = argv[0];
  ErrorInit(NULL, NULL, NULL);
  DiagInit(NULL, NULL, NULL);

  start.reset();

  ac = argc;
  av = argv;
  for (; argc > 1 && ISOPTION(*argv[1]); argc--, argv++) {
    nargs = get_option(argc, argv);
    argc -= nargs;
    argv += nargs;
  }

  if (argc < 3)
    usage_exit(1);

  gcam = GCAMread(argv[1]);
  if (gcam == NULL)
    ErrorExit(ERROR_NOFILE, "%s: could not read gcam from %s", Progname,
              argv[1]);
  mri_lame = GCAMestimateLameConstants(gcam);
  printf("writing energy and lame constants to %s\n", argv[2]);
  MRIwrite(mri_lame, argv[2]);
  msec    = start.milliseconds();
  seconds = nint((float)msec / 1000.0f);
  minutes = seconds / 60;
  seconds = seconds % 60;
  fprintf(stderr,
          "elastic energy calculation took %d minutes"
          " and %d seconds.\n",
          minutes, seconds);
  exit(0);
  return (0);
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int get_option(int argc, char *argv[]) {
  int   nargs = 0;
  char *option;

  option = argv[1] + 1; /* past '-' */
  switch (toupper(*option)) {
  case 'N':
    nargs = 1;
    break;
  case '?':
  case 'U':
    usage_exit(0);
    break;
  default:
    fprintf(stderr, "unknown option %s\n", argv[1]);
    exit(1);
    break;
  }

  return (nargs);
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static void usage_exit(int code) {
  printf("usage: %s [options] <input .m3z> <output energy volume>", Progname);
  printf("\tthe output energy has 5 frames - \n");
  printf("\t(0) total elastic potential energy\n");
  printf("\t(1) rigid energy\n");
  printf("\t(2) volume change energy\n");
  printf("\t(3) lambda (first Lame parameter, bulk modulus)\n");
  printf("\t(3) mu (second Lame parameter, sheer modulus))\n");
  exit(code);
}
