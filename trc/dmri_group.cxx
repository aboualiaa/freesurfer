/**
 * @file  dmri_group.cxx
 * @brief Combine path measures from multiple subjects
 *
 * Combine path measures from multiple subjects
 */
/*
 * Original Author: Anastasia Yendiki
 * CVS Revision Info:
 *    $Author: ayendiki $
 *    $Date: 2016/12/19 17:09:04 $
 *    $Revision: 1.11 $
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
const char *Progname = "dmri_group";

int nSection = 0;

char *inListFile = nullptr, *outRefFile = nullptr, *outBase = nullptr;

struct utsname uts;
char *cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int nargs;
  int cputime;
  unsigned int nmeas;
  unsigned int npt;
  unsigned int narc;
  unsigned int nsubjmin;
  unsigned int ntot;
  float distmin;
  float darc;
  float arcmin;
  float arcmax;
  float arc1m;
  float arc2m;
  float arc1s;
  float arc2s;
  float lthresh1;
  float lthresh2;
  float uthresh1;
  float uthresh2;
  string listline;
  string filename;
  vector<bool>::const_iterator iout;
  vector<unsigned int>::const_iterator insubj;
  vector<float>::const_iterator icenter;
  vector<float>::const_iterator iarc1;
  vector<float>::const_iterator iarc2;
  vector<vector<unsigned int>>::const_iterator iallk;
  vector<vector<float>>::const_iterator itemplate;
  vector<vector<float>>::const_iterator iallm;
  vector<vector<float>>::const_iterator iallp;
  vector<vector<float>>::iterator ialla;
  vector<bool> isout;
  vector<unsigned int> lengths;
  vector<unsigned int> nsubj;
  vector<float> meanpath;
  vector<float> arcend1;
  vector<float> arcend2;
  vector<string> subjlist;
  vector<string> measlist;
  vector<vector<unsigned int>> allknots;
  vector<vector<float>> allarc;
  vector<vector<float>> allpaths;
  vector<vector<float>> allmeas;
  vector<vector<float>> allmeasint;
  vector<vector<float>> allmeassec;
  ifstream listfile;
  ofstream pathfile;
  ofstream pathrasfile;
  MATRIX *outv2r;
  MRI *outref = nullptr;

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

  cputimer.reset();

  // Read output reference volume
  if (outRefFile != nullptr) {
    cout << "Loading output reference volume from " << outRefFile << endl;
    outref = MRIread(outRefFile);
    if (outref == nullptr) {
      cout << "ERROR: Could not read " << outRefFile << endl;
      exit(1);
    }

    outv2r = MRIgetVoxelToRasXform(outref);
  }

  // Read list of inputs
  cout << "Loading list of inputs from " << inListFile << endl;
  listfile.open(inListFile, ios::in);
  if (!listfile) {
    cout << "ERROR: Could not open " << inListFile << " for reading" << endl;
    exit(1);
  }

  while (getline(listfile, listline)) {
    string measline;
    string subjid;
    vector<unsigned int> knots;
    vector<float> arc;
    vector<float> path;
    vector<float> meas;
    vector<string> inputs;
    ifstream infile;
    istringstream liststr(listline);
    MRI *inref = nullptr;
    AffineReg affinereg;
#ifndef NO_CVS_UP_IN_HERE
    NonlinReg nonlinreg;
#endif

    while (liststr >> filename) {
      inputs.push_back(filename);
    }

    if (inputs.empty()) {
      continue;
    }

    // First input on each line is the path directory
    filename = inputs[0] + "/pathstats.byvoxel.txt";

    cout << "Loading measures along the path from " << filename << endl;
    infile.open(filename.c_str(), ios::in);
    if (!infile) {
      cout << "WARN: Could not open " << filename << endl
           << "WARN: Skipping ahead" << endl;
      continue;
    }

    // Second input on each line is an input reference volume
    if (inputs.size() > 1) {
      // Read reference volumes
      cout << "Loading input reference volume from " << inputs[1] << endl;
      inref = MRIread(inputs[1].c_str());
      if (inref == nullptr) {
        cout << "ERROR: Could not read " << inputs[1] << endl;
        exit(1);
      }
    }

    // Third input on each line is an affine transform
    // Fourth input on each line is a nonlinear transform
#ifndef NO_CVS_UP_IN_HERE
    if (inputs.size() > 3) {
      affinereg.ReadXfm(inputs[2].c_str(), inref, nullptr);
      nonlinreg.ReadXfm(inputs[3].c_str(), outref);
    } else
#endif
        if (inputs.size() > 2) {
      affinereg.ReadXfm(inputs[2].c_str(), inref, outref);
    }

    // Read measures along the path
    while (getline(infile, measline)) {
      if (measline.substr(0, 1).compare("#") == 0) { // Comment line
        string word;
        istringstream linestr(measline);

        linestr >> word;
        linestr >> word;

        if (word.compare("subjectname") == 0) {
          linestr >> subjid;
        }
      } else if (measline.substr(0, 1).compare("x") == 0) { // Header line
        string word;
        istringstream linestr(measline);

        // The first three column headers are "x", "y", "z"
        linestr >> word;
        linestr >> word;
        linestr >> word;

        // The remaining column headers are the names of the measures
        measlist.clear();
        while (linestr >> word) {
          measlist.push_back(word);
        }
      } else { // Value line
        float val;
        vector<float> point;
        vector<float> valpoint;
        istringstream linestr(measline);

        // The first three columns are the x, y, z coordinates of this point
        linestr >> val;
        point.push_back(val);
        linestr >> val;
        point.push_back(val);
        linestr >> val;
        point.push_back(val);

        // Apply affine transform
        if (!affinereg.IsEmpty()) {
          affinereg.ApplyXfm(point, point.begin());
        }

#ifndef NO_CVS_UP_IN_HERE
        // Apply nonlinear transform
        if (!nonlinreg.IsEmpty()) {
          nonlinreg.ApplyXfm(point, point.begin());
        }
#endif

        // Add length of new path segment to arc lengths
        if (path.empty()) {
          arc.push_back(0.0);
        } else {
          const float dx = *(path.end() - 3) - point[0];
          const float dy = *(path.end() - 2) - point[1];
          const float dz = *(path.end() - 1) - point[2];
          const float seglen = sqrt(dx * dx + dy * dy + dz * dz);

          arc.push_back(*(arc.end() - 1) + seglen);
        }

        // Coordinates of new path point
        path.insert(path.end(), point.begin(), point.end());

        // The remaining columns are the values of the measures at this point
        while (linestr >> val) {
          meas.push_back(val);
        }
      }
    }

    infile.close();

    if (inref != nullptr) {
      MRIfree(&inref);
    }

    if (arc.empty()) {
      continue;
    }

    subjlist.push_back(subjid);
    lengths.push_back(arc.size());
    allarc.push_back(arc);
    allpaths.push_back(path);
    allmeas.push_back(meas);

    // Start point of the path
    knots.push_back(0);

    // Points that are a quarter of the way or half way along the path
    for (int k = 1; k < 4; k++) {
      const float arcpart = *(arc.end() - 1) * 0.25 * k;
      float dmin = numeric_limits<float>::infinity();
      auto imin = arc.begin();

      for (auto iarc = arc.begin(); iarc < arc.end(); iarc++) {
        const float darc = fabs(arcpart - *iarc);

        if (darc < dmin) {
          imin = iarc;
          dmin = darc;
        } else {
          break;
        }
      }

      knots.push_back(imin - arc.begin());
    }

    // End point of the path
    knots.push_back(arc.size() - 1);

    allknots.push_back(knots);
  }

  nmeas = measlist.size();

  // Find the most representative path
  iallk = allknots.begin();
  itemplate = allpaths.begin();
  icenter = itemplate->begin() + (allknots[0][2] * 3);
  distmin = numeric_limits<float>::infinity();

  for (auto iallp = allpaths.begin(); iallp < allpaths.end(); iallp++) {
    float dist = 0.0;
    auto jallk = allknots.begin();

    for (auto jallp = allpaths.begin(); jallp < allpaths.end(); jallp++) {
      if (jallp != iallp) {
        auto iknot = iallk->begin();

        for (auto jknot = jallk->begin(); jknot < jallk->end(); jknot++) {
          const unsigned int ioffset = (*iknot) * 3;
          const unsigned int joffset = (*jknot) * 3;
          const float dx = iallp->at(ioffset) - jallp->at(joffset);
          const float dy = iallp->at(ioffset + 1) - jallp->at(joffset + 1);
          const float dz = iallp->at(ioffset + 2) - jallp->at(joffset + 2);

          dist += sqrt(dx * dx + dy * dy + dz * dz);

          iknot++;
        }
      }

      jallk++;
    }

    if (dist < distmin) {
      itemplate = iallp;                             // Most representative path
      icenter = iallp->begin() + (iallk->at(2) * 3); // Mid-point on path
      distmin = dist;
    }

    iallk++;
  }

  // Choose sampling interval for measures along the path
  darc = *(allarc[itemplate - allpaths.begin()].end() - 1) /
         lengths[itemplate - allpaths.begin()];

  cout << "INFO: Sampling interval along path is ";
  if (outref != nullptr) {
    cout << darc * outref->xsize << " mm" << endl;
  } else {
    cout << darc << " voxels" << endl;
  }

  if (0) {
    // Write points of most representative path to file as RAS coords
    filename = string(outBase) + ".median.txt";

    cout << "Writing median path to " << filename << endl;
    pathrasfile.open(filename.c_str(), ios::out);

    pathrasfile << "#!ascii label, vox2ras=scanner" << endl
                << itemplate->size() / 3 << endl;

    npt = 1;

    for (auto ipt = itemplate->begin(); ipt < itemplate->end(); ipt += 3) {
      pathrasfile << npt;

      for (int k = 1; k < 4; k++) { // Transform from voxel to RAS coords
        pathrasfile << " "
                    << ipt[0] * outv2r->rptr[k][1] +
                           ipt[1] * outv2r->rptr[k][2] +
                           ipt[2] * outv2r->rptr[k][3] + outv2r->rptr[k][4];
      }

      pathrasfile << " 0" << endl;

      npt++;
    }

    pathrasfile.close();
  }

  // Reparameterize the arc length on each path
  ialla = allarc.begin();

  for (auto iallp = allpaths.begin(); iallp < allpaths.end(); iallp++) {
    float dmin = numeric_limits<float>::infinity();
    float arc0 = 0;
    auto iarc = ialla->begin();

    // Find the closest point to the mid-point of the most representative path
    for (auto ipath = iallp->begin(); ipath < iallp->end(); ipath += 3) {
      const float dx = ipath[0] - icenter[0];
      const float dy = ipath[1] - icenter[1];
      const float dz = ipath[2] - icenter[2];
      const float dist = dx * dx + dy * dy + dz * dz;
      //...

      if (dist < dmin) {
        arc0 = *iarc;
        dmin = dist;
      }

      iarc++;
    }

    // Make this point the origin of the arc length for this path
    for (auto iarcnew = ialla->begin(); iarcnew < ialla->end(); iarcnew++) {
      *iarcnew -= arc0;
    }

    arcend1.push_back(*min_element(ialla->begin(), ialla->end()));
    arcend2.push_back(*max_element(ialla->begin(), ialla->end()));

    ialla++;
  }

  // Find outlier paths based on arc length parameterization
  arc1m = accumulate(arcend1.begin(), arcend1.end(), 0.0) / arcend1.size();
  arc2m = accumulate(arcend2.begin(), arcend2.end(), 0.0) / arcend2.size();

  arc1s =
      sqrt(inner_product(arcend1.begin(), arcend1.end(), arcend1.begin(), 0.0) /
               arcend1.size() -
           arc1m * arc1m);
  arc2s =
      sqrt(inner_product(arcend2.begin(), arcend2.end(), arcend2.begin(), 0.0) /
               arcend2.size() -
           arc2m * arc2m);

  lthresh1 = arc1m - 3 * arc1s;
  lthresh2 = arc2m - 3 * arc2s;

  uthresh1 = arc1m + 3 * arc1s;
  uthresh2 = arc2m + 3 * arc2s;

  isout.resize(allarc.size());
  fill(isout.begin(), isout.end(), false);

  iarc1 = arcend1.begin();
  iarc2 = arcend2.begin();

  for (auto iout = isout.begin(); iout < isout.end(); iout++) {
    if (*iarc1 < lthresh1 || *iarc1 > uthresh1 || *iarc2 < lthresh2 ||
        *iarc2 > uthresh2) {
      *iout = true;
      cout << "Found outlier path: " << subjlist[iout - isout.begin()] << endl;
    }

    iarc1++;
    iarc2++;
  }

  // Interpolate measures at the same arc lengths on every path
  arcmin = *min_element(arcend1.begin(), arcend1.end());
  arcmax = *max_element(arcend2.begin(), arcend2.end());

  narc = static_cast<unsigned int>(floor((arcmax - arcmin) / darc));

  nsubj.resize(narc);
  fill(nsubj.begin(), nsubj.end(), 0);

  meanpath.resize(narc * 3);
  fill(meanpath.begin(), meanpath.end(), 0.0);

  iallm = allmeas.begin();
  iallp = allpaths.begin();
  iout = isout.begin();

  for (auto ialla = allarc.begin(); ialla < allarc.end(); ialla++) {
    float larc = arcmin + darc;
    auto insubj = nsubj.begin();
    auto imean = meanpath.begin();
    vector<float> meas;

    for (unsigned int ilen = 0; ilen < narc; ilen++) {
      float slope;
      vector<float>::const_iterator iarc = ialla->begin();
      vector<float>::const_iterator imeas1;
      vector<float>::const_iterator imeas0;

      if (*iarc > larc) { // No points in this segment, skip ahead
        for (int k = static_cast<int>(nmeas); k > 0; k--) {
          meas.push_back(numeric_limits<float>::infinity());
        }
      } else {
        while (*iarc < larc && iarc < ialla->end()) {
          iarc++;
        }

        if (iarc == ialla->end()) { // No points in this segment, skip ahead
          for (int k = static_cast<int>(nmeas); k > 0; k--) {
            meas.push_back(numeric_limits<float>::infinity());
          }
        } else {
          // Linear interpolation
          slope = (larc - *(iarc - 1)) / (*iarc - *(iarc - 1));

          imeas1 = iallm->begin() + nmeas * (iarc - ialla->begin());
          imeas0 = imeas1 - nmeas;

          // Interpolate values of each measure
          for (int k = static_cast<int>(nmeas); k > 0; k--) {
            meas.push_back(*imeas0 + (*imeas1 - *imeas0) * slope);

            imeas1++;
            imeas0++;
          }

          if (!*iout) {
            auto ipt = iallp->begin() + 3 * (iarc - ialla->begin());

            // Increment number of samples in this segment
            (*insubj)++;

            // Add point towards mean path
            for (int k = 0; k < 3; k++) {
              imean[k] += ipt[k];
            }
          }
        }
      }

      larc += darc;
      insubj++;
      imean += 3;
    }

    allmeasint.push_back(meas);

    iallm++;
    iallp++;
    iout++;
  }

  // Minimum number of subjects that must contribute to a position on the path
  nsubjmin =
      static_cast<unsigned int>(ceil(.2 * static_cast<float>(subjlist.size())));

  // Remove positions from start of path that don't have enough samples
  while (*nsubj.begin() < nsubjmin) {
    nsubj.erase(nsubj.begin());

    meanpath.erase(meanpath.begin(), meanpath.begin() + 3);

    for (auto iallm = allmeasint.begin(); iallm < allmeasint.end(); iallm++) {
      iallm->erase(iallm->begin(), iallm->begin() + nmeas);
    }
  }

  // Remove positions from end of path that don't have enough samples
  while (*(nsubj.end() - 1) < nsubjmin) {
    nsubj.erase(nsubj.end() - 1);

    meanpath.erase(meanpath.end() - 3, meanpath.end());

    for (auto iallm = allmeasint.begin(); iallm < allmeasint.end(); iallm++) {
      iallm->erase(iallm->end() - nmeas, iallm->end());
    }
  }

  // Divide sums of points by number of samples to get mean path points
  insubj = nsubj.begin();

  for (auto ipt = meanpath.begin(); ipt < meanpath.end(); ipt += 3) {
    for (int k = 0; k < 3; k++) {
      ipt[k] /= *insubj;
    }

    insubj++;
  }

  // Write points of mean path to file as voxel and RAS coords
  filename = string(outBase) + ".coords.mean.txt";

  cout << "Writing mean path voxel coords to " << filename << endl;
  pathfile.open(filename.c_str(), ios::out);

  filename = string(outBase) + ".path.mean.txt";

  cout << "Writing mean path RAS coords to " << filename << endl;
  pathrasfile.open(filename.c_str(), ios::out);

  pathrasfile << "#!ascii label, vox2ras=scanner" << endl
              << meanpath.size() / 3 << endl;

  npt = 1;

  for (auto ipt = meanpath.begin(); ipt < meanpath.end(); ipt += 3) {
    // Write voxel coordinates
    pathfile << ipt[0] << " " << ipt[1] << " " << ipt[2] << endl;

    // Write RAS coordinates (in freeview waypoint file format)
    pathrasfile << npt;

    for (int k = 1; k < 4; k++) { // Transform from voxel to RAS coords
      pathrasfile << " "
                  << ipt[0] * outv2r->rptr[k][1] + ipt[1] * outv2r->rptr[k][2] +
                         ipt[2] * outv2r->rptr[k][3] + outv2r->rptr[k][4];
    }

    pathrasfile << " 0" << endl;

    npt++;
  }

  pathfile.close();
  pathrasfile.close();

  // Write output files
  ntot = allmeasint[0].size();

  for (auto imeas = measlist.begin(); imeas < measlist.end(); imeas++) {
    string outname = string(outBase) + "." + *imeas + ".txt";
    ofstream outfile;

    cout << "Writing group table to " << outname << endl;
    outfile.open(outname.c_str(), ios::out);

    // Write subject names
    for (auto isubj = subjlist.begin(); isubj < subjlist.end(); isubj++) {
      outfile << *isubj << " ";
    }

    outfile << endl;

    // Write interpolated values of this measure
    for (unsigned ipt = imeas - measlist.begin(); ipt < ntot; ipt += nmeas) {
      for (iallm = allmeasint.begin(); iallm < allmeasint.end(); iallm++) {
        auto ival = iallm->begin() + ipt;

        if (*ival == numeric_limits<float>::infinity()) {
          outfile << "NaN ";
        } else {
          outfile << *ival << " ";
        }
      }

      outfile << endl;
    }

    outfile.close();
  }

  // Average measures over sections along the path
  if (nSection > 0) {
    char nsec[PATH_MAX];

    sprintf(nsec, "%dsec", nSection);

    darc = (arcmax - arcmin) / nSection;

    iallm = allmeas.begin();

    for (auto ialla = allarc.begin(); ialla < allarc.end(); ialla++) {
      float larc = arcmin + darc;
      vector<float>::const_iterator iarc = ialla->begin();
      vector<float>::const_iterator imeas = iallm->begin();
      vector<float> meas;

      while (larc <= arcmax) {
        int nsamp = 0;
        vector<float> avg(nmeas, 0);

        while (*iarc < larc && iarc < ialla->end()) {
          for (auto iavg = avg.begin(); iavg < avg.end(); iavg++) {
            *iavg += *imeas;

            imeas++;
          }

          iarc++;
          nsamp++;
        }

        if (nsamp > 0) {
          for (auto iavg = avg.begin(); iavg < avg.end(); iavg++) {
            *iavg /= nsamp;
          }
        } else { // No points in this section
          for (auto iavg = avg.begin(); iavg < avg.end(); iavg++) {
            *iavg = numeric_limits<float>::infinity();
          }
        }

        meas.insert(meas.end(), avg.begin(), avg.end());

        larc += darc;
      }

      allmeassec.push_back(meas);

      iallm++;
    }

    // Write output files
    ntot = nSection * nmeas;

    for (auto imeas = measlist.begin(); imeas < measlist.end(); imeas++) {
      string outname = string(outBase) + "." + *imeas + "." + nsec + ".txt";
      ofstream outfile;

      cout << "Writing group table to " << outname << endl;
      outfile.open(outname.c_str(), ios::out);

      // Write subject names
      for (auto isubj = subjlist.begin(); isubj < subjlist.end(); isubj++) {
        outfile << *isubj << " ";
      }

      outfile << endl;

      // Write section averages of values of this measure
      for (unsigned ipt = imeas - measlist.begin(); ipt < ntot; ipt += nmeas) {
        for (iallm = allmeassec.begin(); iallm < allmeassec.end(); iallm++) {
          auto ival = iallm->begin() + ipt;

          if (*ival == numeric_limits<float>::infinity()) {
            outfile << "NaN ";
          } else {
            outfile << *ival << " ";
          }
        }

        outfile << endl;
      }

      outfile.close();
    }
  }

  if (outref != nullptr) {
    MRIfree(&outref);
    MatrixFree(&outv2r);
  }

  cputime = cputimer.milliseconds();
  cout << "Done in " << cputime / 1000.0 << " sec." << endl;

  cout << "dmri_group done" << endl;
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
    } else if (strcmp(option, "--list") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inListFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--ref") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outRefFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outBase = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (strcmp(option, "--sec") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      sscanf(pargv[0], "%d", &nSection);
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
  cout << endl
       << "USAGE: " << Progname << endl
       << endl
       << "Required arguments" << endl
       << "   --list <file>:" << endl
       << "     Text file with list of individual inputs" << endl
       << "   --ref <file>:" << endl
       << "     Reference volume for output path" << endl
       << "   --out <base>:" << endl
       << "     Base name of output text files" << endl
       << endl
       << "Optional arguments" << endl
       << "   --sec <num>:" << endl
       << "     Divide the pathway into a number of sections and output "
       << endl
       << "     average measures for each section" << endl
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
  if (outBase == nullptr) {
    cout << "ERROR: must specify base name for output files" << endl;
    exit(1);
  }
  if (inListFile == nullptr) {
    cout << "ERROR: must specify input list file" << endl;
    exit(1);
  }
}

static void dump_options(FILE *fp) {
  cout << endl
       << vcid << endl
       << "cwd " << cwd << endl
       << "cmdline " << cmdline << endl
       << "sysname  " << uts.sysname << endl
       << "hostname " << uts.nodename << endl
       << "machine  " << uts.machine << endl
       << "user     " << VERuser() << endl;

  cout << "Base name of output files: " << outBase << endl;
  cout << "Text file with list of individual inputs: " << inListFile << endl;
}
