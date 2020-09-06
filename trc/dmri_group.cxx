/**
 * @brief Combine path measures from multiple subjects
 *
 * Combine path measures from multiple subjects
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
static void dump_options(FILE *fp);

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

const char *Progname = "dmri_group";

int nSection = 0;

std::string inListFile, outRefFile, outBase;

struct utsname uts;
char *         cmdline, cwd[2000];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int                                                    nargs;
  int                                                    cputime;
  unsigned int                                           nmeas;
  unsigned int                                           npt;
  unsigned int                                           narc;
  unsigned int                                           nsubjmin;
  unsigned int                                           ntot;
  float                                                  distmin;
  float                                                  darc;
  float                                                  arcmin;
  float                                                  arcmax;
  float                                                  arc1m;
  float                                                  arc2m;
  float                                                  arc1s;
  float                                                  arc2s;
  float                                                  lthresh1;
  float                                                  lthresh2;
  float                                                  uthresh1;
  float                                                  uthresh2;
  std::string                                            listline;
  std::string                                            filename;
  std::vector<bool>::const_iterator                      iout;
  std::vector<unsigned int>::const_iterator              insubj;
  std::vector<float>::const_iterator                     icenter;
  std::vector<float>::const_iterator                     iarc1;
  std::vector<float>::const_iterator                     iarc2;
  std::vector<std::vector<unsigned int>>::const_iterator iallk;
  std::vector<std::vector<float>>::const_iterator        itemplate;
  std::vector<std::vector<float>>::const_iterator        iallm;
  std::vector<std::vector<float>>::const_iterator        iallp;
  std::vector<std::vector<float>>::iterator              ialla;
  std::vector<bool>                                      isout;
  std::vector<unsigned int>                              lengths;
  std::vector<unsigned int>                              nsubj;
  std::vector<float>                                     meanpath;
  std::vector<float>                                     arcend1;
  std::vector<float>                                     arcend2;
  std::vector<std::string>                               subjlist;
  std::vector<std::string>                               measlist;
  std::vector<std::vector<unsigned int>>                 allknots;
  std::vector<std::vector<float>>                        allarc;
  std::vector<std::vector<float>>                        allpaths;
  std::vector<std::vector<float>>                        allmeas;
  std::vector<std::vector<float>>                        allmeasint;
  std::vector<std::vector<float>>                        allmeassec;
  std::ifstream                                          listfile;
  std::ofstream                                          pathfile;
  std::ofstream                                          pathrasfile;
  MATRIX *                                               outv2r;
  MRI *                                                  outref = nullptr;

  nargs = handleVersionOption(argc, argv, "dmri_group");
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

  dump_options(stdout);

  cputimer.reset();

  // Read output reference volume
  if (!outRefFile.empty()) {
    cout << "Loading output reference volume from " << outRefFile << endl;
    outref = MRIread(outRefFile.c_str());
    if (!outref) {
      cout << "ERROR: Could not read " << outRefFile << endl;
      exit(1);
    }

    outv2r = MRIgetVoxelToRasXform(outref);
  }

  // Read list of inputs
  std::cout << "Loading list of inputs from " << inListFile << std::endl;
  listfile.open(inListFile, std::ios::in);
  if (!listfile) {
    std::cout << "ERROR: Could not open " << inListFile << " for reading"
              << std::endl;
    exit(1);
  }

  while (getline(listfile, listline)) {
    std::string               measline;
    std::string               subjid;
    std::vector<unsigned int> knots;
    std::vector<float>        arc;
    std::vector<float>        path;
    std::vector<float>        meas;
    std::vector<std::string>  inputs;
    std::ifstream             infile;
    std::istringstream        liststr(listline);
    MRI *                     inref = nullptr;
    AffineReg                 affinereg;
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

    std::cout << "Loading measures along the path from " << filename
              << std::endl;
    infile.open(filename.c_str(), std::ios::in);
    if (!infile) {
      std::cout << "WARN: Could not open " << filename << std::endl
                << "WARN: Skipping ahead" << std::endl;
      continue;
    }

    // Second input on each line is an input reference volume
    if (inputs.size() > 1) {
      // Read reference volumes
      std::cout << "Loading input reference volume from " << inputs[1]
                << std::endl;
      inref = MRIread(inputs[1].c_str());
      if (inref == nullptr) {
        std::cout << "ERROR: Could not read " << inputs[1] << std::endl;
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
        std::string        word;
        std::istringstream linestr(measline);

        linestr >> word;
        linestr >> word;

        if (word.compare("subjectname") == 0) {
          linestr >> subjid;
        }
      } else if (measline.substr(0, 1).compare("x") == 0) { // Header line
        std::string        word;
        std::istringstream linestr(measline);

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
        float              val;
        std::vector<float> point;
        std::vector<float> valpoint;
        std::istringstream linestr(measline);

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
          const float dx     = *(path.end() - 3) - point[0];
          const float dy     = *(path.end() - 2) - point[1];
          const float dz     = *(path.end() - 1) - point[2];
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
      float       dmin    = std::numeric_limits<float>::infinity();
      auto        imin    = arc.begin();

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
  iallk     = allknots.begin();
  itemplate = allpaths.begin();
  icenter   = itemplate->begin() + (allknots[0][2] * 3);
  distmin   = std::numeric_limits<float>::infinity();

  for (auto iallp = allpaths.begin(); iallp < allpaths.end(); iallp++) {
    float dist  = 0.0;
    auto  jallk = allknots.begin();

    for (auto jallp = allpaths.begin(); jallp < allpaths.end(); jallp++) {
      if (jallp != iallp) {
        auto iknot = iallk->begin();

        for (auto jknot = jallk->begin(); jknot < jallk->end(); jknot++) {
          const unsigned int ioffset = (*iknot) * 3;
          const unsigned int joffset = (*jknot) * 3;
          const float        dx      = iallp->at(ioffset) - jallp->at(joffset);
          const float dy = iallp->at(ioffset + 1) - jallp->at(joffset + 1);
          const float dz = iallp->at(ioffset + 2) - jallp->at(joffset + 2);

          dist += sqrt(dx * dx + dy * dy + dz * dz);

          iknot++;
        }
      }

      jallk++;
    }

    if (dist < distmin) {
      itemplate = iallp; // Most representative path
      icenter   = iallp->begin() + (iallk->at(2) * 3); // Mid-point on path
      distmin   = dist;
    }

    iallk++;
  }

  // Choose sampling interval for measures along the path
  darc = *(allarc[itemplate - allpaths.begin()].end() - 1) /
         lengths[itemplate - allpaths.begin()];

  std::cout << "INFO: Sampling interval along path is ";
  if (outref != nullptr) {
    std::cout << darc * outref->xsize << " mm" << std::endl;
  } else {
    std::cout << darc << " voxels" << std::endl;
  }

  if (0) {
    // Write points of most representative path to file as RAS coords
    filename = string(outBase) + ".median.txt";

    std::cout << "Writing median path to " << filename << std::endl;
    pathrasfile.open(filename.c_str(), std::ios::out);

    pathrasfile << "#!ascii label, vox2ras=scanner" << std::endl
                << itemplate->size() / 3 << std::endl;

    npt = 1;

    for (auto ipt = itemplate->begin(); ipt < itemplate->end(); ipt += 3) {
      pathrasfile << npt;

      for (int k = 1; k < 4; k++) { // Transform from voxel to RAS coords
        pathrasfile << " "
                    << ipt[0] * outv2r->rptr[k][1] +
                           ipt[1] * outv2r->rptr[k][2] +
                           ipt[2] * outv2r->rptr[k][3] + outv2r->rptr[k][4];
      }

      pathrasfile << " 0" << std::endl;

      npt++;
    }

    pathrasfile.close();
  }

  // Reparameterize the arc length on each path
  ialla = allarc.begin();

  for (auto iallp = allpaths.begin(); iallp < allpaths.end(); iallp++) {
    float dmin = std::numeric_limits<float>::infinity();
    float arc0 = 0;
    auto  iarc = ialla->begin();

    // Find the closest point to the mid-point of the most representative path
    for (auto ipath = iallp->begin(); ipath < iallp->end(); ipath += 3) {
      const float dx   = ipath[0] - icenter[0];
      const float dy   = ipath[1] - icenter[1];
      const float dz   = ipath[2] - icenter[2];
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
      std::cout << "Found outlier path: " << subjlist[iout - isout.begin()]
                << std::endl;
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
  iout  = isout.begin();

  for (auto ialla = allarc.begin(); ialla < allarc.end(); ialla++) {
    float              larc   = arcmin + darc;
    auto               insubj = nsubj.begin();
    auto               imean  = meanpath.begin();
    std::vector<float> meas;

    for (unsigned int ilen = 0; ilen < narc; ilen++) {
      float                              slope;
      std::vector<float>::const_iterator iarc = ialla->begin();
      std::vector<float>::const_iterator imeas1;
      std::vector<float>::const_iterator imeas0;

      if (*iarc > larc) { // No points in this segment, skip ahead
        for (int k = static_cast<int>(nmeas); k > 0; k--) {
          meas.push_back(std::numeric_limits<float>::infinity());
        }
      } else {
        while (*iarc < larc && iarc < ialla->end()) {
          iarc++;
        }

        if (iarc == ialla->end()) { // No points in this segment, skip ahead
          for (int k = static_cast<int>(nmeas); k > 0; k--) {
            meas.push_back(std::numeric_limits<float>::infinity());
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

  std::cout << "Writing mean path voxel coords to " << filename << std::endl;
  pathfile.open(filename.c_str(), std::ios::out);

  filename = string(outBase) + ".path.mean.txt";

  std::cout << "Writing mean path RAS coords to " << filename << std::endl;
  pathrasfile.open(filename.c_str(), std::ios::out);

  pathrasfile << "#!ascii label, vox2ras=scanner" << std::endl
              << meanpath.size() / 3 << std::endl;

  npt = 1;

  for (auto ipt = meanpath.begin(); ipt < meanpath.end(); ipt += 3) {
    // Write voxel coordinates
    pathfile << ipt[0] << " " << ipt[1] << " " << ipt[2] << std::endl;

    // Write RAS coordinates (in freeview waypoint file format)
    pathrasfile << npt;

    for (int k = 1; k < 4; k++) { // Transform from voxel to RAS coords
      pathrasfile << " "
                  << ipt[0] * outv2r->rptr[k][1] + ipt[1] * outv2r->rptr[k][2] +
                         ipt[2] * outv2r->rptr[k][3] + outv2r->rptr[k][4];
    }

    pathrasfile << " 0" << std::endl;

    npt++;
  }

  pathfile.close();
  pathrasfile.close();

  // Write output files
  ntot = allmeasint[0].size();

  for (auto imeas = measlist.begin(); imeas < measlist.end(); imeas++) {
    std::string   outname = string(outBase) + "." + *imeas + ".txt";
    std::ofstream outfile;

    std::cout << "Writing group table to " << outname << std::endl;
    outfile.open(outname.c_str(), std::ios::out);

    // Write subject names
    for (auto isubj = subjlist.begin(); isubj < subjlist.end(); isubj++) {
      outfile << *isubj << " ";
    }

    outfile << std::endl;

    // Write interpolated values of this measure
    for (unsigned ipt = imeas - measlist.begin(); ipt < ntot; ipt += nmeas) {
      for (iallm = allmeasint.begin(); iallm < allmeasint.end(); iallm++) {
        auto ival = iallm->begin() + ipt;

        if (*ival == std::numeric_limits<float>::infinity()) {
          outfile << "NaN ";
        } else {
          outfile << *ival << " ";
        }
      }

      outfile << std::endl;
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
      float                              larc  = arcmin + darc;
      std::vector<float>::const_iterator iarc  = ialla->begin();
      std::vector<float>::const_iterator imeas = iallm->begin();
      std::vector<float>                 meas;

      while (larc <= arcmax) {
        int                nsamp = 0;
        std::vector<float> avg(nmeas, 0);

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
            *iavg = std::numeric_limits<float>::infinity();
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
      std::string outname =
          string(outBase) + "." + *imeas + "." + nsec + ".txt";
      std::ofstream outfile;

      std::cout << "Writing group table to " << outname << std::endl;
      outfile.open(outname.c_str(), std::ios::out);

      // Write subject names
      for (auto isubj = subjlist.begin(); isubj < subjlist.end(); isubj++) {
        outfile << *isubj << " ";
      }

      outfile << std::endl;

      // Write section averages of values of this measure
      for (unsigned ipt = imeas - measlist.begin(); ipt < ntot; ipt += nmeas) {
        for (iallm = allmeassec.begin(); iallm < allmeassec.end(); iallm++) {
          auto ival = iallm->begin() + ipt;

          if (*ival == std::numeric_limits<float>::infinity()) {
            outfile << "NaN ";
          } else {
            outfile << *ival << " ";
          }
        }

        outfile << std::endl;
      }

      outfile.close();
    }
  }

  if (outref != nullptr) {
    MRIfree(&outref);
    MatrixFree(&outv2r);
  }

  cputime = cputimer.milliseconds();
  std::cout << "Done in " << cputime / 1000.0 << " sec." << std::endl;

  std::cout << "dmri_group done" << std::endl;
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
    } else if (strcmp(option, "--list") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      inListFile = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (strcmp(option, "--ref") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outRefFile = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (strcmp(option, "--out") == 0) {
      if (nargc < 1) {
        CMDargNErr(option, 1);
      }
      outBase   = fio_fullpath(pargv[0]);
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
  std::cout
      << std::endl
      << "USAGE: " << Progname << std::endl
      << std::endl
      << "Required arguments" << std::endl
      << "   --list <file>:" << std::endl
      << "     Text file with list of individual inputs" << std::endl
      << "   --ref <file>:" << std::endl
      << "     Reference volume for output path" << std::endl
      << "   --out <base>:" << std::endl
      << "     Base name of output text files" << std::endl
      << std::endl
      << "Optional arguments" << std::endl
      << "   --sec <num>:" << std::endl
      << "     Divide the pathway into a number of sections and output "
      << std::endl
      << "     average measures for each section" << std::endl
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
static void check_options(void) {
  if (outBase.empty()) {
    cout << "ERROR: must specify base name for output files" << endl;
    exit(1);
  }
  if (inListFile.empty()) {
    cout << "ERROR: must specify input list file" << endl;
    exit(1);
  }
}

static void dump_options(FILE *fp) {
  std::cout << std::endl
            << getVersion() << std::endl
            << "cwd " << cwd << std::endl
            << "cmdline " << cmdline << std::endl
            << "sysname  " << uts.sysname << std::endl
            << "hostname " << uts.nodename << std::endl
            << "machine  " << uts.machine << std::endl
            << "user     " << VERuser() << std::endl;

  std::cout << "Base name of output files: " << outBase << std::endl;
  std::cout << "Text file with list of individual inputs: " << inListFile
            << std::endl;

  return;
}
