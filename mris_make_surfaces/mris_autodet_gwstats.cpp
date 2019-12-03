/**
 * @file  mri_autodet_gwstats.cpp
 * @brief Manages the computation of the gray/white stats used to
 * place the surface (currently in MRIScomputeBorderValues())
 *
 */
/*
 * Original Author: Douglas N Greve (but basically a rewrite of
 * mris_make_surfaces by BF) CVS Revision Info: $Author: greve $ $Date:
 * 2017/02/15 21:04:18 $ $Revision: 1.246 $
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

#include <cstdio>
#include <cstdlib>
#include <cmath>
double round(double x);
#include <sys/utsname.h>
#include <unistd.h>

#include "utils.h"
#include "mrisurf.h"
#include "mrisutils.h"
#include "error.h"
#include "diag.h"
#include "mri.h"
#include "mri2.h"
#include "fio.h"
#include "version.h"
#include "cmdargs.h"
#include "romp_support.h"

static int parse_commandline(int argc, char **argv);
static void check_options();
static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();
static void dump_options(FILE *fp);

struct utsname uts;
char *cmdline, cwd[2000];
int debug = 0, checkoptsonly = 0;
int nthreads = 1;

int main(int argc, char *argv[]);

static char vcid[] = "$Id$";
const char *Progname = "mris_autodet_gwstats";
AutoDetGWStats adgws;
char *outfile = nullptr;
char *involpath = nullptr;
char *wmvolpath = nullptr;
char *insurfpath = nullptr;
char *lhsurfpath = nullptr;
char *rhsurfpath = nullptr;
char *subject = nullptr, *insurfname = "orig",
     *involname = "brain.finalsurfs.mgz", *wmvolname = "wm.mgz";
char tmpstr[2000];
char *SUBJECTS_DIR = nullptr;
int hemicode = -1;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  char *cmdline2, cwd[2000];

  /* rkt: check for and handle version tag */
  nargs = handle_version_option(argc, argv, vcid, "$Name:  $");
  if (nargs && argc - nargs == 1)
    exit(0);
  argc -= nargs;
  cmdline = argv2cmdline(argc, argv);
  uname(&uts);
  getcwd(cwd, 2000);
  cmdline2 = argv2cmdline(argc, argv);

  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);
  DiagInit(nullptr, nullptr, nullptr);
  Gdiag |= DIAG_SHOW;

  if (argc == 0)
    usage_exit();
  parse_commandline(argc, argv);
  check_options();
  if (checkoptsonly)
    return (0);

  // print out version of this program
  printf("%s\n", vcid);
  printf("\n");
  printf("cd %s\n", cwd);
  printf("setenv SUBJECTS_DIR %s\n", getenv("SUBJECTS_DIR"));
  printf("%s\n", cmdline2);
  printf("\n");
  fflush(stdout);

  printf("Reading in intensity volume %s\n", involpath);
  adgws.mri_T1 = MRIread(involpath);
  if (adgws.mri_T1 == nullptr)
    exit(1);

  printf("Reading in wm volume %s\n", wmvolpath);
  adgws.mri_wm = MRIread(wmvolpath);
  if (adgws.mri_wm == nullptr)
    exit(1);

  if (lhsurfpath) {
    printf("Reading in lhsurf %s\n", lhsurfpath);
    adgws.mrisADlh = MRISread(lhsurfpath);
    if (adgws.mrisADlh == nullptr)
      exit(1);
    adgws.hemicode = 1;
  }
  if (rhsurfpath) {
    printf("Reading in rhsurf %s\n", rhsurfpath);
    adgws.mrisADrh = MRISread(rhsurfpath);
    if (adgws.mrisADrh == nullptr)
      exit(1);
    adgws.hemicode = 2;
  }
  if (insurfpath) {
    printf("Reading in surf %s\n", insurfpath);
    adgws.mrisAD = MRISread(insurfpath);
    if (adgws.mrisAD == nullptr)
      exit(1);
    adgws.hemicode = hemicode;
  }

  int err = adgws.AutoDetectStats();
  if (err)
    exit(1);
  adgws.Write(outfile);

  printf("#VMPC# mris_autodet_gwstats VmPeak  %d\n", GetVmPeak());
  printf("mris_autodet_gwstats done\n");

  return (0);
}
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/
/*-----------------------------------------------------------------*/

