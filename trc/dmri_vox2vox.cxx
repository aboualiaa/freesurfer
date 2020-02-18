/**
 * @file  dmri_vox2vox.cxx
 * @brief Apply affine and non-linear warp to voxel coordinates in text file
 *
 * Apply affine and non-linear warp to voxel coordinates in text file
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2013/02/12 01:58:07 $
 *    $Revision: 1.3 $
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

#include "vial.h" // Needs to be included first because of CVS libs

#include <sys/utsname.h>

#include "cmdargs.h"
#include "diag.h"
#include "fio.h"
#include "timer.h"
#include "version.h"

using namespace std;

static int parse_commandline(int argc, char **argv);
static void check_options();
static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();
static void dump_options(FILE *fp);

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

static char vcid[] = "";
const char *Progname = "dmri_vox2vox";

int doInvNonlin = 0, nin = 0, nout = 0;
char *inDir = nullptr, *inFile[100], *outDir = nullptr, *outFile[100],
     *inRefFile = nullptr, *outRefFile = nullptr, *affineXfmFile = nullptr,
     *nonlinXfmFile = nullptr;

struct utsname uts;
char *cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  int cputime;
  char fname[PATH_MAX];
  vector<float> point(3);
  MRI *inref = 0;
  MRI *outref = 0;
  AffineReg affinereg;
#ifndef NO_CVS_UP_IN_HERE
  NonlinReg nonlinreg;
#endif

  nargs = handleVersionOption(argc, argv, "dmri_vox2vox");
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

  dump_options(stdout);

  // Read reference volumes
  inref = MRIread(inRefFile);
  outref = MRIread(outRefFile);

  // Read transform files
#ifndef NO_CVS_UP_IN_HERE
  if (nonlinXfmFile != nullptr) {
    if (affineXfmFile != nullptr) {
      affinereg.ReadXfm(affineXfmFile, inref, nullptr);
    }
    nonlinreg.ReadXfm(nonlinXfmFile, outref);
  } else
#endif
      if (affineXfmFile != nullptr) {
    affinereg.ReadXfm(affineXfmFile, inref, outref);
  }

  for (int k = 0; k < nout; k++) {
    float coord;
    ifstream infile;
    ofstream outfile;
    vector<float> inpts;

    printf("Processing coordinate file %d of %d...\n", k + 1, nout);
    cputimer.reset();

    // Read input text file
    if (inDir != nullptr) {
      sprintf(fname, "%s/%s", inDir, inFile[k]);
    } else {
      strcpy(fname, inFile[k]);
    }

    infile.open(fname, ios::in);
    if (!infile) {
      cout << "ERROR: Could not open " << fname << " for reading" << endl;
      exit(1);
    }

    inpts.clear();
    while (infile >> coord) {
      inpts.push_back(coord);
    }

    if (inpts.size() % 3 != 0) {
      cout << "ERROR: File " << fname << " must contain triplets of coordinates"
           << endl;
      exit(1);
    }

    infile.close();

    for (auto ipt = inpts.begin(); ipt < inpts.end(); ipt += 3) {
      copy(ipt, ipt + 3, point.begin());

      // Apply affine transform
      if (!affinereg.IsEmpty()) {
        affinereg.ApplyXfm(point, point.begin());
      }

#ifndef NO_CVS_UP_IN_HERE
      // Apply nonlinear transform
      if (!nonlinreg.IsEmpty()) {
        if (doInvNonlin != 0) {
          nonlinreg.ApplyXfmInv(point, point.begin());
        } else {
          nonlinreg.ApplyXfm(point, point.begin());
        }
      }
#endif

      copy(point.begin(), point.end(), ipt);
    }

    // Write output text file
    if (outDir != nullptr) {
      sprintf(fname, "%s/%s", outDir, outFile[k]);
    } else {
      strcpy(fname, outFile[k]);
    }

    outfile.open(fname, ios::out);
    if (!outfile) {
      cout << "ERROR: Could not open " << fname << " for writing" << endl;
      exit(1);
    }

    for (auto ipt = inpts.begin(); ipt < inpts.end(); ipt += 3) {
      outfile << ipt[0] << " " << ipt[1] << " " << ipt[2] << endl;
    }

    outfile.close();

    cputime = cputimer.milliseconds();
    printf("Done in %g sec.\n", cputime / 1000.0);
  }

  MRIfree(&inref);
  MRIfree(&outref);

  printf("dmri_vox2vox done\n");
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
    } else if (strcmp(option, "--indir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inDir = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--in") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        inFile[nin] = pargv[nargsused];
        nargsused++;
        nin++;
      }
    } else if (strcmp(option, "--outdir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outDir = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outFile[nout] = pargv[nargsused];
        nargsused++;
        nout++;
      }
    } else if (strcmp(option, "--inref") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inRefFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--outref") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outRefFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--reg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      affineXfmFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--regnl") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nonlinXfmFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcasecmp(option, "--invnl") == 0) {
      doInvNonlin = 1;
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
  printf("\n");
  printf("USAGE: ./dmri_vox2vox\n");
  printf("\n");
  printf("Basic inputs\n");
  printf("   --in <file> [...]:\n");
  printf("     Input text file(s)\n");
  printf("   --indir <dir>:\n");
  printf("     Input directory (optional)\n");
  printf("     If specified, names of input text files are relative to this\n");
  printf("   --out <file> [...]:\n");
  printf("     Output text file(s), as many as inputs\n");
  printf("   --outdir <dir>:\n");
  printf("     Output directory (optional)\n");
  printf(
      "     If specified, names of output text files are relative to this)\n");
  printf("   --inref <file>:\n");
  printf("     Input reference volume\n");
  printf("   --outref <file>:\n");
  printf("     Output reference volume\n");
  printf("   --reg <file>:\n");
  printf("     Affine registration (.mat), applied first\n");
  printf("   --regnl <file>:\n");
  printf("     Nonlinear registration (.m3z), applied second\n");
  printf("   --invnl:\n");
  printf("     Apply inverse of nonlinear warp (with --regnl, default: no)\n");
  printf("\n");
  printf("Other options\n");
  printf("   --debug:     turn on debugging\n");
  printf("   --checkopts: don't run anything, just check options and exit\n");
  printf("   --help:      print out information on how to use this program\n");
  printf("   --version:   print out version and exit\n");
  printf("\n");
}

/* --------------------------------------------- */
static void print_help() {
  print_usage();
  printf("\n");
  printf("...\n");
  printf("\n");
  exit(1);
}

