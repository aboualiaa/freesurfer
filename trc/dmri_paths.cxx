/**
 * @brief Probabilistic global tractography
 *
 * Probabilistic global tractography
 */
/*
 * Original Author: Anastasia Yendiki
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

#include "coffin.h"

#include <sys/utsname.h>

#include "cmdargs.h"
#include "diag.h"
#include "fio.h"
#include "timer.h"
#include "version.h"

static int  parse_commandline(int argc, char **argv);
static void check_options();
static void print_usage();
static void usage_exit();
static void print_help();
static void print_version();
static void dump_options();

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

const char *Progname = "dmri_paths";

unsigned int nlab1 = 0, nlab2 = 0;
unsigned int nTract = 1, nBurnIn = 5000, nSample = 5000, nKeepSample = 10,
             nUpdateProp = 40, localPriorSet = 15, neighPriorSet = 14;
float fminPath = 0;
char *dwiFile = nullptr, *gradFile = nullptr, *bvalFile = nullptr,
     *maskFile = nullptr, *bedpostDir = nullptr, *baseXfmFile = nullptr,
     *baseMaskFile = nullptr, *affineXfmFile = nullptr,
     *nonlinXfmFile = nullptr;
std::vector<char *> outDir, inDirList, initFile, roiFile1, roiFile2,
    roiMeshFile1, roiMeshFile2, roiRefFile1, roiRefFile2, xyzPriorFile0,
    xyzPriorFile1, tangPriorFile, curvPriorFile, neighPriorFile, neighIdFile,
    localPriorFile, localIdFile, asegList, stdPropFile;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  bool doxyzprior   = true;
  bool dotangprior  = true;
  bool docurvprior  = true;
  bool doneighprior = true;
  bool dolocalprior = true;
  bool dopropinit   = true;
  int  nargs;
  int  cputime;
  int  ilab1 = 0;
  int  ilab2 = 0;

  nargs = handleVersionOption(argc, argv, "dmri_paths");
  if (nargs && argc - nargs == 1)
    exit(0);
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

  srand(6875);
  srand48(6875);

  if (xyzPriorFile0.empty()) {
    doxyzprior = false;
  }
  if (tangPriorFile.empty()) {
    dotangprior = false;
  }
  if (curvPriorFile.empty()) {
    docurvprior = false;
  }
  if (neighPriorFile.empty()) {
    doneighprior = false;
  }
  if (localPriorFile.empty()) {
    dolocalprior = false;
  }
  if (stdPropFile.empty()) {
    dopropinit = false;
  }

  Coffin mycoffin(
      outDir[0], inDirList, dwiFile, gradFile, bvalFile, maskFile, bedpostDir,
      nTract, fminPath, baseXfmFile, baseMaskFile, initFile[0], roiFile1[0],
      roiFile2[0],
      strstr(roiFile1[0], ".label") != nullptr ? roiMeshFile1[ilab1] : nullptr,
      strstr(roiFile2[0], ".label") != nullptr ? roiMeshFile2[ilab2] : nullptr,
      strstr(roiFile1[0], ".label") != nullptr ? roiRefFile1[ilab1] : nullptr,
      strstr(roiFile2[0], ".label") != nullptr ? roiRefFile2[ilab2] : nullptr,
      doxyzprior ? xyzPriorFile0[0] : nullptr,
      doxyzprior ? xyzPriorFile1[0] : nullptr,
      dotangprior ? tangPriorFile[0] : nullptr,
      docurvprior ? curvPriorFile[0] : nullptr,
      doneighprior ? neighPriorFile[0] : nullptr,
      doneighprior ? neighIdFile[0] : nullptr, doneighprior ? neighPriorSet : 0,
      dolocalprior ? localPriorFile[0] : nullptr,
      dolocalprior ? localIdFile[0] : nullptr, dolocalprior ? localPriorSet : 0,
      asegList, affineXfmFile, nonlinXfmFile, nBurnIn, nSample, nKeepSample,
      nUpdateProp, dopropinit ? stdPropFile[0] : nullptr, debug != 0);

  if (strstr(roiFile1[0], ".label") != nullptr) {
    ilab1++;
  }
  if (strstr(roiFile2[0], ".label") != nullptr) {
    ilab2++;
  }

  for (unsigned int iout = 0; iout < outDir.size(); iout++) {
    if (iout > 0) {
      mycoffin.SetOutputDir(outDir[iout]);
      mycoffin.SetPathway(
          initFile[iout], roiFile1[iout], roiFile2[iout],
          strstr(roiFile1[iout], ".label") != nullptr ? roiMeshFile1[ilab1]
                                                      : nullptr,
          strstr(roiFile2[iout], ".label") != nullptr ? roiMeshFile2[ilab2]
                                                      : nullptr,
          strstr(roiFile1[iout], ".label") != nullptr ? roiRefFile1[ilab1]
                                                      : nullptr,
          strstr(roiFile2[iout], ".label") != nullptr ? roiRefFile2[ilab2]
                                                      : nullptr,
          doxyzprior ? xyzPriorFile0[iout] : nullptr,
          doxyzprior ? xyzPriorFile1[iout] : nullptr,
          dotangprior ? tangPriorFile[iout] : nullptr,
          docurvprior ? curvPriorFile[iout] : nullptr,
          doneighprior ? neighPriorFile[iout] : nullptr,
          doneighprior ? neighIdFile[iout] : nullptr,
          dolocalprior ? localPriorFile[iout] : nullptr,
          dolocalprior ? localIdFile[iout] : nullptr);
      mycoffin.SetMcmcParameters(nBurnIn, nSample, nKeepSample, nUpdateProp,
                                 dopropinit ? stdPropFile[iout] : nullptr);

      if (strstr(roiFile1[iout], ".label") != nullptr) {
        ilab1++;
      }
      if (strstr(roiFile2[iout], ".label") != nullptr) {
        ilab2++;
      }
    }

    std::cout << "Processing pathway " << iout + 1 << " of " << outDir.size()
              << "..." << std::endl;
    cputimer.reset();

    // if (mycoffin.RunMcmcFull())
    if (mycoffin.RunMcmcSingle()) {
      mycoffin.WriteOutputs();
    } else {
      std::cout << "ERROR: Pathway reconstruction failed" << std::endl;
    }

    cputime = cputimer.milliseconds();
    printf("Done in %g sec.\n", cputime / 1000.0);
  }

  printf("dmri_paths done\n");
  return (0);
  exit(0);
}

/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv) {
  int    nargc;
  int    nargsused;
  char **pargv;
  char * option;

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
    } else if (strcmp(option, "--outdir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outDir.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--indir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        inDirList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--dwi") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      dwiFile   = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--grad") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      gradFile  = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--bval") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      bvalFile  = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--mask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      maskFile  = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--bpdir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      bedpostDir = pargv[0];
      nargsused  = 1;
    } else if (strcmp(option, "--ntr") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &nTract);
      nargsused = 1;
    } else if (strcmp(option, "--fmin") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%f", &fminPath);
      nargsused = 1;
    } else if (strcmp(option, "--basereg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      baseXfmFile = pargv[0];
      nargsused   = 1;
    } else if (strcmp(option, "--basemask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      baseMaskFile = fio_fullpath(pargv[0]);
      nargsused    = 1;
    } else if (strcmp(option, "--roi1") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiFile1.push_back(fio_fullpath(pargv[nargsused]));
        if (strstr(*(roiFile1.end() - 1), ".label") != nullptr) {
          nlab1++;
        }
        nargsused++;
      }
    } else if (strcmp(option, "--roi2") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiFile2.push_back(fio_fullpath(pargv[nargsused]));
        if (strstr(*(roiFile2.end() - 1), ".label") != nullptr) {
          nlab2++;
        }
        nargsused++;
      }
    } else if (strcmp(option, "--roimesh1") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiMeshFile1.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--roimesh2") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiMeshFile2.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--roiref1") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiRefFile1.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--roiref2") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        roiRefFile2.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--reg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      affineXfmFile = fio_fullpath(pargv[0]);
      nargsused     = 1;
    } else if (strcmp(option, "--regnl") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nonlinXfmFile = fio_fullpath(pargv[0]);
      nargsused     = 1;
    } else if (strcmp(option, "--init") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        initFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--sdp") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        stdPropFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--prior") == 0) {
      if (nargc < 2) {
        CMDargNErr(option, 2);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        xyzPriorFile0.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
        xyzPriorFile1.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--nprior") == 0) {
      if (nargc < 2) {
        CMDargNErr(option, 2);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        neighPriorFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
        neighIdFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--nset") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &neighPriorSet);
      nargsused = 1;
    } else if (strcmp(option, "--lprior") == 0) {
      if (nargc < 2) {
        CMDargNErr(option, 2);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        localPriorFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
        localIdFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--lset") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &localPriorSet);
      nargsused = 1;
    } else if (strcmp(option, "--seg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        asegList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--tprior") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        tangPriorFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--cprior") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        curvPriorFile.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--nb") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &nBurnIn);
      nargsused = 1;
    } else if (strcmp(option, "--ns") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &nSample);
      nargsused = 1;
    } else if (strcmp(option, "--nk") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &nKeepSample);
      nargsused = 1;
    } else if (strcmp(option, "--nu") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%u", &nUpdateProp);
      nargsused = 1;
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
  std::cout
      << std::endl
      << "USAGE: " << Progname << std::endl
      << std::endl
      << "Basic inputs (native DWI space)" << std::endl
      << "   --indir <dir> [...]:" << std::endl
      << "     Input subject directory (optional)" << std::endl
      << "     If specified, names of all basic inputs are relative to this"
      << std::endl
      << "     Specify multiple input directories for longitudinal data"
      << std::endl
      << "   --outdir <dir> [...]:" << std::endl
      << "     Output directory (one per path)" << std::endl
      << "   --dwi <file>:" << std::endl
      << "     DWI volume series" << std::endl
      << "   --grad <file>:" << std::endl
      << "     Text file of diffusion gradients" << std::endl
      << "   --bval <file>:" << std::endl
      << "     Text file of diffusion b-values" << std::endl
      << "   --mask <file>:" << std::endl
      << "     Mask volume" << std::endl
      << "   --bpdir <dir>:" << std::endl
      << "     BEDPOST directory" << std::endl
      << "   --ntr <num>:" << std::endl
      << "     Max number of tracts per voxel (default 1)" << std::endl
      << "   --fmin <num>:" << std::endl
      << "     Tract volume fraction threshold (default 0)" << std::endl
      << "   --basereg <file> [...]:" << std::endl
      << "     Base-to-DWI registration, needed for longitudinal data only"
      << std::endl
      << "     (.mat, as many as input directories)" << std::endl
      << std::endl
      << "Longitudinal inputs (base template space)" << std::endl
      << "   --basemask <file>:" << std::endl
      << "     Base template mask volume" << std::endl
      << std::endl
      << "End ROIs (atlas space)" << std::endl
      << "   --roi1 <file> [...]:" << std::endl
      << "     End ROI 1 (volume or label, one per path)" << std::endl
      << "   --roi2 <file> [...]:" << std::endl
      << "     End ROI 2 (volume or label, one per path)" << std::endl
      << "   --roimesh1 <file> [...]:" << std::endl
      << "     Mesh for end ROI 1 (for label ROIs)" << std::endl
      << "   --roimesh2 <file> [...]:" << std::endl
      << "     Mesh for end ROI 2 (for label ROIs)" << std::endl
      << "   --roiref1 <file> [...]:" << std::endl
      << "     Reference volume for end ROI 1 (for label ROIs)" << std::endl
      << "   --roiref2 <file> [...]:" << std::endl
      << "     Reference volume for end ROI 2 (for label ROIs)" << std::endl
      << std::endl
      << "Prior-related inputs (atlas space)" << std::endl
      << "   --prior <file0 file1> [...]:" << std::endl
      << "     Spatial path priors (negative log-likelihoods off and"
      << std::endl
      << "     on the path, one pair per path)" << std::endl
      << "   --nprior <priorfile idfile> [...]:" << std::endl
      << "     Near-neighbor label priors (negative log-likelihood" << std::endl
      << "     and list of labels, one pair per path)" << std::endl
      << "   --nset <num>:" << std::endl
      << "     Subset of near-neighbor label priors (default all)" << std::endl
      << "   --lprior <priorfile idfile> [...]:" << std::endl
      << "     Local-neighbor label priors (negative log-likelihood "
      << std::endl
      << "     and list of labels, one pair per path)" << std::endl
      << "   --lset <num>:" << std::endl
      << "     Subset of local-neighbor label priors (default all)" << std::endl
      << "   --seg <file> [...]:" << std::endl
      << "     Segmentation map of test subject" << std::endl
      << "     Specify multiple segmentation maps for longitudinal data"
      << std::endl
      << "   --tprior <file> [...]:" << std::endl
      << "     Path tangent vector priors (negative log-likelihood,"
      << std::endl
      << "     one per path)" << std::endl
      << "   --cprior <file> [...]:" << std::endl
      << "     Path curvature priors (negative log-likelihood," << std::endl
      << "     one per path)" << std::endl
      << "   --reg <file>:" << std::endl
      << "     DWI-to-atlas affine registration (.mat)" << std::endl
      << "   --regnl <file>:" << std::endl
      << "     DWI-to-atlas nonlinear registration (.m3z)" << std::endl
      << std::endl
      << "MCMC options (native diffusion or base template space)" << std::endl
      << "   --init <file> [...]:" << std::endl
      << "     Text file of initial control points (one per path)" << std::endl
      << "   --nb <num>:" << std::endl
      << "     Number of burn-in samples (default 5000)" << std::endl
      << "   --ns <num>:" << std::endl
      << "     Number of post-burn-in samples (default 5000)" << std::endl
      << "   --nk <num>:" << std::endl
      << "     Keep every nk-th sample (default 10)" << std::endl
      << "   --nu <num>:" << std::endl
      << "     Update proposal every nu-th sample (default 40)" << std::endl
      << "   --sdp <file> [...]:" << std::endl
      << "     Text file with initial proposal standard deviations" << std::endl
      << "     for control point perturbations (one per path or" << std::endl
      << "     default SD=1 for all control points and all paths)" << std::endl
      << std::endl
      << "Other options" << std::endl
      << "   --debug:     turn on debugging" << std::endl
      << "   --checkopts: don't run anything, just check options and exit"
      << std::endl
      << "   --help:      print out information on how to use this program"
      << std::endl
      << "   --version:   print out version and exit" << std::endl
      << std::endl;
}

/* --------------------------------------------- */
static void print_help() {
  print_usage();

  std::cout << std::endl << "..." << std::endl << std::endl;

  exit(1);
}

