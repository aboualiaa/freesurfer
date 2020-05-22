/**
 * @brief Compute measures of head motion in DWIs
 *
 * Compute measures of head motion in DWIs
 */
/*
 * Original Author: Anastasia Yendiki
 *
 * Copyright © 2013 The General Hospital Corporation (Boston, MA) "MGH"
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
double round(double x);
#include <float.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits.h>
#include <limits>
#include <math.h>
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "cmdargs.h"
#include "diag.h"
#include "error.h"
#include "fio.h"
#include "mri.h"
#include "timer.h"
#include "version.h"

static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void);
static void usage_exit(void);
static void print_help(void);
static void print_version(void);
static void dump_options();

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

const char *Progname = "dmri_motion";

float T = 100, D = .001;

std::vector<char *> inDwiList, inBvalList;
char *              inMatFile = NULL, *outFile = NULL, *outFrameFile = NULL;

MRI *dwi;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int                nargs, cputime;
  float              travg = 0, roavg = 0, score = 0, pbad = 0;
  std::vector<int>   runstart(1, 0), nbadframe;
  std::vector<float> trframe, roframe, scoreframe;
  std::string        matline;
  std::ofstream      outfile;

  nargs = handleVersionOption(argc, argv, "dmri_motion");
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
  DiagInit(NULL, NULL, NULL);

  if (argc == 0)
    usage_exit();

  parse_commandline(argc, argv);
  check_options();
  if (checkoptsonly)
    return (0);

  dump_options();

  cputimer.reset();

  if (!inDwiList.empty()) { // Estimate within-volume motion
    int                nslice = 0, nbad = 0;
    const unsigned int nrun = inDwiList.size();

    for (unsigned int irun = 0; irun < nrun; irun++) {
      int                                nx, ny, nz, nd, nxy;
      float                              minvox, b;
      std::vector<int>                   r, r1;
      std::vector<int>::const_iterator   ir1;
      std::vector<float>                 bvals;
      std::vector<float>::const_iterator ibval;
      std::ifstream                      infile;

      // Read DWI volume series
      std::cout << "Loading DWI volume series from " << inDwiList[irun]
                << std::endl;
      dwi = MRIread(inDwiList[irun]);
      if (!dwi) {
        std::cout << "ERROR: Could not read " << inDwiList[irun] << std::endl;
        exit(1);
      }

      nx = dwi->width;
      ny = dwi->height;
      nz = dwi->depth;
      nd = dwi->nframes;

      runstart.push_back(*(runstart.end() - 1) + nd);

      nxy    = nx * ny;
      minvox = 0.05 * nxy;

      nbadframe.insert(nbadframe.end(), nd, 0);
      scoreframe.insert(scoreframe.end(), nd, 0.0);

      // Read b-value table
      std::cout << "Loading b-value table from " << inBvalList[irun]
                << std::endl;
      infile.open(inBvalList[irun], std::ios::in);
      if (!infile) {
        std::cout << "ERROR: Could not open " << inBvalList[irun]
                  << " for reading" << std::endl;
        exit(1);
      }

      while (infile >> b)
        bvals.push_back(b);

      infile.close();

      if (bvals.size() != (unsigned int)nd) {
        std::cout << "ERROR: Number of b-values (" << bvals.size() << ") in "
                  << inBvalList[irun] << " and number of volumes (" << nd
                  << ") in " << inDwiList[irun] << " do not match" << std::endl;
        exit(1);
      }

      std::cout << "Computing within-volume head motion measures" << std::endl;

      // Find unique b-values
      std::set<float> blist(bvals.begin(), bvals.end());

      // Compare frames acquired with each b-value separately
      for (std::set<float>::const_iterator ib = blist.begin();
           ib != blist.end(); ib++) {
        const float                  thresh = T * exp(-(*ib) * D);
        std::vector<int>::iterator   inbad  = nbadframe.end() - nd;
        std::vector<float>::iterator iscore = scoreframe.end() - nd;

        r1.clear();
        ibval = bvals.begin();

        for (int id = 0; id < nd; id++) {
          if (*ibval == *ib) {
            r.clear();

            // Find number of voxels above threshold in each slice of this frame
            for (int iz = 0; iz < nz; iz++) {
              int count = 0;

              for (int iy = 0; iy < ny; iy++)
                for (int ix = 0; ix < nx; ix++)
                  if (MRIgetVoxVal(dwi, ix, iy, iz, id) > thresh)
                    count++;

              r.push_back(count);
            }

            if (r1.empty()) // First frame with this b-value
              r1.insert(r1.begin(), r.begin(), r.end());

            // Motion score (from Benner et al MRM 2011)
            ir1 = r1.begin();
            for (std::vector<int>::const_iterator ir = r.begin(); ir < r.end();
                 ir++) {
              if (*ir >= minvox) { // Do not count empty slices
                const float S = 2 - *ir / (0.7 * (*ir1));

                nslice++;

                if (S > 1) {
                  (*inbad)++;
                  (*iscore) += S;
                }
              }

              ir1++;
            }

            nbad += *inbad;
            score += *iscore;

            // Average motion score of bad slices in this frame
            if (*inbad > 0)
              *iscore /= *inbad;
            else
              *iscore = 1;
          }

          ibval++;
          inbad++;
          iscore++;
        }
      }
    }

    // Percentage of bad slices among all non-empty slices
    if (nslice > 0)
      pbad = nbad / (float)nslice * 100;

    // Average motion score of bad slices
    if (nbad > 0)
      score /= nbad;
    else
      score = 1;
  }

  if (inMatFile) { // Estimate between-volume motion
    bool               isMat;
    int                nframe = 0;
    std::vector<float> xform, tr(3, 0), ro(3, 0), tr0(3, 0), ro0(3, 0),
        dtr(3, 0), dro(3, 0), trtot(3, 0), rotot(3, 0);
    std::ifstream infile;

    std::cout << "Loading volume-to-baseline affine transformations"
              << std::endl;
    infile.open(inMatFile, std::ios::in);
    if (!infile) {
      std::cout << "ERROR: Could not open " << inMatFile << " for reading"
                << std::endl;
      exit(1);
    }

    // Determine if file contains transformation matrices or parameters
    while (getline(infile, matline))
      if (!matline.empty() && !isalpha(matline[0])) {
        float              xval;
        std::istringstream matstr(matline);

        while (matstr >> xval)
          xform.push_back(xval);

        break;
      }

    if (xform.size() == 4)
      isMat = true;
    else if (xform.size() == 16)
      isMat = false;
    else {
      std::cout << "ERROR: Unexpected number of entries per line ("
                << xform.size() << ") in  " << inMatFile << std::endl;
      exit(1);
    }

    xform.clear();
    infile.clear();
    infile.seekg(0, std::ios::beg);

    std::cout << "Computing between-volume head motion measures" << std::endl;

    while (getline(infile, matline))
      if (!matline.empty() && !isalpha(matline[0])) {
        float              xval;
        std::istringstream matstr(matline);

        if (isMat) { // Read matrix from file and compute parameters
          while (matstr >> xval)
            xform.push_back(xval);

          if (xform.size() == 16) {
            AffineReg reg(xform);

            // Decompose affine registration matrix into its parameters
            reg.DecomposeXfm();

            // Translations with respect to first frame
            copy(reg.GetTranslate(), reg.GetTranslate() + 3, tr.begin());

            // Rotations with respect to first frame
            copy(reg.GetRotate(), reg.GetRotate() + 3, ro.begin());
          } else
            continue;
        } else { // Read parameters from file
          // Translations with respect to first frame
          for (int k = 0; k < 3; k++) {
            matstr >> xval;
            tr[k] = xval;
          }

          // Rotations with respect to first frame
          for (int k = 0; k < 3; k++) {
            matstr >> xval;
            ro[k] = xval;
          }
        }

        // Find translation/rotation with respect to previous frame,
        // unless this is the first frame of a new DWI run
        if (find(runstart.begin(), runstart.end(), nframe) == runstart.end())
          for (int k = 0; k < 3; k++) {
            dtr[k] = tr[k] - tr0[k];
            dro[k] = ro[k] - ro0[k];
          }
        else {
          fill(dtr.begin(), dtr.end(), 0);
          fill(dro.begin(), dro.end(), 0);
        }

        // Frame-to-frame translations
        trframe.insert(trframe.end(), dtr.begin(), dtr.end());

        // Cumulative frame-to-frame translations
        for (int k = 0; k < 3; k++)
          trtot[k] += fabs(*(trframe.end() - 3 + k));

        copy(tr.begin(), tr.end(), tr0.begin());

        // Frame-to-frame rotations
        roframe.insert(roframe.end(), dro.begin(), dro.end());

        // Cumulative frame-to-frame rotations
        for (int k = 0; k < 3; k++)
          rotot[k] += fabs(*(roframe.end() - 3 + k));

        copy(ro.begin(), ro.end(), ro0.begin());

        xform.clear();

        nframe++;
      }

    infile.close();

    std::cout << "INFO: Processed transforms for " << nframe << " volumes"
              << std::endl;

    travg =
        sqrt(trtot[0] * trtot[0] + trtot[1] * trtot[1] + trtot[2] * trtot[2]) /
        nframe;
    roavg = (rotot[0] + rotot[1] + rotot[2]) / nframe;
  }

  // Write overall measures to file
  outfile.open(outFile, std::ios::out);
  outfile << "AvgTranslation AvgRotation PercentBadSlices AvgDropoutScore"
          << std::endl
          << travg << " " << roavg << " " << pbad << " " << score << std::endl;
  outfile.close();

  // Write frame-by-frame measures to file
  if (outFrameFile) {
    std::vector<float>::const_iterator itr, iro, iscore;

    if (trframe.empty())
      trframe.insert(trframe.begin(), nbadframe.size() * 3, 0.0);
    else if (trframe.size() != nbadframe.size() * 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << trframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (roframe.empty())
      roframe.insert(roframe.begin(), nbadframe.size() * 3, 0.0);
    else if (roframe.size() != nbadframe.size() * 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << roframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (nbadframe.empty())
      nbadframe.insert(nbadframe.begin(), trframe.size() / 3, 0);
    else if (nbadframe.size() != trframe.size() / 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << trframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (scoreframe.empty())
      scoreframe.insert(scoreframe.begin(), trframe.size() / 3, 0.0);
    else if (scoreframe.size() != trframe.size() / 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << trframe.size() / 3 << ") and "
                << "within-volume (" << scoreframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    outfile.open(outFrameFile, std::ios::out);
    outfile << "TranslationX TranslationY TranslationZ "
            << "RotationX RotationY RotationZ "
            << "PercentBadSlices AvgDropoutScore" << std::endl;

    itr    = trframe.begin();
    iro    = roframe.begin();
    iscore = scoreframe.begin();

    for (std::vector<int>::const_iterator inbad = nbadframe.begin();
         inbad < nbadframe.end(); inbad++) {

      outfile << itr[0] << " " << itr[1] << " " << itr[2] << " " << iro[0]
              << " " << iro[1] << " " << iro[2] << " " << *inbad << " "
              << *iscore << std::endl;

      itr += 3;
      iro += 3;
      iscore++;
    }

    outfile.close();
  }

  cputime = cputimer.milliseconds();
  std::cout << "Done in " << cputime / 1000.0 << " sec." << std::endl;

  std::cout << "dmri_motion done" << std::endl;
  return (0);
  exit(0);
}

/* --------------------------------------------- */
static int parse_commandline(int argc, char **argv) {
  int    nargc, nargsused;
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
    else if (!strcmp(option, "--dwi")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      while (nargsused < nargc && strncmp(pargv[nargsused], "--", 2)) {
        inDwiList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (!strcmp(option, "--bval")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      while (nargsused < nargc && strncmp(pargv[nargsused], "--", 2)) {
        inBvalList.push_back(fio_fullpath(pargv[nargsused]));
        nargsused++;
      }
    } else if (!strcmp(option, "--mat")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      inMatFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--T")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      sscanf(pargv[0], "%f", &T);
      nargsused = 1;
    } else if (!strcmp(option, "--D")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      sscanf(pargv[0], "%f", &D);
      nargsused = 1;
    } else if (!strcmp(option, "--out")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outFile   = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--outf")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outFrameFile = fio_fullpath(pargv[0]);
      nargsused    = 1;
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
static void print_usage(void) {
  std::cout
      << std::endl
      << "USAGE: " << Progname << std::endl
      << std::endl
      << "Required arguments" << std::endl
      << "   --out <file>:" << std::endl
      << "     Output text file of motion measures" << std::endl
      << std::endl
      << "Optional arguments" << std::endl
      << "   --outf <file>:" << std::endl
      << "     Output text file of frame-by-frame motion measures" << std::endl
      << std::endl
      << "Arguments needed for between-volume motion measures" << std::endl
      << "   --mat <file>:" << std::endl
      << "     Input text file of volume-to-baseline affine transformations"
      << std::endl
      << "     Can be transformation matrices (e.g., from eddy_correct) or"
      << std::endl
      << "     transformation parameters (e.g., from eddy)" << std::endl
      << std::endl
      << "Arguments needed for within-volume motion measures" << std::endl
      << "(see Benner et al MRM 2011):" << std::endl
      << "   --dwi <file> [...]:" << std::endl
      << "     Input DWI scan(s), unprocessed" << std::endl
      << "   --bval <file> [...]:" << std::endl
      << "     Input b-value table(s), one per scan" << std::endl
      << "   --T <num>:" << std::endl
      << "     Low-b image intensity threshold (default: 100)" << std::endl
      << "   --D <num>:" << std::endl
      << "     Nominal diffusivity (default: .001)" << std::endl
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
static void print_help(void) {
  print_usage();

  std::cout << std::endl << "..." << std::endl << std::endl;

  exit(1);
}

/* ------------------------------------------------------ */
static void usage_exit(void) {
  print_usage();
  exit(1);
}

/* --------------------------------------------- */
static void print_version(void) {
  std::cout << getVersion() << std::endl;
  exit(1);
}

/* --------------------------------------------- */
static void check_options(void) {
  if (!outFile) {
    std::cout << "ERROR: must specify output file" << std::endl;
    exit(1);
  }
  if (inDwiList.size() != inBvalList.size()) {
    std::cout << "ERROR: must specify equal numbers of DWI and b-value files"
              << std::endl;
    exit(1);
  }
  if (inBvalList.empty() && !inMatFile) {
    std::cout << "ERROR: must specify inputs for between-volume and/or "
              << "within-volume motion measures" << std::endl;
    exit(1);
  }
  if (D < 0) {
    std::cout << "ERROR: diffusivity must be positive" << std::endl;
    exit(1);
  }
  return;
}

static void dump_options() {
  std::cout << std::endl
            << getVersion() << std::endl
            << "cwd " << cwd << std::endl
            << "cmdline " << cmdline << std::endl
            << "sysname  " << uts.sysname << std::endl
            << "hostname " << uts.nodename << std::endl
            << "machine  " << uts.machine << std::endl
            << "user     " << VERuser() << std::endl;

  std::cout << "Output motion measure file: " << outFile << std::endl;

  if (outFrameFile)
    std::cout << "Output frame-by-frame motion measure file: " << outFrameFile
              << std::endl;

  if (inMatFile)
    std::cout << "Input transform file: " << inMatFile << std::endl;

  if (!inDwiList.empty()) {
    std::cout << "Input DWI file(s):";
    for (std::vector<char *>::const_iterator ifile = inDwiList.begin();
         ifile < inDwiList.end(); ifile++)
      std::cout << " " << *ifile;
    std::cout << std::endl;

    std::cout << "Input b-value table(s):";
    for (std::vector<char *>::const_iterator ifile = inBvalList.begin();
         ifile < inBvalList.end(); ifile++)
      std::cout << " " << *ifile;
    std::cout << std::endl;

    std::cout << "Low-b image intensity threshold: " << T << std::endl;
    std::cout << "Nominal diffusivity: " << D << std::endl;
  }

  return;
}
