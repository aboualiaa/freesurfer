/**
 * @file  dmri_trk2trk.cxx
 * @brief Transform streamlines in .trk file
 *
 * Transform streamlines in .trk file:
 * Apply affine or non-linear warp, inclusion or exclusion masks,
 * convert to text file or volume file, etc.
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2015/08/28 20:33:54 $
 *    $Revision: 1.21 $
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

#include "TrackIO.h"
#include "vial.h" // Needs to be included first because of CVS libs

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/utsname.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "cmdargs.h"
#include "diag.h"
#include "error.h"
#include "fio.h"
#include "mri.h"
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

static char vcid[]   = "";
const char *Progname = "dmri_trk2trk";

int doInvNonlin = 0, doFill = 0, doMean = 0, doNth = 0, strNum = -1,
    lengthMin = -1, lengthMax = -1;
unsigned int nTract = 0;
char *inDir = nullptr, *outDir = nullptr, *inRefFile = nullptr,
     *outRefFile = nullptr, *affineXfmFile = nullptr, *nonlinXfmFile = nullptr;
vector<char *> inTrkList, inAscList, outTrkList, outAscList, outVolList,
    incMaskList, excMaskList;
vector<MRI *> incMask, excMask;

struct utsname uts;
char *cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  int cputime;
  char fname[PATH_MAX];
  char outorient[4];
  vector<float> point(3);
  vector<float> step(3, 0);
  MATRIX *outv2r;
  MRI *inref  = 0;
  MRI *outref = 0;
  MRI *outvol = 0;
  AffineReg affinereg;
#ifndef NO_CVS_UP_IN_HERE
  NonlinReg nonlinreg;
#endif

  /* rkt: check for and handle version tag */
  nargs = handle_version_option(argc, argv, vcid, "$Name:  $");
  if ((nargs != 0) && argc - nargs == 1) {
    exit(0);
  }
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
  inref  = MRIread(inRefFile);
  outref = MRIread(outRefFile);

  if (!outVolList.empty()) {
    outvol = MRIclone(outref, nullptr);
  }

  // Output space orientation information
  outv2r = MRIgetVoxelToRasXform(outref);
  MRIdircosToOrientationString(outref, outorient);

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

  // Read inclusion masks
  for (auto imask = incMaskList.begin(); imask < incMaskList.end(); imask++) {
    incMask.push_back(MRIread(*imask));
  }

  // Read exclusion masks
  for (auto imask = excMaskList.begin(); imask < excMaskList.end(); imask++) {
    excMask.push_back(MRIread(*imask));
  }

  for (unsigned int itract = 0; itract < nTract; itract++) {
    int npts;
    int nstr = 0;
    CTrackReader trkreader;
    TRACK_HEADER trkheadin;
    vector<vector<float>> streamlines;

    cout << "Processing input file " << itract + 1 << " of " << nTract << "..."
         << endl;
    cputimer.reset();

    if (!inTrkList.empty()) { // Read streamlines from .trk file
      if (inDir != nullptr) {
        sprintf(fname, "%s/%s", inDir, inTrkList[itract]);
      } else {
        strcpy(fname, inTrkList[itract]);
      }

      if (!trkreader.Open(fname, &trkheadin)) {
        cout << "ERROR: Cannot open input file " << fname << endl;
        cout << "ERROR: " << trkreader.GetLastErrorMessage() << endl;
        exit(1);
      }

      while (trkreader.GetNextPointCount(&npts)) {
        const int veclen = npts * 3;
        float *iraw;
        float *rawpts = new float[veclen];
        vector<float> newpts(veclen);

        // Read a streamline from input file
        trkreader.GetNextTrackData(npts, rawpts);

        if (((doNth != 0) && nstr != strNum) ||
            (lengthMin > -1 && npts <= lengthMin) ||
            (lengthMax > -1 && npts >= lengthMax)) {
          delete[] rawpts;
          nstr++;
          continue;
        }

        iraw = rawpts;

        // Divide by input voxel size and make 0-based to get voxel coords
        for (auto ipt = newpts.begin(); ipt < newpts.end(); ipt += 3) {
          for (int k = 0; k < 3; k++) {
            ipt[k] = *iraw / trkheadin.voxel_size[k] - .5;
            iraw++;
          }
        }

        delete[] rawpts;
        streamlines.push_back(newpts);

        nstr++;
      }
    } else if (!inAscList.empty()) { // Read streamlines from text file
      string ptline;
      ifstream infile;
      vector<float> newpts;

      if (inDir != nullptr) {
        sprintf(fname, "%s/%s", inDir, inAscList[itract]);
      } else {
        strcpy(fname, inAscList[itract]);
      }

      infile.open(fname, ios::in);
      if (!infile) {
        cout << "Error: Cannot open input file " << fname << endl;
        exit(1);
      }

      while (getline(infile, ptline)) {
        float val;
        istringstream linestr(ptline);
        vector<float> point;

        while (linestr >> val) {
          point.push_back(val);
        }

        if (point.empty()) { // Empty line marks end of streamline
          if (((doNth == 0) || nstr == strNum) &&
              (lengthMin == -1 ||
               static_cast<int>(newpts.size()) / 3 > lengthMin) &&
              (lengthMax == -1 ||
               static_cast<int>(newpts.size()) / 3 < lengthMax)) {
            streamlines.push_back(newpts);
          }

          newpts.clear();
          nstr++;
        } else if (point.size() != 3) {
          cout << "ERROR: Unexpected number of entries in a line ("
               << point.size() << ") in file " << fname << endl;
          exit(1);
        } else {
          newpts.insert(newpts.end(), point.begin(), point.end());
        }
      }

      infile.close();
    }

    nstr = streamlines.size();

    for (int kstr = nstr - 1; kstr >= 0; kstr--) {
      vector<float> newpts;

      for (auto ipt = streamlines[kstr].begin(); ipt < streamlines[kstr].end();
           ipt += 3) {
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

      for (auto ipt = streamlines[kstr].begin(); ipt < streamlines[kstr].end();
           ipt += 3) {
        float dmax = 1; // This will not remove duplicate points

        if ((doFill != 0) && ipt < streamlines[kstr].end() - 3) {
          // Calculate step for filling in gap between points
          // Gaps could result when mapping to a higher-resolution space
          for (int k = 0; k < 3; k++) {
            float dist = ipt[k + 3] - ipt[k];

            step[k] = dist;
            dist    = fabs(dist);

            if (dist > dmax) {
              dmax = dist;
            }
          }

          if (dmax > 0) {
            for (int k = 0; k < 3; k++) {
              step[k] /= dmax;
            }
          }
        }

        copy(ipt, ipt + 3, point.begin());

        for (int istep = static_cast<int>(round(dmax)); istep > 0; istep--) {
          newpts.insert(newpts.end(), point.begin(), point.end());

          for (int k = 0; k < 3; k++) {
            point[k] += step[k];
          }
        }
      }

      streamlines[kstr].resize(newpts.size());
      copy(newpts.begin(), newpts.end(), streamlines[kstr].begin());
    }

    // Apply inclusion masks
    nstr = streamlines.size();

    for (int kstr = nstr - 1; kstr >= 0; kstr--) {
      bool dokeep = true;

      // There must be at least one point that intersects each inclusion mask
      for (auto imask = incMask.begin(); imask < incMask.end(); imask++) {
        dokeep = false;

        for (auto ipt = streamlines[kstr].begin();
             ipt < streamlines[kstr].end(); ipt += 3) {
          int ix = (int)round(ipt[0]);
          int iy = (int)round(ipt[1]);
          int iz = (int)round(ipt[2]);

          if (ix < 0) {
            ix = 0;
          }
          if (ix >= (*imask)->width) {
            ix = (*imask)->width - 1;
          }
          if (iy < 0) {
            iy = 0;
          }
          if (iy >= (*imask)->height) {
            iy = (*imask)->height - 1;
          }
          if (iz < 0) {
            iz = 0;
          }
          if (iz >= (*imask)->depth) {
            iz = (*imask)->depth - 1;
          }

          if (MRIgetVoxVal(*imask, ix, iy, iz, 0) > 0) {
            dokeep = true;
            break;
          }
        }

        if (!dokeep) {
          break;
        }
      }

      if (!dokeep) {
        streamlines.erase(streamlines.begin() + kstr);
        continue;
      }

      // There must be no point that intersects any exclusion mask
      for (auto ipt = streamlines[kstr].begin(); ipt < streamlines[kstr].end();
           ipt += 3) {
        for (auto imask = excMask.begin(); imask < excMask.end(); imask++) {
          int ix = (int)round(ipt[0]);
          int iy = (int)round(ipt[1]);
          int iz = (int)round(ipt[2]);

          if (ix < 0) {
            ix = 0;
          }
          if (ix >= (*imask)->width) {
            ix = (*imask)->width - 1;
          }
          if (iy < 0) {
            iy = 0;
          }
          if (iy >= (*imask)->height) {
            iy = (*imask)->height - 1;
          }
          if (iz < 0) {
            iz = 0;
          }
          if (iz >= (*imask)->depth) {
            iz = (*imask)->depth - 1;
          }

          if (MRIgetVoxVal(*imask, ix, iy, iz, 0) > 0) {
            dokeep = false;
            break;
          }
        }

        if (!dokeep) {
          break;
        }
      }

      if (!dokeep) {
        streamlines.erase(streamlines.begin() + kstr);
      }
    }

    if ((doMean != 0) && !streamlines.empty()) {
      unsigned int nstr = streamlines.size();
      unsigned int lmin;
      unsigned int kmax;
      unsigned int nstrout;
      unsigned int kstrmean = 0;
      float dmin            = numeric_limits<float>::infinity();
      vector<bool> isout(nstr);
      vector<unsigned int> lengths(nstr);
      vector<float> steps(nstr);
      vector<float> strmean;
      vector<float> strstd;
      vector<float> strU;
      vector<float> strL;
      vector<bool>::iterator iout;
      vector<unsigned int>::iterator ilen;
      vector<float>::iterator istep;
      vector<float>::iterator imean;
      vector<float>::iterator istd;
      vector<float>::iterator iupper;
      vector<float>::iterator ilower;

      // Find the minimum streamline length
      ilen = lengths.begin();

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        *ilen = istr->size() / 3;
        ilen++;
      }

      lmin = *min_element(lengths.begin(), lengths.end());
      kmax = lmin - 1;

      // Set the step size for each streamline to have equal number of steps
      istep = steps.begin();

      for (ilen = lengths.begin(); ilen < lengths.end(); ilen++) {
        *istep = (*ilen - 1) / static_cast<float>(kmax);
        istep++;
      }

      lengths.clear();

      // Compute mean and standard deviation of point coordinates at each step
      strmean.resize(lmin * 3);
      fill(strmean.begin(), strmean.end(), 0.0);
      strstd.resize(strmean.size());
      fill(strstd.begin(), strstd.end(), 0.0);

      istep = steps.begin();

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        imean = strmean.begin();
        istd  = strstd.begin();

        for (unsigned int kpt = 0; kpt < kmax; kpt++) {
          const auto idx = static_cast<unsigned int>(round(kpt * (*istep)));
          auto ipt       = istr->begin() + idx * 3;

          if (ipt > istr->end() - 3) {
            ipt = istr->end() - 3;
          }

          for (int k = 0; k < 3; k++) {
            imean[k] += ipt[k];
            istd[k] += ipt[k] * ipt[k];
          }

          imean += 3;
          istd += 3;
        }

        istep++;
      }

      // Compute upper and lower limit for flagging a point as an outlier
      strU.resize(strmean.size());
      strL.resize(strmean.size());

      istd   = strstd.begin();
      iupper = strU.begin();
      ilower = strL.begin();

      for (imean = strmean.begin(); imean < strmean.end(); imean++) {
        float dout;

        *imean /= nstr;

        if (nstr > 1) {
          *istd = sqrt((*istd - nstr * (*imean) * (*imean)) / (nstr - 1));
        } else {
          *istd = 0;
        }

        if (imean == strmean.begin() || imean == strmean.end() - 1) {
          dout = *istd;
        } else {
          dout = 2 * (*istd);
        }

        *iupper = *imean + dout;
        *ilower = *imean - dout;

        istd++;
        iupper++;
        ilower++;
      }

      // Flag streamlines with at least one outlier point
      fill(isout.begin(), isout.end(), false);

      iout  = isout.begin();
      istep = steps.begin();

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        iupper = strU.begin();
        ilower = strL.begin();

        for (unsigned int kpt = 0; kpt < kmax; kpt++) {
          const auto idx = static_cast<unsigned int>(round(kpt * (*istep)));
          auto ipt       = istr->begin() + idx * 3;

          for (int k = 0; k < 3; k++) {
            if (*ipt > *iupper || *ipt < *ilower) {
              *iout = true;
              break;
            }

            ipt++;
            iupper++;
            ilower++;
          }

          if (*iout) {
            break;
          }
        }

        iout++;
        istep++;
      }

      nstrout = count(isout.begin(), isout.end(), true);

      cout << "INFO: Found " << nstrout
           << " streamlines with at least one outlier coordinate" << endl;

      if (nstrout == nstr) {
        cout << "INFO: Turning off outlier checks" << endl;

        fill(isout.begin(), isout.end(), false);
      }

      // Find the non-outlier streamline that is closest to the mean streamline
      iout  = isout.begin();
      istep = steps.begin();

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        if (!*iout) {
          float dist = 0;

          imean = strmean.begin();
          istd  = strstd.begin();

          for (unsigned int kpt = 0; kpt < kmax; kpt++) {
            const auto idx = static_cast<unsigned int>(round(kpt * (*istep)));
            auto ipt       = istr->begin() + idx * 3;

            const float dx = ipt[0] - imean[0];
            const float dy = ipt[1] - imean[1];
            const float dz = ipt[2] - imean[2];

            dist += sqrt(dx * dx + dy * dy + dz * dz);

            imean += 3;
            istd += 3;
          }

          if (dist < dmin) {
            dmin     = dist;
            kstrmean = istr - streamlines.begin();
          }
        }

        iout++;
        istep++;
      }

      cout << "INFO: Streamline closest to the mean is " << kstrmean << endl;

      // Keep only the chosen streamline for writing to disk
      streamlines.erase(streamlines.begin(), streamlines.begin() + kstrmean);
      streamlines.erase(streamlines.begin() + 1, streamlines.end());
    }

    // Write transformed streamlines to volume
    if (!outVolList.empty()) {
      if (outDir != nullptr) {
        sprintf(fname, "%s/%s", outDir, outVolList[itract]);
      } else {
        strcpy(fname, outVolList[itract]);
      }

      MRIclear(outvol);

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        for (auto ipt = istr->begin(); ipt < istr->end(); ipt += 3) {
          int ix = (int)round(ipt[0]);
          int iy = (int)round(ipt[1]);
          int iz = (int)round(ipt[2]);

          if (ix < 0) {
            ix = 0;
          }
          if (ix >= outvol->width) {
            ix = outvol->width - 1;
          }
          if (iy < 0) {
            iy = 0;
          }
          if (iy >= outvol->height) {
            iy = outvol->height - 1;
          }
          if (iz < 0) {
            iz = 0;
          }
          if (iz >= outvol->depth) {
            iz = outvol->depth - 1;
          }

          MRIsetVoxVal(outvol, ix, iy, iz, 0,
                       MRIgetVoxVal(outvol, ix, iy, iz, 0) + 1);
        }
      }

      MRIwrite(outvol, fname);
    }

    // Write transformed streamlines to text file
    if (!outAscList.empty()) {
      ofstream outfile;

      if (outDir != nullptr) {
        sprintf(fname, "%s/%s", outDir, outAscList[itract]);
      } else {
        strcpy(fname, outAscList[itract]);
      }

      outfile.open(fname, ios::out);
      if (!outfile) {
        cout << "ERROR: Could not open " << fname << " for writing" << endl;
        exit(1);
      }

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        for (auto ipt = istr->begin(); ipt < istr->end(); ipt += 3) {
          outfile << static_cast<int>(round(ipt[0])) << " "
                  << static_cast<int>(round(ipt[1])) << " "
                  << static_cast<int>(round(ipt[2])) << endl;
        }

        outfile << endl;
      }

      outfile.close();
    }

    // Write transformed streamlines to .trk file
    if (!outTrkList.empty()) {
      CTrackWriter trkwriter;
      TRACK_HEADER trkheadout;

      // Set output .trk header
      if (inTrkList.empty()) {
        trkheadout.Initialize();

        trkheadout.origin[0] = 0;
        trkheadout.origin[1] = 0;
        trkheadout.origin[2] = 0;
      } else {
        trkheadout = trkheadin;
      }

      trkheadout.voxel_size[0] = outref->xsize;
      trkheadout.voxel_size[1] = outref->ysize;
      trkheadout.voxel_size[2] = outref->zsize;

      trkheadout.dim[0] = outref->width;
      trkheadout.dim[1] = outref->height;
      trkheadout.dim[2] = outref->depth;

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          trkheadout.vox_to_ras[i][j] = outv2r->rptr[i + 1][j + 1];
        }
      }

      strcpy(trkheadout.voxel_order, outorient);

      // Find patient-to-scanner coordinate transform:
      // Take x and y vectors from vox2RAS matrix, convert to LPS,
      // divide by voxel size
      trkheadout.image_orientation_patient[0] =
          -trkheadout.vox_to_ras[0][0] / trkheadout.voxel_size[0];
      trkheadout.image_orientation_patient[1] =
          -trkheadout.vox_to_ras[1][0] / trkheadout.voxel_size[0];
      trkheadout.image_orientation_patient[2] =
          trkheadout.vox_to_ras[2][0] / trkheadout.voxel_size[0];
      trkheadout.image_orientation_patient[3] =
          -trkheadout.vox_to_ras[0][1] / trkheadout.voxel_size[1];
      trkheadout.image_orientation_patient[4] =
          -trkheadout.vox_to_ras[1][1] / trkheadout.voxel_size[1];
      trkheadout.image_orientation_patient[5] =
          trkheadout.vox_to_ras[2][1] / trkheadout.voxel_size[1];

      trkheadout.n_count = static_cast<int>(streamlines.size());

      // Open output .trk file
      if (outDir != nullptr) {
        sprintf(fname, "%s/%s", outDir, outTrkList[itract]);
      } else {
        strcpy(fname, outTrkList[itract]);
      }

      if (!trkwriter.Initialize(fname, trkheadout)) {
        cout << "ERROR: Cannot open output file " << fname << endl;
        cout << "ERROR: " << trkwriter.GetLastErrorMessage() << endl;
        exit(1);
      }

      for (auto istr = streamlines.begin(); istr < streamlines.end(); istr++) {
        // Make .5-based and multiply back by output voxel size
        for (auto ipt = istr->begin(); ipt < istr->end(); ipt += 3) {
          for (int k = 0; k < 3; k++) {
            ipt[k] = (ipt[k] + .5) * trkheadout.voxel_size[k];
          }
        }

        trkwriter.WriteNextTrack(istr->size() / 3, &(istr->at(0)));
      }

      trkwriter.Close();
    }

    cputime = cputimer.milliseconds();
    cout << "Done in " << cputime / 1000.0 << " sec." << endl;
  }

  MatrixFree(&outv2r);
  MRIfree(&inref);
  MRIfree(&outref);
  if (!outVolList.empty()) {
    MRIfree(&outvol);
  }
  for (auto imask = incMask.begin(); imask < incMask.end(); imask++) {
    MRIfree(&(*imask));
  }
  for (auto imask = excMask.begin(); imask < excMask.end(); imask++) {
    MRIfree(&(*imask));
  }

  cout << "dmri_trk2trk done" << endl;
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
      inDir     = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--in") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        inTrkList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--inasc") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        inAscList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--outdir") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outDir    = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outTrkList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--outasc") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outAscList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--outvol") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        outVolList.push_back(pargv[nargsused]);
        nargsused++;
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
      nargsused  = 1;
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
    } else if (strcasecmp(option, "--invnl") == 0) {
      doInvNonlin = 1;
    } else if (strcasecmp(option, "--fill") == 0) {
      doFill = 1;
    } else if (strcmp(option, "--imask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        incMaskList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcmp(option, "--emask") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      nargsused = 0;
      while (nargsused < nargc && (strncmp(pargv[nargsused], "--", 2) != 0)) {
        excMaskList.push_back(pargv[nargsused]);
        nargsused++;
      }
    } else if (strcasecmp(option, "--lmin") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%d", &lengthMin);
      nargsused = 1;
    } else if (strcasecmp(option, "--lmax") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%d", &lengthMax);
      nargsused = 1;
    } else if (strcasecmp(option, "--mean") == 0) {
      doMean = 1;
    } else if (strcasecmp(option, "--nth") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%d", &strNum);
      nargsused = 1;
      doNth     = 1;
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
  cout
      << endl
      << "USAGE: " << Progname << endl
      << endl
      << "Basic inputs" << endl
      << "   --in <file> [...]:" << endl
      << "     Input .trk file(s)" << endl
      << "   --inasc <file> [...]:" << endl
      << "     Input ASCII plain text file(s), as an alternative to .trk"
      << endl
      << "     Each line in the text file must have the voxel coordinates of a "
         "single point of a streamline, with empty lines between streamlines)"
      << endl
      << "   --indir <dir>:" << endl
      << "     Input directory (optional)" << endl
      << "     If specified, names of input .trk files are relative to this"
      << endl
      << "   --out <file> [...]:" << endl
      << "     Output .trk file(s), as many as inputs" << endl
      << "   --outasc <file> [...]:" << endl
      << "     Output ASCII plain text file(s), as many as inputs" << endl
      << "   --outvol <file> [...]:" << endl
      << "     Output volume(s), as many as inputs" << endl
      << "   --outdir <dir>:" << endl
      << "     Output directory (optional)" << endl
      << "     If specified, names of output .trk files and volumes are "
         "relative to this)"
      << endl
      << "   --inref <file>:" << endl
      << "     Input reference volume" << endl
      << "   --outref <file>:" << endl
      << "     Output reference volume" << endl
      << "   --reg <file>:" << endl
      << "     Affine registration (.mat), applied first" << endl
      << "   --regnl <file>:" << endl
      << "     Nonlinear registration (.m3z), applied second" << endl
      << "   --invnl:" << endl
      << "     Apply inverse of nonlinear warp (with --regnl, default: no)"
      << endl
      << "   --fill:" << endl
      << "     Fill gaps b/w mapped points by linear interpolation" << endl
      << "     (Default: don't fill)" << endl
      << "   --imask <file> [...]:" << endl
      << "     Inclusion mask(s), applied to all input .trk files" << endl
      << "   --emask <file> [...]:" << endl
      << "     Exclusion mask(s), applied to all input .trk files" << endl
      << "   --lmin <num>:" << endl
      << "     Only save streamlines with length greater than this number"
      << endl
      << "   --lmax <num>:" << endl
      << "     Only save streamlines with length smaller than this number"
      << endl
      << "   --mean:" << endl
      << "     Only save the mean streamline (Default: save all)" << endl
      << "   --nth <num>:" << endl
      << "     Only save the n-th (0-based) streamline (Default: save all)"
      << endl
      << endl
      << "Other options" << endl
      << "   --debug:     turn on debugging" << endl
      << "   --checkopts: don't run anything, just check options and exit"
      << endl
      << "   --help:      print out information on how to use this program"
      << endl
      << "   --version:   print out version and exit" << endl
      << endl
      << "Order of operations (all optional):" << endl
      << "   1. Keep n-th streamline only" << endl
      << "   2. Apply streamline length threshold(s)" << endl
      << "   3. Apply affine transform" << endl
      << "   4. Apply non-linear transform" << endl
      << "   5. Apply inclusion mask(s)" << endl
      << "   6. Apply exclusion mask(s)" << endl
      << "   7. Find mean streamline" << endl
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
  if (inTrkList.empty() && inAscList.empty()) {
    cout << "ERROR: must specify input .trk or text file(s)" << endl;
    exit(1);
  }
  if (!inTrkList.empty() && !inAscList.empty()) {
    cout << "ERROR: cannot use both --in and --inasc at the same time" << endl;
    exit(1);
  }

  nTract = inTrkList.size() + inAscList.size();

  if (outTrkList.empty() && outAscList.empty() && outVolList.empty()) {
    cout << "ERROR: must specify output .trk or text or volume file(s)" << endl;
    exit(1);
  }
  if (!outTrkList.empty() && outTrkList.size() != nTract) {
    cout << "ERROR: must specify as many output .trk files as input files"
         << endl;
    exit(1);
  }
  if (!outAscList.empty() && outAscList.size() != nTract) {
    cout << "ERROR: must specify as many output text files as input .trk files"
         << endl;
    exit(1);
  }
  if (!outVolList.empty() && outVolList.size() != nTract) {
    cout << "ERROR: must specify as many output volumes as input .trk files"
         << endl;
    exit(1);
  }
  if (inRefFile == nullptr) {
    cout << "ERROR: must specify input reference volume" << endl;
    exit(1);
  }
  if (outRefFile == nullptr) {
    cout << "ERROR: must specify output reference volume" << endl;
    exit(1);
  }
  if ((doMean != 0) && (doNth != 0)) {
    cout << "ERROR: cannot use both --mean and --nth" << endl;
    exit(1);
  }
}

/* --------------------------------------------- */
static void dump_options(FILE *fp) {
  cout << endl
       << vcid << endl
       << "cwd " << cwd << endl
       << "cmdline " << cmdline << endl
       << "sysname  " << uts.sysname << endl
       << "hostname " << uts.nodename << endl
       << "machine  " << uts.machine << endl
       << "user     " << VERuser() << endl;

  if (inDir != nullptr) {
    cout << "Input directory: " << inDir << endl;
  }
  if (!inTrkList.empty()) {
    cout << "Input .trk files:";
    for (auto istr = inTrkList.begin(); istr < inTrkList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (!inAscList.empty()) {
    cout << "Input text files:";
    for (auto istr = inAscList.begin(); istr < inAscList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (outDir != nullptr) {
    cout << "Output directory: " << outDir << endl;
  }
  if (!outTrkList.empty()) {
    cout << "Output .trk files:";
    for (auto istr = outTrkList.begin(); istr < outTrkList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (!outAscList.empty()) {
    cout << "Output text files:";
    for (auto istr = outAscList.begin(); istr < outAscList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (!outVolList.empty()) {
    cout << "Output volumes:";
    for (auto istr = outVolList.begin(); istr < outVolList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (!incMaskList.empty()) {
    cout << "Inclusion mask volumes:";
    for (auto istr = incMaskList.begin(); istr < incMaskList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (!excMaskList.empty()) {
    cout << "Exclusion mask volumes:";
    for (auto istr = excMaskList.begin(); istr < excMaskList.end(); istr++) {
      cout << " " << *istr;
    }
    cout << endl;
  }
  if (lengthMin > -1) {
    cout << "Lower length threshold: " << lengthMin << endl;
  }
  if (lengthMax > -1) {
    cout << "Upper length threshold: " << lengthMax << endl;
  }
  cout << "Input reference: " << inRefFile << endl;
  cout << "Output reference: " << outRefFile << endl;
  if (affineXfmFile != nullptr) {
    cout << "Affine registration: " << affineXfmFile << endl;
  }
  if (nonlinXfmFile != nullptr) {
    cout << "Nonlinear registration: " << nonlinXfmFile << endl;
    cout << "Invert nonlinear morph: " << doInvNonlin << endl;
  }
  cout << "Fill gaps between points: " << doFill << endl;
  if (doMean != 0) {
    cout << "Saving mean streamline" << endl;
  } else if (doNth != 0) {
    cout << "Saving single streamline: " << strNum << endl;
  } else {
    cout << "Saving all streamlines" << endl;
  }
}