/* ------------------------------------------------------ */
static void usage_exit() {
  print_usage();
  exit(1);
}

/* --------------------------------------------- */
static void print_version() {
  printf("%s\n", vcid);
  exit(1);
}

/* --------------------------------------------- */
static void check_options() {
  if (nin == 0) {
    printf("ERROR: must specify input text file(s)\n");
    exit(1);
  }
  if (nout == 0) {
    printf("ERROR: must specify output text file(s)\n");
    exit(1);
  }
  if (nout != nin) {
    printf("ERROR: must specify as many output text files as input files\n");
    exit(1);
  }
  if (inRefFile == nullptr) {
    printf("ERROR: must specify input reference volume\n");
    exit(1);
  }
  if (outRefFile == nullptr) {
    printf("ERROR: must specify output reference volume\n");
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options(FILE *fp) {
  fprintf(fp, "\n");
  fprintf(fp, "%s\n", vcid);
  fprintf(fp, "cwd %s\n", cwd);
  fprintf(fp, "cmdline %s\n", cmdline);
  fprintf(fp, "sysname  %s\n", uts.sysname);
  fprintf(fp, "hostname %s\n", uts.nodename);
  fprintf(fp, "machine  %s\n", uts.machine);
  fprintf(fp, "user     %s\n", VERuser());

  if (inDir != nullptr) {
    fprintf(fp, "Input directory: %s\n", inDir);
  }
  fprintf(fp, "Input files:");
  for (int k = 0; k < nin; k++) {
    fprintf(fp, " %s", inFile[k]);
  }
  fprintf(fp, "\n");
  if (outDir != nullptr) {
    fprintf(fp, "Output directory: %s\n", outDir);
  }
  if (nout > 0) {
    fprintf(fp, "Output files:");
    for (int k = 0; k < nout; k++) {
      fprintf(fp, " %s", outFile[k]);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "Input reference: %s\n", inRefFile);
  fprintf(fp, "Output reference: %s\n", outRefFile);
  if (affineXfmFile != nullptr) {
    fprintf(fp, "Affine registration: %s\n", affineXfmFile);
  }
  if (nonlinXfmFile != nullptr) {
    fprintf(fp, "Nonlinear registration: %s\n", nonlinXfmFile);
    fprintf(fp, "Invert nonlinear morph: %d\n", doInvNonlin);
  }
}
