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

#include "diag.h"
#include "error.h"
#include "mri.h"
#include "version.h"

int         main(int argc, char *argv[]);
static void print_help();
static int  get_option(char *argv[]);

const char *Progname;

#define WINDOW_SIZE 5

static int verbose     = 1;
static int window_size = WINDOW_SIZE;

int main(int argc, char *argv[]) {
  char **av;
  int    ac;
  int    nargs;
  MRI *  mri_src;
  MRI *  mri_dst = nullptr;
  char * in_fname;
  char * out_fname;

  nargs = handleVersionOption(argc, argv, "mri_polv");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;

  Progname = argv[0];
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  ac = argc;
  av = argv;
  for (; argc > 1 && ISOPTION(*argv[1]); argc--, argv++) {
    nargs = get_option(argv);
    argc -= nargs;
    argv += nargs;
  }

  if (argc < 1) {
    argc = 1;
  }

  if (argc < 1) {
    ErrorExit(ERROR_BADPARM, "%s: no input name specified", Progname);
  }
  in_fname = argv[1];

  if (argc < 2) {
    ErrorExit(ERROR_BADPARM, "%s: no output name specified", Progname);
  }
  out_fname = argv[2];

  if (verbose != 0) {
    fprintf(stderr, "reading from %s...", in_fname);
  }
  mri_src = MRIread(in_fname);
  if (mri_src == nullptr) {
    ErrorExit(ERROR_NO_FILE, "%s: could not open source file %s", Progname,
              in_fname);
  }

  if (verbose != 0) {
    fprintf(stderr, "done.\ncalculating plane of least variance...");
  }
  mri_dst = MRIcentralPlaneOfLeastVarianceNormal(mri_src, nullptr, window_size);
  if (mri_dst == nullptr) {
    ErrorExit(ERROR_BADPARM, "%s: plane of least variance calculation failed",
              Progname);
  }

  if (verbose != 0) {
    fprintf(stderr, "\ndone. writing output to %s", out_fname);
  }
  MRIwrite(mri_dst, out_fname);
  if (verbose != 0) {
    fprintf(stderr, "\n");
  }
  exit(0);
}

/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int get_option(char *argv[]) {
  int   nargs = 0;
  char *option;

  option = argv[1] + 1; /* past '-' */
  if (stricmp(option, "-help") == 0) {
    print_help();
  }
  switch (toupper(*option)) {
  case 'V':
    verbose = static_cast<int>(verbose) == 0;
    break;
  case 'W':
    if (sscanf(argv[2], "%d", &window_size) < 0) {
      ErrorExit(ERROR_BADPARM, "%s: could not scan window size from %s",
                Progname, argv[2]);
    }
    fprintf(stderr, "using a %d x %d window of CPOLV calcualation\n",
            window_size, window_size);
    nargs = 1;
    break;
  case '?':
  case 'U':
    print_help();
    break;
  default:
    fprintf(stderr, "unknown option %s\n", argv[1]);
    exit(1);
  }

  return (nargs);
}

static void print_help() {
  printf("usage: %s [options] <input image> <output image>\n", Progname);
  printf("\n");
  printf("calculate an image specifying the plane of least variance at each\n");
  printf("point in the input image.\n\n");
  printf("Valid options are:\n\n");
  printf("  -w <window size>   - specify the window size to be used in the "
         "calculation\n"
         "                       of the central plane of least variance"
         " (default=5).\n");
  exit(1);
}