/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv) {
  int nargc, nargsused;
  char **pargv, *option;

  if (argc < 1)
    usage_exit();

  nargc = argc;
  pargv = argv;
  while (nargc > 0) {

    option = pargv[0];
    if (debug)
      printf("%d %s\n", nargc, option);
    nargc -= 1;
    pargv += 1;

    nargsused = 0;

    if (!strcasecmp(option, "--help"))
      print_help();
    else if (!strcasecmp(option, "--version"))
      print_version();
    else if (!strcasecmp(option, "--debug"))
      debug = 1;
    else if (!strcasecmp(option, "--checkopts"))
      checkoptsonly = 1;
    else if (!strcasecmp(option, "--nocheckopts"))
      checkoptsonly = 0;
    else if (!strcmp(option, "--o")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outfile = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--i")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      involpath = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--wm")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      wmvolpath = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--surf")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      insurfpath = pargv[0];
      char *surfbasename = fio_basename(pargv[0], nullptr);
      hemicode = -1;
      if (strncmp(surfbasename, "lh", 2) == 0)
        hemicode = 1;
      if (strncmp(surfbasename, "rh", 2) == 0)
        hemicode = 2;
      nargsused = 1;
    } else if (!strcmp(option, "--surfs")) {
      if (nargc < 2)
        CMDargNErr(option, 2);
      lhsurfpath = pargv[0];
      rhsurfpath = pargv[1];
      nargsused = 2;
    } else if (!strcmp(option, "--lh-surf")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      lhsurfpath = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--rh-surf")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      rhsurfpath = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--sd")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      printf("using %s as SUBJECTS_DIR...\n", pargv[0]);
      setenv("SUBJECTS_DIR", pargv[0], 1);
      nargsused = 1;
    } else if (!strcmp(option, "--s")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      subject = pargv[0];
      nargsused = 1;
    } else if (!strcasecmp(option, "--threads") ||
               !strcasecmp(option, "--nthreads")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      sscanf(pargv[0], "%d", &nthreads);
#ifdef _OPENMP
      omp_set_num_threads(nthreads);
#endif
      nargsused = 1;
    } else {
      fprintf(stderr, "ERROR: Option %s unknown\n", option);
      if (CMDsingleDash(option))
        fprintf(stderr, "       Did you really mean -%s ?\n", option);
      exit(-1);
    }
    nargc -= nargsused;
    pargv += nargsused;
  }
  return (0);
}
/* --------------------------------------------- */
static void check_options() {
  if (outfile == nullptr) {
    printf("ERROR: no output set\n");
    exit(1);
  }
  SUBJECTS_DIR = getenv("SUBJECTS_DIR");
  if (subject != nullptr) {
    if (involpath == nullptr) {
      sprintf(tmpstr, "%s/%s/mri/%s", SUBJECTS_DIR, subject, involname);
      involpath = strcpyalloc(tmpstr);
    }
    if (wmvolpath == nullptr) {
      sprintf(tmpstr, "%s/%s/mri/%s", SUBJECTS_DIR, subject, wmvolname);
      wmvolpath = strcpyalloc(tmpstr);
    }
    if (lhsurfpath == nullptr) {
      sprintf(tmpstr, "%s/%s/surf/lh.%s", SUBJECTS_DIR, subject, insurfname);
      lhsurfpath = strcpyalloc(tmpstr);
    }
    if (rhsurfpath == nullptr) {
      sprintf(tmpstr, "%s/%s/surf/rh.%s", SUBJECTS_DIR, subject, insurfname);
      rhsurfpath = strcpyalloc(tmpstr);
    }
  }
  if (involpath == nullptr) {
    printf("ERROR: no input volume set\n");
    exit(1);
  }
  if (wmvolpath == nullptr) {
    printf("ERROR: no wm volume set\n");
    exit(1);
  }
  if (lhsurfpath == nullptr && rhsurfpath == nullptr && insurfpath == nullptr) {
    printf("ERROR: no surface set\n");
    exit(1);
  }
  if (lhsurfpath != nullptr && insurfpath != nullptr) {
    printf("ERROR: cannot spec lhsurf and insurf\n");
    exit(1);
  }
  if (rhsurfpath != nullptr && insurfpath != nullptr) {
    printf("ERROR: cannot spec rhsurf and insurf\n");
    exit(1);
  }

  return;
}

/* --------------------------------------------- */
static void print_usage() {
  printf("\n");
  printf("PROGRAM: mris_autodet_gwstats\n");
  printf("Manages the computation of the gray/white statistics used to place "
         "the \n");
  printf("  white and pial surfaces (currently in MRIScomputeBorderValues()\n");
  printf(" --o outputfile : output text file with stats\n");
  printf(" --i  T1wvolume (usually brain.finalsurfs.mgz)\n");
  printf(" --wm wmvolume  (usually wm.mgz)\n");
  printf(" --surf  surf (usually ?h.orig)\n");
  printf(" --surfs lhsurf rhsurf \n");
  printf(" --lh-surf lhsurf \n");
  printf(" --rh-surf rhsurf \n");
  printf(" --s subject : reads in brain.finalsurfs.mgz, wm.mgz, lh.orig and "
         "rh.orig\n");
  printf(" --sd SUBJECTS_DIR \n");
  printf("\n");
}

/* --------------------------------------------- */
static void print_help() {
  print_usage();
  printf("\n");
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
static void dump_options(FILE *fp) {
  fprintf(fp, "\n");
  fprintf(fp, "%s\n", vcid);
  fprintf(fp, "cwd %s\n", cwd);
  fprintf(fp, "cmdline %s\n", cmdline);
  fprintf(fp, "sysname  %s\n", uts.sysname);
  fprintf(fp, "hostname %s\n", uts.nodename);
  fprintf(fp, "machine  %s\n", uts.machine);
  fprintf(fp, "user     %s\n", VERuser());
  return;
}