/* ------------------------------------------------------ */
static void usage_exit() {
  print_usage();
  exit(1);
}

/* --------------------------------------------- */
static void print_version(void) {
  cout << getVersion() << endl;
  exit(1);
}

/* --------------------------------------------- */
static void check_options() {
  if (outDir.empty()) {
    std::cout << "ERROR: Must specify output directory" << std::endl;
    exit(1);
  }
  if (dwiFile == nullptr) {
    std::cout << "ERROR: Must specify DWI volume series" << std::endl;
    exit(1);
  }
  if (gradFile == nullptr) {
    std::cout << "ERROR: Must specify gradient text file" << std::endl;
    exit(1);
  }
  if (bvalFile == nullptr) {
    std::cout << "ERROR: Must specify b-value text file" << std::endl;
    exit(1);
  }
  if (maskFile == nullptr) {
    std::cout << "ERROR: Must specify mask volume" << std::endl;
    exit(1);
  }
  if (bedpostDir == nullptr) {
    std::cout << "ERROR: Must specify BEDPOST directory" << std::endl;
    exit(1);
  }
  if (initFile.size() != outDir.size()) {
    std::cout
        << "ERROR: Must specify as many control point initialization files"
        << " as outputs" << std::endl;
    exit(1);
  }
  if (roiFile1.size() != outDir.size()) {
    std::cout << "ERROR: Must specify as many end ROI 1's as outputs"
              << std::endl;
    exit(1);
  }
  if (roiFile2.size() != outDir.size()) {
    std::cout << "ERROR: Must specify as many end ROI 2's as outputs"
              << std::endl;
    exit(1);
  }
  if (roiMeshFile1.size() != nlab1) {
    std::cout << "ERROR: Must specify as many meshes as labels for ROI 1"
              << std::endl;
    exit(1);
  }
  if (roiRefFile1.size() != nlab1) {
    std::cout
        << "ERROR: Must specify as many reference volumes as labels for ROI 1"
        << std::endl;
    exit(1);
  }
  if (roiMeshFile2.size() != nlab2) {
    std::cout << "ERROR: Must specify as many meshes as labels for ROI 2"
              << std::endl;
    exit(1);
  }
  if (roiRefFile2.size() != nlab2) {
    std::cout
        << "ERROR: Must specify as many reference volumes as labels for ROI 2"
        << std::endl;
    exit(1);
  }
  if (!xyzPriorFile0.empty() && xyzPriorFile0.size() != outDir.size()) {
    std::cout << "ERROR: Must specify as many spatial prior pairs as outputs"
              << std::endl;
    exit(1);
  }
  if (!neighPriorFile.empty() && neighPriorFile.size() != outDir.size()) {
    std::cout
        << "ERROR: Must specify as many neighbor aseg prior pairs as outputs"
        << std::endl;
    exit(1);
  }
  if (!localPriorFile.empty() && localPriorFile.size() != outDir.size()) {
    std::cout << "ERROR: Must specify as many local aseg prior pairs as outputs"
              << std::endl;
    exit(1);
  }
  if (!localPriorFile.empty() &&
      (localPriorSet != 1 && localPriorSet != 7 && localPriorSet != 15)) {
    std::cout << "ERROR: invalid set of local aseg priors" << std::endl;
    exit(1);
  }
  if (!neighPriorFile.empty() && (neighPriorSet != 6 && neighPriorSet != 14)) {
    std::cout << "ERROR: invalid set of neighbor aseg priors" << std::endl;
    exit(1);
  }
  if (asegList.empty() &&
      (!neighPriorFile.empty() || !localPriorFile.empty())) {
    std::cout << "ERROR: Must specify segmentation map file with aseg prior"
              << std::endl;
    exit(1);
  }
  if (!stdPropFile.empty() && stdPropFile.size() != outDir.size()) {
    std::cout << "ERROR: Must specify as many control point proposal"
              << " standard deviation files as outputs" << std::endl;
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options() {
  vector<char *>::const_iterator istr;

  cout << endl
       << getVersion() << endl
       << "cwd " << cwd << endl
       << "cmdline " << cmdline << endl
       << "sysname  " << uts.sysname << endl
       << "hostname " << uts.nodename << endl
       << "machine  " << uts.machine << endl
       << "user     " << VERuser() << endl;

  cout << "Output directory:";
  for (istr = outDir.begin(); istr < outDir.end(); istr++)
    cout << " " << *istr;
  cout << endl;

  if (!inDirList.empty()) {
    std::cout << "Input directory:";
    for (istr = inDirList.begin(); istr < inDirList.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }

  std::cout << "DWIs: " << dwiFile << std::endl
            << "Gradients: " << gradFile << std::endl
            << "B-values: " << bvalFile << std::endl
            << "Mask: " << maskFile << std::endl
            << "BEDPOST directory: " << bedpostDir << std::endl
            << "Max number of tracts per voxel: " << nTract << std::endl
            << "Tract volume fraction threshold: " << fminPath << std::endl;

  std::cout << "Initial control point file:";
  for (istr = initFile.begin(); istr < initFile.end(); istr++) {
    std::cout << " " << *istr;
  }
  std::cout << std::endl;

  std::cout << "End ROI 1:";
  for (istr = roiFile1.begin(); istr < roiFile1.end(); istr++) {
    std::cout << " " << *istr;
  }
  std::cout << std::endl;

  if (nlab1 > 0) {
    std::cout << "End ROI 1 mesh:";
    for (istr = roiMeshFile1.begin(); istr < roiMeshFile1.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "End ROI 1 reference volume:";
    for (istr = roiRefFile1.begin(); istr < roiRefFile1.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }

  std::cout << "End ROI 2:";
  for (istr = roiFile2.begin(); istr < roiFile2.end(); istr++) {
    std::cout << " " << *istr;
  }
  std::cout << std::endl;

  if (nlab2 > 0) {
    std::cout << "End ROI 2 mesh:";
    for (istr = roiMeshFile2.begin(); istr < roiMeshFile2.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "End ROI 2 reference volume:";
    for (istr = roiRefFile2.begin(); istr < roiRefFile2.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }

  if (!xyzPriorFile0.empty()) {
    std::cout << "Spatial prior (off path):";
    for (istr = xyzPriorFile0.begin(); istr < xyzPriorFile0.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "Spatial prior (on path):";
    for (istr = xyzPriorFile1.begin(); istr < xyzPriorFile1.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }

  if (!neighPriorFile.empty()) {
    std::cout << "Neighbor aseg prior:";
    for (istr = neighPriorFile.begin(); istr < neighPriorFile.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "Neighbor aseg label ID list:";
    for (istr = neighIdFile.begin(); istr < neighIdFile.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "Neighbor aseg prior set: " << neighPriorSet << std::endl;
  }

  if (!localPriorFile.empty()) {
    std::cout << "Local aseg prior:";
    for (istr = localPriorFile.begin(); istr < localPriorFile.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "Local aseg label ID list:";
    for (istr = localIdFile.begin(); istr < localIdFile.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;

    std::cout << "Local aseg prior set: " << localPriorSet << std::endl;
  }

  if (!asegList.empty()) {
    std::cout << "Segmentation map: ";
    for (istr = asegList.begin(); istr < asegList.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }

  if (affineXfmFile != nullptr) {
    std::cout << "DWI-to-atlas affine registration: " << affineXfmFile
              << std::endl;
  }

  if (nonlinXfmFile != nullptr) {
    std::cout << "DWI-to-atlas nonlinear registration: " << nonlinXfmFile
              << std::endl;
  }

  std::cout << "Number of burn-in samples: " << nBurnIn << std::endl
            << "Number of post-burn-in samples: " << nSample << std::endl
            << "Keep every: " << nKeepSample << "-th sample" << std::endl
            << "Update proposal every: " << nUpdateProp << "-th sample"
            << std::endl;

  if (!stdPropFile.empty()) {
    std::cout << "Initial proposal SD file:";
    for (istr = stdPropFile.begin(); istr < stdPropFile.end(); istr++) {
      std::cout << " " << *istr;
    }
    std::cout << std::endl;
  }
}
