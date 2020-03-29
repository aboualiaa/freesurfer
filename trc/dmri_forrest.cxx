/**
 * @file  dmri_forrest.cxx
 * @brief Random-forrest classifier for white-matter segmentation
 *
 * Random-forrest classifier for white-matter segmentation
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2014/05/27 14:49:34 $
 *    $Revision: 1.2 $
 *
 * Copyright © 2031 The General Hospital Corporation (Boston, MA) "MGH"
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

#include "forrest.h"

#include <sys/utsname.h>

#include "cmdargs.h"
#include "diag.h"
#include "error.h"
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

static char vcid[]   = "";
const char *Progname = "dmri_forrest";

char *testDir = nullptr, *trainListFile = nullptr, *maskFile = nullptr,
     *asegFile = nullptr, *orientFile = nullptr;
std::vector<char *> tractFileList;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  int cputime;
  int nx;
  int ny;
  int nz;
  int ntrain;

  nargs = handleVersionOption(argc, argv, "dmri_forrest");
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

  Forrest myforrest;

  cputimer.reset();

  std::cout << "Reading test subject data..." << std::endl;
  myforrest.ReadTestSubject(testDir, maskFile, asegFile, orientFile);

  // Get volume dimensions from test subject
  nx = myforrest.GetNx();
  ny = myforrest.GetNy();
  nz = myforrest.GetNz();

  std::cout << "Reading training subject data..." << std::endl;
  myforrest.ReadTrainingSubjects(trainListFile, maskFile, asegFile, orientFile,
                                 tractFileList);

  // Get total number of training samples
  ntrain = myforrest.GetNumTrain();

  for (int k = 0; k < 100; k++) {
    const int                 ix    = (int)round(drand48() * (nx - 1));
    const int                 iy    = (int)round(drand48() * (ny - 1));
    const int                 iz    = (int)round(drand48() * (nz - 1));
    const int                 isamp = (int)round(drand48() * (ntrain - 1));
    std::vector<int>          xyz;
    std::vector<unsigned int> aseg;
    std::vector<unsigned int> tracts;
    std::vector<float>        orient;

    // Check if this voxel is inside the brain mask of the test subject
    if (!myforrest.IsInMask(ix, iy, iz)) {
      continue;
    }

    // Get anatomical segmentation neighbors for a voxel in the test subject
    aseg = myforrest.GetTestAseg(ix, iy, iz);
    if (!aseg.empty()) { // If anatomical segmentations were provided
      std::cout << "Anatomical segmentation neighbors of voxel (" << ix << ", "
                << iy << ", " << iz << ")"
                << " in test subject:";
      for (unsigned int iseg = 0; iseg < aseg.size(); iseg++) {
        std::cout << " " << aseg[iseg];
      }
      std::cout << std::endl;
    }

    // Get diffusion orientation for a voxel in the test subject
    orient = myforrest.GetTestOrient(ix, iy, iz);
    if (!orient.empty()) { // If diffusion orientations were provided
      std::cout << "Diffusion orientation at voxel (" << ix << ", " << iy
                << ", " << iz << ")"
                << " in test subject: " << orient[0] << " " << orient[1] << " "
                << orient[2] << std::endl;
    }

    // Sample spatial location from the training data
    xyz = myforrest.GetTrainXyz(isamp);
    std::cout << "Spatial coordinates of training sample " << isamp << ": "
              << xyz[0] << " " << xyz[1] << " " << xyz[2] << std::endl;

    // Sample anatomical segmentation neighbors from the training data
    aseg = myforrest.GetTrainAseg(isamp);
    if (!aseg.empty()) { // If anatomical segmentations were provided
      std::cout << "Anatomical segmentation neighbors of training sample "
                << isamp << ":";
      for (unsigned int iseg = 0; iseg < aseg.size(); iseg++) {
        std::cout << " " << aseg[iseg];
      }
      std::cout << std::endl;
    }

    // Sample diffusion orientation from the training data
    orient = myforrest.GetTrainOrient(isamp);
    if (!orient.empty()) { // If diffusion orientations were provided
      std::cout << "Diffusion orientation of training sample " << isamp << ": "
                << orient[0] << " " << orient[1] << " " << orient[2]
                << std::endl;
    }

    // Sample tract membership from the training data
    tracts = myforrest.GetTrainTractIds(isamp);
    std::cout << "Tract membership of training sample " << isamp << ":";
    if (tracts.empty()) { // If voxel doesn't belong to any tracts
      std::cout << " " << 0 << std::endl;
    } else {
      for (unsigned int itract = 0; itract < tracts.size(); itract++) {
        std::cout << " " << tracts[itract];
      }
      std::cout << std::endl;
    }
  }

  cputime = cputimer.milliseconds();
  std::cout << "Done in " << cputime / 1000.0 << " sec." << std::endl;

  std::cout << "dmri_forrest done" << std::endl;
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
    } else if (strcmp(option, "--test") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      testDir   = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--train") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      trainListFile = fio_fullpath(pargv[0]);
      nargsused     = 1;
    } else if (strcmp(option, "--mask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      maskFile  = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--seg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      asegFile  = pargv[0];
      nargsused = 1;
    } else if (strcmp(option, "--diff") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      orientFile = pargv[0];
      nargsused  = 1;
    } else if (strcmp(option, "--tract") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        tractFileList.push_back(pargv[nargsused]);
        nargsused++;
      }
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
      << "Basic inputs (all files must be in common space)" << std::endl
      << "   --test <dir>:" << std::endl
      << "     Test subject directory" << std::endl
      << "   --train <file>:" << std::endl
      << "     Text file with list of training subject directories" << std::endl
      << "   --mask <file>:" << std::endl
      << "     Name of input brain mask volume" << std::endl
      << "     (Name relative to test/training subject directory)" << std::endl
      << "   --tract <file> [...]:" << std::endl
      << "     Name(s) of input tract label volume(s)" << std::endl
      << "     (Names relative to test/training subject directory)" << std::endl
      << "   --seg <file>:" << std::endl
      << "     Name of input aparc+aseg volume (optional)" << std::endl
      << "     (Name relative to test/training subject directory)" << std::endl
      << "   --diff <file>:" << std::endl
      << "     Name of input diffusion orientation volume (optional)"
      << std::endl
      << "     (Name relative to test/training subject directory)" << std::endl
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
static void print_version() {
  std::cout << vcid << std::endl;
  exit(1);
}

/* --------------------------------------------- */
static void check_options() {
  if (testDir == nullptr) {
    std::cout << "ERROR: Must specify test subject directory" << std::endl;
    exit(1);
  }
  if (trainListFile == nullptr) {
    std::cout << "ERROR: Must specify training subject list file" << std::endl;
    exit(1);
  }
  if (maskFile == nullptr) {
    std::cout << "ERROR: Must specify brain mask volume" << std::endl;
    exit(1);
  }
  if (tractFileList.empty()) {
    std::cout << "ERROR: Must specify at least one tract label volume"
              << std::endl;
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options() {
  std::cout << std::endl
            << vcid << std::endl
            << "cwd " << cwd << std::endl
            << "cmdline " << cmdline << std::endl
            << "sysname  " << uts.sysname << std::endl
            << "hostname " << uts.nodename << std::endl
            << "machine  " << uts.machine << std::endl
            << "user     " << VERuser() << std::endl;

  std::cout << "Test subject directory: " << testDir << std::endl;

  std::cout << "Training subject directory list: " << trainListFile
            << std::endl;

  std::cout << "Location of brain masks relative to subject directory: "
            << maskFile << std::endl;

  std::cout << "Location of streamline files relative to subject directory:";

  for (auto istr = tractFileList.begin(); istr < tractFileList.end(); istr++) {
    std::cout << " " << *istr;
  }
  std::cout << std::endl;

  if (asegFile != nullptr) {
    std::cout << "Location of aparc+aseg's relative to subject directory: "
              << asegFile << std::endl;
  }

  if (orientFile != nullptr) {
    std::cout
        << "Location of diffusion orientations relative to subject directory: "
        << orientFile << std::endl;
  }
}
