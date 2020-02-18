/**
 * @file  dmri_spline.cxx
 * @brief Interpolate a spline from its control points
 *
 * Interpolate a spline from its control points
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2013/02/12 01:50:37 $
 *    $Revision: 1.8 $
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

#include <sys/utsname.h>

#include "cmdargs.h"
#include "diag.h"
#include "error.h"
#include "fio.h"
#include "timer.h"
#include "version.h"

#include "spline.h"

using namespace std;

static int parse_commandline(int argc, char **argv);
static void check_options();
static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();
static void dump_options();

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

static char vcid[] = "";
const char *Progname = "dmri_spline";

bool showControls = false;
char *inFile = nullptr, *maskFile = nullptr, *outVolFile = nullptr,
     *outTextFile = nullptr, *outVecBase = nullptr;

struct utsname uts;
char *cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  int cputime;

  nargs = handleVersionOption(argc, argv, "dmri_spline");
  if (nargs && argc - nargs == 1) exit (0);
  argc -= nargs;
  cmdline = argv2cmdline(argc, argv);
  uname(&uts);
  getcwd(cwd, 2000);

  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);

  if (argc == 0) {
    usage_exit();
  }

  parse_commandline(argc, argv);
  check_options();
  if (checkoptsonly != 0) {
    return (0);
  }

  dump_options();

  Spline myspline(inFile, maskFile);

  printf("Computing spline...\n");
  cputimer.reset();

  myspline.InterpolateSpline();

  cputime = cputimer.milliseconds();
  printf("Done in %g sec.\n", cputime / 1000.0);

  if (outVolFile != nullptr) {
    myspline.WriteVolume(outVolFile, showControls);
  }

  if (outTextFile != nullptr) {
    myspline.WriteAllPoints(outTextFile);
  }

  if (outVecBase != nullptr) {
    char fname[PATH_MAX];

    printf("Computing analytical tangent, normal, and curvature...\n");
    cputimer.reset();

    myspline.ComputeTangent(true);
    myspline.ComputeNormal(true);
    myspline.ComputeCurvature(true);

    cputime = cputimer.milliseconds();
    printf("Done in %g sec.\n", cputime / 1000.0);

    // Write tangent, normal, and curvature (analytical) to text files
    sprintf(fname, "%s_tang.txt", outVecBase);
    myspline.WriteTangent(fname);
    sprintf(fname, "%s_norm.txt", outVecBase);
    myspline.WriteNormal(fname);
    sprintf(fname, "%s_curv.txt", outVecBase);
    myspline.WriteCurvature(fname);

    printf("Computing discrete tangent, normal, and curvature...\n");
    cputimer.reset();

    myspline.ComputeTangent(false);
    myspline.ComputeNormal(false);
    myspline.ComputeCurvature(false);

    cputime = cputimer.milliseconds();
    printf("Done in %g sec.\n", cputime / 1000.0);

    // Write tangent, normal, and curvature (discrete) to text files
    sprintf(fname, "%s_tang_diff.txt", outVecBase);
    myspline.WriteTangent(fname);
    sprintf(fname, "%s_norm_diff.txt", outVecBase);
    myspline.WriteNormal(fname);
    sprintf(fname, "%s_curv_diff.txt", outVecBase);
    myspline.WriteCurvature(fname);
  }

  printf("dmri_spline done\n");
  return (0);
  exit(0);
}

/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv) {
  int nargc;
  int nargsused;
  char **pargv;
  char *option;

  if (argc < 1) {
    usage_exit();
  }

  nargc = argc;
  pargv = argv;
  while (nargc > 0) {
    option = pargv[0];
    if (debug != 0) {
      printf("%d %s\n", nargc, option);
    }
    nargc -= 1;
    pargv += 1;

    nargsused = 0;

    if (strcasecmp(option, "--help") == 0) {
      print_help();
    } else if (strcasecmp(option, "--version") == 0) {
      print_version();
    } else if (strcasecmp(option, "--debug") == 0) {
      debug = 1;
    } else if (strcasecmp(option, "--checkopts") == 0) {
      checkoptsonly = 1;
    } else if (strcasecmp(option, "--nocheckopts") == 0) {
      checkoptsonly = 0;
    } else if (strcmp(option, "--cpts") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outVolFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--outpts") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outTextFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--outvec") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outVecBase = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--mask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      maskFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--show") == 0) {
      showControls = true;
    } else {
      fprintf(stderr, "ERROR: Option %s unknown\n", option);
      if (CMDsingleDash(option) != 0) {
        fprintf(stderr, "       Did you really mean -%s ?\n", option);
      }
      exit(-1);
    }
    nargc -= nargsused;
    pargv += nargsused;
  }
  return (0);
}

/* --------------------------------------------- */
static void print_usage() {
  cout << endl
       << "USAGE: " << Progname << endl
       << endl
       << "Basic inputs" << endl
       << "   --cpts <file>:" << endl
       << "     Input text file containing control points" << endl
       << "   --mask <file>:" << endl
       << "     Input mask volume (spline is not allowed to stray off mask)"
       << endl
       << endl
       << "Outputs (at least one output type must be specified)" << endl
       << "   --out <file>:" << endl
       << "     Output volume of the interpolated spline" << endl
       << "   --show:" << endl
       << "     Highlight control points in output volume (default: no)" << endl
       << "   --outpts <file>:" << endl
       << "     Output text file containing all interpolated spline points"
       << endl
       << "   --outvec <base>:" << endl
       << "     Base name of output text files containing tangent vectors,"
       << endl
       << "     normal vectors, and curvatures at every point along the" << endl
       << "     spline (both analytical and finite-difference versions)" << endl
       << endl
       << "Other options" << endl
       << "   --debug:     turn on debugging" << endl
       << "   --checkopts: don't run anything, just check options and exit"
       << endl
       << "   --help:      print out information on how to use this program"
       << endl
       << "   --version:   print out version and exit" << endl
       << endl;
}

/* --------------------------------------------- */
static void print_help() {
  print_usage();

  cout << endl << "..." << endl << endl;

  exit(1);
}

/* ------------------------------------------------------ */
static void usage_exit() {
  print_usage();
  exit(1);
}

/* --------------------------------------------- */
static void print_version() {
  cout << vcid << endl;
  exit(1);
}

/* --------------------------------------------- */
static void check_options() {
  if (inFile == nullptr) {
    cout << "ERROR: Must specify input text file" << endl;
    exit(1);
  }
  if (maskFile == nullptr) {
    cout << "ERROR: Must specify mask volume" << endl;
    exit(1);
  }
  if ((outVolFile == nullptr) && (outTextFile == nullptr) &&
      (outVecBase == nullptr)) {
    cout << "ERROR: Must specify at least one type of output file" << endl;
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options() {
  cout << endl
       << vcid << endl
       << "cwd " << cwd << endl
       << "cmdline " << cmdline << endl
       << "sysname  " << uts.sysname << endl
       << "hostname " << uts.nodename << endl
       << "machine  " << uts.machine << endl
       << "user     " << VERuser() << endl;

  cout << "Control points: " << inFile << endl;
  cout << "Mask volume: " << maskFile << endl;
  if (outVolFile != nullptr) {
    cout << "Output volume: " << outVolFile << endl
         << "Show controls: " << showControls << endl;
  }
  if (outTextFile != nullptr) {
    cout << "Output text file: " << outTextFile << endl;
  }
  if (outVecBase != nullptr) {
    cout << "Output tangent vector file base name: " << outVecBase << endl;
  }
}
