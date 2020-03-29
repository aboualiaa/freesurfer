/**
 * @file  dmri_motion.cxx
 * @brief Compute measures of head motion in DWIs
 *
 * Compute measures of head motion in DWIs
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2014/05/27 14:49:34 $
 *    $Revision: 1.6 $
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

static char vcid[]   = "";
const char *Progname = "dmri_motion";

float T = 100, D = .001;

char *inMatFile = nullptr, *inDwiFile = nullptr, *inBvalFile = nullptr,
     *outFile = nullptr, *outFrameFile = nullptr;

MRI *dwi;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int                nargs;
  int                cputime;
  float              travg = 0;
  float              roavg = 0;
  float              score = 0;
  float              pbad  = 0;
  std::vector<int>   nbadframe;
  std::vector<float> trframe;
  std::vector<float> roframe;
  std::vector<float> scoreframe;
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

  cputimer.reset();

  if (inMatFile != nullptr) { // Estimate between-volume motion
    int                nframe = 0;
    std::vector<float> xform;
    std::vector<float> tr0(3, 0);
    std::vector<float> ro0(3, 0);
    std::vector<float> trtot(3, 0);
    std::vector<float> rotot(3, 0);
    std::ifstream      infile;

    std::cout << "Loading volume-to-baseline affine transformations"
              << std::endl;
    infile.open(inMatFile, std::ios::in);
    if (!infile) {
      std::cout << "ERROR: Could not open " << inMatFile << " for reading"
                << std::endl;
      exit(1);
    }

    std::cout << "Computing between-volume head motion measures" << std::endl;

    while (getline(infile, matline)) {
      if (((~static_cast<int>(matline.empty()) != 0)) &&
          ((~isalpha(matline[0])) != 0)) {
        float              xval;
        std::istringstream matstr(matline);

        while (matstr >> xval) {
          xform.push_back(xval);
        }

        if (xform.size() == 16) {
          std::vector<float> tr;
          std::vector<float> ro;
          AffineReg          reg(xform);

          // Decompose affine registration matrix into its parameters
          reg.DecomposeXfm();

          // Translations with respect to first frame
          tr = reg.GetTranslate();

          // Frame-to-frame translations
          for (int k = 0; k < 3; k++) {
            trframe.push_back(tr[k] - tr0[k]);
          }

          // Cumulative frame-to-frame translations
          for (int k = 0; k < 3; k++) {
            trtot[k] += fabs(*(trframe.end() - 3 + k));
          }

          copy(tr.begin(), tr.end(), tr0.begin());

          // Rotations with respect to first frame
          ro = reg.GetRotate();

          // Frame-to-frame rotations
          for (int k = 0; k < 3; k++) {
            roframe.push_back(ro[k] - ro0[k]);
          }

          // Cumulative frame-to-frame rotations
          for (int k = 0; k < 3; k++) {
            rotot[k] += fabs(*(roframe.end() - 3 + k));
          }

          copy(ro.begin(), ro.end(), ro0.begin());

          xform.clear();

          nframe++;
        }
      }
    }

    infile.close();

    std::cout << "INFO: Processed transforms for " << nframe << " volumes"
              << std::endl;

    travg =
        sqrt(trtot[0] * trtot[0] + trtot[1] * trtot[1] + trtot[2] * trtot[2]) /
        nframe;
    roavg = (rotot[0] + rotot[1] + rotot[2]) / nframe;
  }

  if (inBvalFile != nullptr) { // Estimate within-volume motion
    int                                nx;
    int                                ny;
    int                                nz;
    int                                nd;
    int                                nxy;
    int                                nslice = 0;
    int                                nbad   = 0;
    float                              minvox;
    float                              b;
    std::vector<int>                   r;
    std::vector<int>                   r1;
    std::vector<int>::const_iterator   ir1;
    std::vector<float>                 bvals;
    std::vector<float>::const_iterator ibval;
    std::ifstream                      infile;

    // Read DWI volume series
    std::cout << "Loading DWI volume series from " << inDwiFile << std::endl;
    dwi = MRIread(inDwiFile);
    if (dwi == nullptr) {
      std::cout << "ERROR: Could not read " << inDwiFile << std::endl;
      exit(1);
    }

    nx = dwi->width;
    ny = dwi->height;
    nz = dwi->depth;
    nd = dwi->nframes;

    nxy    = nx * ny;
    minvox = 0.05 * nxy;

    nbadframe.resize(nd);
    fill(nbadframe.begin(), nbadframe.end(), 0);
    scoreframe.resize(nd);
    fill(scoreframe.begin(), scoreframe.end(), 0.0);

    // Read b-value table
    std::cout << "Loading b-value table from " << inBvalFile << std::endl;
    infile.open(inBvalFile, std::ios::in);
    if (!infile) {
      std::cout << "ERROR: Could not open " << inBvalFile << " for reading"
                << std::endl;
      exit(1);
    }

    while (infile >> b) {
      bvals.push_back(b);
    }

    infile.close();

    std::cout << "Computing within-volume head motion measures" << std::endl;

    // Find unique b-values
    std::set<float> blist(bvals.begin(), bvals.end());

    // Compare frames acquired with each b-value separately
    for (std::set<float>::const_iterator ib = blist.begin(); ib != blist.end();
         ib++) {
      const float thresh = T * exp(-(*ib) * D);
      auto        inbad  = nbadframe.begin();
      auto        iscore = scoreframe.begin();

      r1.clear();
      ibval = bvals.begin();

      for (int id = 0; id < nd; id++) {
        if (*ibval == *ib) {
          r.clear();

          // Find number of voxels above threshold for each slice in this frame
          for (int iz = 0; iz < nz; iz++) {
            int count = 0;

            for (int iy = 0; iy < ny; iy++) {
              for (int ix = 0; ix < nx; ix++) {
                if (MRIgetVoxVal(dwi, ix, iy, iz, id) > thresh) {
                  count++;
                }
              }
            }

            r.push_back(count);
          }

          if (r1.empty()) { // First frame with this b-value
            r1.insert(r1.begin(), r.begin(), r.end());
          }

          // Motion score (from Benner et al MRM 2011)
          ir1 = r1.begin();
          for (auto ir = r.begin(); ir < r.end(); ir++) {
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
          if (*inbad > 0) {
            *iscore /= *inbad;
          } else {
            *iscore = 1;
          }
        }

        ibval++;
        inbad++;
        iscore++;
      }
    }

    // Percentage of bad slices among all non-empty slices
    if (nslice > 0) {
      pbad = nbad / static_cast<float>(nslice) * 100;
    }

    // Average motion score of bad slices
    if (nbad > 0) {
      score /= nbad;
    } else {
      score = 1;
    }
  }

  // Write overall measures to file
  outfile.open(outFile, std::ios::out);
  outfile << "AvgTranslation AvgRotation PercentBadSlices AvgDropoutScore"
          << std::endl
          << travg << " " << roavg << " " << pbad << " " << score << std::endl;
  outfile.close();

  // Write frame-by-frame measures to file
  if (outFrameFile != nullptr) {
    std::vector<float>::const_iterator itr    = trframe.begin();
    std::vector<float>::const_iterator iro    = roframe.begin();
    std::vector<float>::const_iterator iscore = scoreframe.begin();

    if (trframe.empty()) {
      trframe.resize(nbadframe.size() * 3);
      fill(trframe.begin(), trframe.end(), 0.0);
    } else if (trframe.size() != nbadframe.size() * 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << trframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (roframe.empty()) {
      roframe.resize(nbadframe.size() * 3);
      fill(roframe.begin(), roframe.end(), 0.0);
    } else if (roframe.size() != nbadframe.size() * 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << roframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (nbadframe.empty()) {
      nbadframe.resize(trframe.size() / 3);
      fill(nbadframe.begin(), nbadframe.end(), 0);
    } else if (nbadframe.size() != trframe.size() / 3) {
      std::cout << "ERROR: inconsistent number of frames for "
                << "between-volume (" << trframe.size() / 3 << ") and "
                << "within-volume (" << nbadframe.size() << ") measures"
                << std::endl;
      exit(1);
    }

    if (scoreframe.empty()) {
      scoreframe.resize(trframe.size() / 3);
      fill(scoreframe.begin(), scoreframe.end(), 0.0);
    } else if (scoreframe.size() != trframe.size() / 3) {
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

    for (auto inbad = nbadframe.begin(); inbad < nbadframe.end(); inbad++) {

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
    } else if (strcmp(option, "--dwi") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inDwiFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--bval") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inBvalFile = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (strcmp(option, "--mat") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inMatFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--T") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%f", &T);
      nargsused = 1;
    } else if (strcmp(option, "--D") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%f", &D);
      nargsused = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outFile   = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--outf") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outFrameFile = fio_fullpath(pargv[0]);
      nargsused    = 1;
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
      << std::endl
      << "Arguments needed for within-volume motion measures" << std::endl
      << "(see Benner et al MRM 2011):" << std::endl
      << "   --dwi <file>:" << std::endl
      << "     Input DWI volume series, unprocessed" << std::endl
      << "   --bval <file>:" << std::endl
      << "     Input b-value table" << std::endl
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
  if (outFile == nullptr) {
    std::cout << "ERROR: must specify output file" << std::endl;
    exit(1);
  }
  if (((inDwiFile != nullptr) && (inBvalFile == nullptr)) ||
      ((inDwiFile == nullptr) && (inBvalFile != nullptr))) {
    std::cout << "ERROR: must specify both DWI and b-value files" << std::endl;
    exit(1);
  }
  if ((inBvalFile == nullptr) && (inMatFile == nullptr)) {
    std::cout << "ERROR: must specify inputs for between-volume and/or "
              << "within-volume motion measures" << std::endl;
    exit(1);
  }
  if (D < 0) {
    std::cout << "ERROR: diffusivity must be positive" << std::endl;
    exit(1);
  }
}

static void dump_options() {
  std::cout << std::endl
            << vcid << std::endl
            << "cwd " << cwd << std::endl
            << "cmdline " << cmdline << std::endl
            << "sysname  " << uts.sysname << std::endl
            << "hostname " << uts.nodename << std::endl
            << "machine  " << uts.machine << std::endl
            << "user     " << VERuser() << std::endl;

  std::cout << "Output motion measure file: " << outFile << std::endl;

  if (outFrameFile != nullptr) {
    std::cout << "Output frame-by-frame motion measure file: " << outFrameFile
              << std::endl;
  }

  if (inMatFile != nullptr) {
    std::cout << "Input transform file: " << inMatFile << std::endl;
  }

  if (inBvalFile != nullptr) {
    std::cout << "Input DWI file: " << inDwiFile << std::endl;
    std::cout << "Input b-value table: " << inBvalFile << std::endl;
    std::cout << "Low-b image intensity threshold: " << T << std::endl;
    std::cout << "Nominal diffusivity: " << D << std::endl;
  }
}
