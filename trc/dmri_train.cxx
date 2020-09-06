/**
 * @brief Training of priors for probabilistic global tractography
 *
 * Training of priors for probabilistic global tractography
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

#include "blood.h"

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

const char *Progname = "dmri_train";

bool useTrunc = false, excludeStr = false;
vector<float> trainMaskLabel;
vector<vector<int>> nControl;
vector<std::string> outBase, trainTrkList, trainRoi1List, trainRoi2List,
               testMaskList, testFaList, testBaseXfmList;
std::string outDir, outTestDir, trainListFile,
  trainAsegFile, trainMaskFile, testAffineXfmFile,
  testNonlinXfmFile, testNonlinRefFile, testBaseMaskFile;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs, cputime;
  std::string excfile, fbase;

  nargs = handleVersionOption(argc, argv, "dmri_train");
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

  if (excludeStr) {
    if (!outDir.empty()) {
      excfile = outDir + '/' + outBase.at(0) + "_cpts_all.bad.txt";
    } else {
      excfile = outBase.at(0) + "_cpts_all.bad.txt";
    }
  }
  
  Blood myblood(trainListFile, trainTrkList[0],
                trainRoi1List.size() ? trainRoi1List[0] : std::string(),
                trainRoi2List.size() ? trainRoi2List[0] : std::string(),
                trainAsegFile, trainMaskFile,
                trainMaskLabel.size() ? trainMaskLabel[0] : 0.0f,
                excludeStr ? excfile : std::string(),
                testMaskList, testFaList,
                testAffineXfmFile, testNonlinXfmFile, testNonlinRefFile,
                testBaseXfmList, testBaseMaskFile,
                useTrunc, nControl[0],
                debug);

  for (unsigned int itrk = 0; itrk < trainTrkList.size(); itrk++) {
    if (itrk > 0) {
      if (excludeStr) {
        if (!outDir.empty()) {
	  excfile = outDir + '/' + outBase.at(itrk) + "_cpts_all.bad.txt";
        } else {
	  excfile = outBase.at(itrk) + "_cpts_all.bad.txt";
	}
      }

      if (nControl.size() > 1) { // Variable number of controls
        myblood.SetNumControls(nControl[itrk]);
      }

      myblood.ReadStreamlines(
          trainListFile, trainTrkList[itrk],
          trainRoi1List.size() != 0u ? trainRoi1List[itrk] : nullptr,
          trainRoi2List.size() != 0u ? trainRoi2List[itrk] : nullptr,
          trainMaskLabel.size() != 0u ? trainMaskLabel[itrk] : 0,
          excludeStr ? excfile : nullptr);
    }

    std::cout << "Processing pathway " << itrk + 1 << " of "
              << trainTrkList.size() << "..." << std::endl;
    cputimer.reset();

    myblood.ComputePriors();

    if (!outDir.empty()) {
      fbase = outDir + '/' + outBase.at(itrk);
    } else {
      fbase = outBase.at(itrk);
    }

    if (!outTestDir.empty()) {
      std::string ftbase;

      ftbase = outTestDir + outBase.at(itrk);
      myblood.WriteOutputs(fbase.c_str(), ftbase.c_str());
    }
    else {
      myblood.WriteOutputs(fbase.c_str());
    }

    cputime = cputimer.milliseconds();
    std::cout << "Done in " << cputime / 1000.0 << " sec." << std::endl;
  }

  std::cout << "dmri_train done" << std::endl;
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
      outDir    = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--cptdir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outTestDir = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outBase.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--slist") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      trainListFile = fio_fullpath(pargv[0]);
      nargsused     = 1;
    } else if (strcmp(option, "--trk") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        trainTrkList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--rois") == 0) {
      if (nargc < 2) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        trainRoi1List.push_back(pargv[nargsused]);
        nargsused++;
        trainRoi2List.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--seg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      trainAsegFile = pargv[0];
      nargsused     = 1;
    } else if (strcmp(option, "--cmask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      trainMaskFile = pargv[0];
      nargsused     = 1;
    } else if (strcmp(option, "--lmask") == 0) {
      float labid;
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        sscanf(pargv[nargsused], "%f", &labid);
        trainMaskLabel.push_back(labid);
        nargsused++;
      }
    } else if (strcmp(option, "--bmask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        testMaskList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--fa") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        testFaList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--reg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      testAffineXfmFile = fio_fullpath(pargv[nargsused]);
      nargsused         = 1;
    } else if (strcmp(option, "--regnl") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      testNonlinXfmFile = fio_fullpath(pargv[nargsused]);
      nargsused         = 1;
    } else if (strcmp(option, "--refnl") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      testNonlinRefFile = fio_fullpath(pargv[nargsused]);
      nargsused         = 1;
    } else if (strcmp(option, "--basereg") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        testBaseXfmList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (strcmp(option, "--baseref") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      testBaseMaskFile = fio_fullpath(pargv[nargsused]);
      nargsused        = 1;
    } else if (strcmp(option, "--ncpts") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        int              ncpts;
        std::vector<int> ncptlist;

        sscanf(pargv[nargsused], "%d", &ncpts);
        ncptlist.push_back(ncpts);
        nControl.push_back(ncptlist);
        nargsused++;
      }
    } else if (strcmp(option, "--trunc") == 0) {
      useTrunc = true;
    } else if (strcmp(option, "--xstr") == 0) {
      excludeStr = true;
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
      << "Basic inputs (all must be in common space)" << std::endl
      << "   --out <base> [...]:" << std::endl
      << "     Base name(s) of output(s), one per path" << std::endl
      << "   --outdir <dir>:" << std::endl
      << "     Output directory (optional)" << std::endl
      << "   --cptdir <dir>:" << std::endl
      << "     Output directory for control points in test subject's space"
      << std::endl
      << "     (optional, requires registration files)" << std::endl
      << "     If specified, base names of outputs are relative to this"
      << std::endl
      << "   --slist <file>:" << std::endl
      << "     Text file with list of training subject directories" << std::endl
      << "   --trk <file> [...]:" << std::endl
      << "     Name(s) of input .trk file(s), one per path" << std::endl
      << "     (Names relative to training subject directory)" << std::endl
      << "   --rois <file1> <file2> [...]:" << std::endl
      << "     Optional, names of input tract labeling ROIs, two per path"
      << std::endl
      << "     (Names relative to training subject directory)" << std::endl
      << "   --seg <file>:" << std::endl
      << "     Name of input aparc+aseg volume" << std::endl
      << "     (Name relative to training subject directory)" << std::endl
      << "   --cmask <file>:" << std::endl
      << "     Name of input cortex mask volume" << std::endl
      << "   --lmask <id> [...]:" << std::endl
      << "     Add a label ID from aparc+aseg to cortex mask, one per path"
      << std::endl
      << "     (0 doesn't add any label)" << std::endl
      << "   --bmask <file> [...]:" << std::endl
      << "     Input brain mask volume(s) for test subject" << std::endl
      << "   --fa <file> [...]:" << std::endl
      << "     Input FA volume(s) for test subject (optional)" << std::endl
      << "   --reg <file>:" << std::endl
      << "     Affine registration from atlas to base space (optional)"
      << std::endl
      << "   --regnl <file>:" << std::endl
      << "     Nonlinear registration from atlas to base space (optional)"
      << std::endl
      << "   --refnl <file>:" << std::endl
      << "     Nonlinear registration source reference volume (optional)"
      << std::endl
      << "   --basereg <file> [...]:" << std::endl
      << "     Affine registration(s) from base to FA volume(s) (optional)"
      << std::endl
      << "   --baseref <file> [...]:" << std::endl
      << "     Base space reference volume (optional)" << std::endl
      << "   --ncpts <num> [...]:" << std::endl
      << "     Number of control points for initial spline, one per path"
      << std::endl
      << "     or one for all paths" << std::endl
      << "   --xstr:" << std::endl
      << "     Exclude previously chosen center streamline(s) (Default: No)"
      << std::endl
      << "   --trunc:" << std::endl
      << "     Also save results using all streamlines, truncated or not"
      << std::endl
      << "     (Default: Only save results using non-truncated streamlines)"
      << std::endl
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
  std::cout << getVersion() << std::endl;
  exit(1);
}

/* --------------------------------------------- */
static void check_options() {
  if (outBase.empty()) {
    std::cout << "ERROR: Must specify at least one output name" << std::endl;
    exit(1);
  }
  if (trainListFile.empty()) {
    cout << "ERROR: Must specify training subject list file" << endl;
    exit(1);
  }
  if (trainTrkList.empty()) {
    std::cout << "ERROR: Must specify location of at least one streamline file"
              << std::endl;
    exit(1);
  }
  if (trainTrkList.size() != outBase.size()) {
    std::cout
        << "ERROR: Numbers of input .trk files and output names must match"
        << std::endl;
    exit(1);
  }
  if (trainAsegFile.empty()) {
    cout << "ERROR: Must specify location of aparc+aseg volume" << endl;
    exit(1);
  }
  if (trainMaskFile.empty()) {
    cout << "ERROR: Must specify location of cortex mask volume" << endl;
    exit(1);
  }
  if (!trainRoi1List.empty() && trainRoi1List.size() != trainTrkList.size()) {
    std::cout << "ERROR: Numbers of input .trk files and start ROIs must match"
              << std::endl;
    exit(1);
  }
  if (!trainRoi2List.empty() && trainRoi2List.size() != trainTrkList.size()) {
    std::cout << "ERROR: Numbers of input .trk files and end ROIs must match"
              << std::endl;
    exit(1);
  }
  if (!trainMaskLabel.empty() && trainMaskLabel.size() != trainTrkList.size()) {
    std::cout
        << "ERROR: Numbers of input .trk files and mask label IDs must match"
        << std::endl;
    exit(1);
  }
  if (testMaskList.empty()) {
    std::cout << "ERROR: Must specify brain mask volume for output subject"
              << std::endl;
    exit(1);
  }
  if (!testFaList.empty() && (testFaList.size() != testMaskList.size())) {
    std::cout << "ERROR: Must specify as many FA volumes as brain masks"
              << std::endl;
    exit(1);
  }
  if (nControl.empty()) {
    std::cout
        << "ERROR: Must specify number of control points for initial spline"
        << std::endl;
    exit(1);
  }
  if (nControl.size() > 1 && nControl.size() != trainTrkList.size()) {
    std::cout
        << "ERROR: Must specify number of control points for each .trk file"
        << "ERROR: or a single number of control points for all .trk files"
        << std::endl;
    exit(1);
  }
  if ((!testNonlinXfmFile.empty()) && testNonlinRefFile.empty()) {
    cout << "ERROR: Must specify source reference volume for nonlinear warp"
         << endl;
    exit(1);
  }
  if (!testBaseXfmList.empty() && testBaseMaskFile.empty()) {
    cout << "ERROR: Must specify reference volume for base space" << endl;
    exit(1);
  }
  if (!testBaseXfmList.empty() &&
      (testBaseXfmList.size() != testFaList.size())) {
    std::cout << "ERROR: Must specify as many base registrations as FA volumes"
              << std::endl;
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options() {
  vector<std::string>::const_iterator istr;

  std::cout << std::endl
            << getVersion() << std::endl
            << "cwd " << cwd << std::endl
            << "cmdline " << cmdline << std::endl
            << "sysname  " << uts.sysname << std::endl
            << "hostname " << uts.nodename << std::endl
            << "machine  " << uts.machine << std::endl
            << "user     " << VERuser() << std::endl;

  if (!outDir.empty()) {
    cout << "Output directory: " << outDir << endl;
  }

  if (!outTestDir.empty()) {
    cout << "Output directory in test subject's space: " << outTestDir << endl;
  }

  cout << "Output base:";
  for (istr = outBase.begin(); istr < outBase.end(); istr++) {
    cout << " " << *istr;
  }
  cout << endl;

  std::cout << "Training subject directory list: " << trainListFile
            << std::endl;

  cout << "Location of streamline files relative to base:";
  for (istr = trainTrkList.begin(); istr < trainTrkList.end(); istr++) {
    cout << " " << *istr;
  }
  cout << endl;

  if (!trainRoi1List.empty()) {
    cout << "Location of start ROI files relative to base:";
    for (istr = trainRoi1List.begin(); istr < trainRoi1List.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;

    cout << "Location of end ROI files relative to base:";
    for (istr = trainRoi2List.begin(); istr < trainRoi2List.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }

  std::cout << "Location of cortex masks relative to base: " << trainMaskFile
            << std::endl;

  if (!trainMaskLabel.empty()) {
    cout << "Label ID's from aparc+aseg to add to cortex mask:";
    for (vector<float>::const_iterator ilab = trainMaskLabel.begin();
	 ilab < trainMaskLabel.end(); ilab++) {
      cout << " " << (int) *ilab;
    }
    cout << endl;
  }

  std::cout << "Location of aparc+aseg's relative to base: " << trainAsegFile
            << std::endl;

  cout << "Brain mask for output subject:";
  for (auto ifile = testMaskList.begin();
       ifile < testMaskList.end(); ifile++) {
    cout << " " << *ifile;
  }
  cout << endl;

  if (!testFaList.empty()) {
    cout << "FA map for output subject:";
    for (auto ifile = testFaList.begin();
	 ifile < testFaList.end(); ifile++) {
      cout << " " << *ifile;
    }
    cout << endl;
  }

  if (!testAffineXfmFile.empty()) {
    cout << "Affine registration from atlas to base for output subject: "
         << testAffineXfmFile << endl;
  }

  if (!testNonlinXfmFile.empty()) {
    cout << "Nonlinear registration from atlas to base for output subject: "
         << testNonlinXfmFile << endl;
  }

  if (!testNonlinRefFile.empty()) {
    cout << "Nonlinear registration source reference for output subject: "
         << testNonlinRefFile << endl;
  }

  if (!testBaseXfmList.empty()) {
    cout << "Affine registration from base to FA map for output subject:";
    for (auto ifile = testBaseXfmList.begin();
	 ifile < testBaseXfmList.end(); ifile++) {
      cout << " " << *ifile;
    }
    cout << endl;
  }

  if (!testBaseMaskFile.c_str()) {
    cout << "Base mask for output subject: " << testBaseMaskFile << endl;
  }

  cout << "Number of control points for initial spline:";
  for (vector< vector<int> >::const_iterator inlist = nControl.begin();
       inlist < nControl.end(); inlist++) {
    for (vector<int>::const_iterator inum = inlist->begin();
	 inum < inlist->end(); inum++) {
      cout << " " << *inum;
    }
  }
  cout << endl;

  std::cout << "Number of control points for initial spline:";
  for (auto inlist = nControl.begin(); inlist < nControl.end(); inlist++) {
    for (auto inum = inlist->begin(); inum < inlist->end(); inum++) {
      std::cout << " " << *inum;
    }
  }
  std::cout << std::endl;

  std::cout << "Exclude previously chosen center streamlines: " << excludeStr
            << std::endl
            << "Use truncated streamlines: " << useTrunc << std::endl;
}
