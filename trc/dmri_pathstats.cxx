/**
 * @brief Compute measures on probabilistic or deterministic tractography paths
 *
 * Compute measures on probabilistic or deterministic tractography paths
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
#include "spline.h"

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

#include "TrackIO.h"

static int  parse_commandline(int argc, char **argv);
static void check_options(void);
static void print_usage(void);
static void usage_exit(void);
static void print_help(void);
static void print_version(void);
static void dump_options(FILE *fp);
static void WriteHeader(const std::string OutFile);

int debug = 0, checkoptsonly = 0;

int main(int argc, char *argv[]);

const char *Progname = "dmri_pathstats";

float probThresh = .2, faThresh = 0;
char PathMAP[] = "path.map.txt";
std::string inTrkFile, inRoi1File, inRoi2File, inTrcDir, inVoxFile = std::string(PathMAP),
  dtBase, outFile, outVoxFile, outMedianFile, outEndBase, refVolFile, fname;

MRI *l1, *l2, *l3, *v1;

struct utsname uts;
char *         cmdline, cwd[2000], subjName[100], pathName[100];

Timer cputimer;

/*--------------------------------------------------*/
int main(int argc, char **argv) {
  int                nargs, cputime, count, volume, lenmin, lenmax, lencent;
  float              lenavg;
  std::vector<float> avg, wavg, cavg;
  std::vector<MRI *> meas;
  std::ofstream      fout;

  nargs = handleVersionOption(argc, argv, "dmri_pathstats");
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

  dump_options(stdout);

  printf("Computing statistics on %s...\n", !inTrcDir.empty()?inTrcDir.c_str():inTrkFile.c_str());
  cputimer.reset();

  if (!dtBase.empty()) {
    fname = dtBase + "_L1.nii.gz";
    l1 = MRIread(fname.c_str());
    fname =  dtBase + "_L2.nii.gz";
    l2 = MRIread(fname.c_str());
    fname =  dtBase + "_L3.nii.gz";
    l3 = MRIread(fname.c_str());
    fname =  dtBase + "_v1.nii.gz";
    v1 = MRIread(fname.c_str());

    fname = dtBase + "_L1.nii.gz";
    meas.push_back(MRIread(fname.c_str()));		// Axial diffusivity
    fname = dtBase + "_L2.nii.gz";
    meas.push_back(MRIread(fname.c_str()));
    MRIadd(l3, meas[1], meas[1]);
    MRIscalarMul(meas[1], meas[1], .5);		// Radial diffusivity
    fname = dtBase + "_MD.nii.gz";
    meas.push_back(MRIread(fname.c_str()));		// Mean diffusivity
    fname = dtBase + "_FA.nii.gz";
    meas.push_back(MRIread(fname.c_str()));		// Fractional anisotropy
  }

  if (!outVoxFile.c_str()) {
    WriteHeader(outVoxFile);

    std::ofstream fvox(outVoxFile, std::ios::app);
    fvox << "# pathway start" << std::endl
         << "x y z AD RD MD FA AD_Avg RD_Avg MD_Avg FA_Avg" << std::endl;
    fvox.close();
  }

  if (!inTrcDir.empty()) {		     // Probabilistic paths
    int len, nx, ny, nz, nvox = 0;
    float wtot = 0, pthresh = 0;
    vector<int> lengths, pathmap, basepathmap;
    vector<float>::iterator iavg, iwavg;
    MRI *post;
    ifstream lenfile, infile;
    string pathline;

    // Read lengths of path samples
    fname = inTrcDir + "/length.samples.txt";
    lenfile.open(fname, ios::in);
    if (!lenfile) {
      std::cout << "ERROR: Could not open " << fname << " for reading"
                << std::endl;
      exit(1);
    }

    // Sum path lengths
    lenavg = 0;
    while (lenfile >> len) {
      lengths.push_back(len);
      lenavg += len;
    }

    lenfile.close();

    // Read path posterior distribution
    fname = inTrcDir + "/path.pd.nii.gz";
    post = MRIread(fname.c_str());
    nx = post->width;
    ny = post->height;
    nz = post->depth;

    // Find (robust) maximum value of posterior distribution
    pthresh = (float)MRIfindPercentile(post, .99, 0);

    // Set probability threshold as a portion (default: 20%) of (robust) maximum
    pthresh *= probThresh;

    // Compute average and weighted average of measures on thresholded posterior
    avg.resize(meas.size());
    fill(avg.begin(), avg.end(), 0.0);
    wavg.resize(meas.size());
    fill(wavg.begin(), wavg.end(), 0.0);

    for (int iz = 0; iz < nz; iz++)
      for (int iy = 0; iy < ny; iy++)
        for (int ix = 0; ix < nx; ix++) {
          const float h = MRIgetVoxVal(post, ix, iy, iz, 0);

          if (h > pthresh) {
            if (faThresh > 0) // If FA threshold has been set
              if (MRIgetVoxVal(*(meas.end() - 1), ix, iy, iz, 0) <= faThresh)
                continue;

            iavg  = avg.begin();
            iwavg = wavg.begin();

            for (std::vector<MRI *>::const_iterator ivol = meas.begin();
                 ivol < meas.end(); ivol++) {
              *iavg += MRIgetVoxVal(*ivol, ix, iy, iz, 0);
              *iwavg += h * MRIgetVoxVal(*ivol, ix, iy, iz, 0);

              iavg++;
              iwavg++;
            }

            nvox++;
            wtot += h;
          }
        }

    if (nvox > 0) {
      for (iavg = avg.begin(); iavg < avg.end(); iavg++) {
        *iavg /= nvox;
      }
    }

    if (wtot > 0) {
      for (iwavg = wavg.begin(); iwavg < wavg.end(); iwavg++) {
        *iwavg /= wtot;
      }
    }

    // Read maximum a posteriori path coordinates
    fname = inTrcDir + '/' + inVoxFile;
    infile.open(fname, ios::in);
    if (!infile) {
      std::cout << "ERROR: Could not open " << fname << " for reading"
                << std::endl;
      exit(1);
    }

    while (getline(infile, pathline)) {
      float              coord;
      std::istringstream pathstr(pathline);

      for (int k = 0; k < 3; k++)
        if (pathstr >> coord)
          pathmap.push_back((int)round(coord));

      for (int k = 0; k < 3; k++)
        if (pathstr >> coord)
          basepathmap.push_back((int)round(coord));
    }

    if (!basepathmap.empty() && basepathmap.size() != pathmap.size()) {
      std::cout << "ERROR: Unexpected number of coordinates in " << fname
                << std::endl;
      exit(1);
    }

    // Overall measures
    count   = lengths.size();
    volume  = nvox;
    lenmin  = *min_element(lengths.begin(), lengths.end());
    lenmax  = *max_element(lengths.begin(), lengths.end());
    lenavg  = ((lenavg > 0) ? (lenavg / (float)lengths.size()) : 0);
    lencent = pathmap.size() / 3;

    if (!dtBase.empty()) {
      vector<float>::iterator iavg;

      cavg.resize(meas.size());
      fill(cavg.begin(), cavg.end(), 0.0);

      for (std::vector<int>::const_iterator ipt = pathmap.begin();
           ipt < pathmap.end(); ipt += 3) {
        iavg = cavg.begin();

        for (std::vector<MRI *>::const_iterator ivol = meas.begin();
             ivol < meas.end(); ivol++) {
          *iavg += MRIgetVoxVal(*ivol, ipt[0], ipt[1], ipt[2], 0);
          iavg++;
        }
      }

      for (iavg = cavg.begin(); iavg < cavg.end(); iavg++)
        *iavg /= lencent;
    }

    // Measures by voxel on MAP streamline
    if (!outVoxFile.empty()) {
      int npts;
      CTrackReader trkreader;
      TRACK_HEADER trkheadin;
      vector<int>::const_iterator iptbase;
      vector<float> valsum(meas.size());
      vector<float>::iterator ivalsum;
      vector< vector<int> > pathsamples;
      ofstream outfile(outVoxFile, ios::app);

      if (!outfile) {
        std::cout << "ERROR: Could not open " << outVoxFile << " for writing"
                  << std::endl;
        exit(1);
      }

      // Read sample paths from .trk file
      fname = inTrcDir + "/path.pd.trk";

      if (!trkreader.Open(fname.c_str(), &trkheadin)) {
        cout << "ERROR: Cannot open input file " << fname << endl;
        cout << "ERROR: " << trkreader.GetLastErrorMessage() << endl;
        exit(1);
      }

      while (trkreader.GetNextPointCount(&npts)) {
        float *                    iraw, *rawpts = new float[npts * 3];
        std::vector<int>           coords(npts * 3);
        std::vector<int>::iterator icoord = coords.begin();

        // Read a streamline from input file
        trkreader.GetNextTrackData(npts, rawpts);

        // Divide by input voxel size and make 0-based to get voxel coords
        iraw = rawpts;
        for (int ipt = npts; ipt > 0; ipt--)
          for (int k = 0; k < 3; k++) {
            *icoord = (int)round(*iraw / trkheadin.voxel_size[k] - .5);
            iraw++;
            icoord++;
          }

        pathsamples.push_back(coords);
        delete[] rawpts;
      }

      // Loop over all points along the MAP path
      if (!basepathmap.empty())
        iptbase = basepathmap.begin();

      for (std::vector<int>::const_iterator ipt = pathmap.begin();
           ipt < pathmap.end(); ipt += 3) {
        int nsamp = 0;

        // Write coordinates of this point
        if (!basepathmap.empty()) // In base space if longitudinal
          outfile << iptbase[0] << " " << iptbase[1] << " " << iptbase[2];
        else // In native space if cross-sectional
          outfile << ipt[0] << " " << ipt[1] << " " << ipt[2];

        // Write value of each diffusion measure at this point
        for (std::vector<MRI *>::const_iterator ivol = meas.begin();
             ivol < meas.end(); ivol++)
          outfile << " " << MRIgetVoxVal(*ivol, ipt[0], ipt[1], ipt[2], 0);

        // Find closest point on each sample path
        fill(valsum.begin(), valsum.end(), 0.0);

        for (std::vector<std::vector<int>>::const_iterator ipath =
                 pathsamples.begin();
             ipath < pathsamples.end(); ipath++) {
          int                              dmin   = 1000000;
          std::vector<int>::const_iterator iptmin = ipath->begin();

          for (std::vector<int>::const_iterator ipathpt = ipath->begin();
               ipathpt < ipath->end(); ipathpt += 3) {
            int dist = 0;

            for (int k = 0; k < 3; k++) {
              const int diff = ipathpt[k] - ipt[k];
              dist += diff * diff;
            }

            if (dist < dmin) {
              dmin   = dist;
              iptmin = ipathpt;
            }
          }

          /* TESTING
          if (MRIgetVoxVal(post, iptmin[0], iptmin[1], iptmin[2], 0) <= pthresh)
            continue;
*/

          if (faThresh > 0) // If FA threshold has been set
            if (MRIgetVoxVal(*(meas.end() - 1), iptmin[0], iptmin[1], iptmin[2],
                             0) <= faThresh)
              continue;

          nsamp++;

          ivalsum = valsum.begin();

          for (std::vector<MRI *>::const_iterator ivol = meas.begin();
               ivol < meas.end(); ivol++) {
            *ivalsum += MRIgetVoxVal(*ivol, iptmin[0], iptmin[1], iptmin[2], 0);
            ivalsum++;
          }
        }

        // Write average value of each diffusion measure around this point
        ivalsum = valsum.begin();

        for (std::vector<MRI *>::const_iterator ivol = meas.begin();
             ivol < meas.end(); ivol++) {
          outfile << " " << *ivalsum / nsamp;
          ivalsum++;
        }

        outfile << std::endl;

        if (!basepathmap.empty())
          iptbase += 3;
      }
    }
  } else { // Deterministic paths
    // Read .trk file
    Blood myblood(inTrkFile, inRoi1File, inRoi2File);

    myblood.ComputeHistogram();
    myblood.MatchStreamlineEnds();
    myblood.FindCenterStreamline();

    // Overall measures
    count   = myblood.GetNumStr();
    volume  = myblood.GetVolume();
    lenmin  = myblood.GetLengthMin();
    lenmax  = myblood.GetLengthMax();
    lenavg  = myblood.GetLengthAvg();
    lencent = myblood.GetLengthCenter();

    if (!dtBase.empty()) {
      avg  = myblood.ComputeAvgPath(meas);
      wavg = myblood.ComputeWeightAvgPath(meas);
      cavg = myblood.ComputeAvgCenter(meas);
    }

    // Measures by voxel on median streamline
    if (!outVoxFile.empty()) {
      myblood.WriteValuesPointwise(meas, outVoxFile);
    }

    // Save median streamline
    if (!outMedianFile.empty()) {
      myblood.WriteCenterStreamline(outMedianFile, inTrkFile);
    }

    // Save streamline end points
    if (!outEndBase.empty()) {
      MRI *refvol;

      if (!refVolFile.empty()) {
        refvol = MRIread(refVolFile.c_str());
      } else {
        refvol = l1;
      }

      myblood.WriteEndPoints(outEndBase, refvol);
    }
  }

  if (!outFile.empty()) {
    WriteHeader(outFile);

    fout.open(outFile, std::ios::app);

    fout << "Count " << count << std::endl
         << "Volume " << volume << std::endl
         << "Len_Min " << lenmin << std::endl
         << "Len_Max " << lenmax << std::endl
         << "Len_Avg " << lenavg << std::endl
         << "Len_Center " << lencent << std::endl;

    if (!dtBase.empty())
      fout << "AD_Avg "        << avg[0]  << endl
           << "AD_Avg_Weight " << wavg[0] << endl
           << "AD_Avg_Center " << cavg[0] << endl
           << "RD_Avg "        << avg[1]  << endl
           << "RD_Avg_Weight " << wavg[1] << endl
           << "RD_Avg_Center " << cavg[1] << endl
           << "MD_Avg "        << avg[2]  << endl
           << "MD_Avg_Weight " << wavg[2] << endl
           << "MD_Avg_Center " << cavg[2] << endl
           << "FA_Avg "        << avg[3]  << endl
           << "FA_Avg_Weight " << wavg[3] << endl
           << "FA_Avg_Center " << cavg[3] << endl;

    fout.close();
  }

  if (!outVoxFile.empty()) {
    ofstream fvox(outVoxFile, ios::app);
    fvox << "# pathway end" << endl;
    fvox.close();
  }

  cputime = cputimer.milliseconds();
  printf("Done in %g sec.\n", cputime / 1000.0);

  printf("dmri_pathstats done\n");
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
    else if (!strcmp(option, "--intrk")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      inTrkFile = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--rois")) {
      if (nargc < 2)
        CMDargNErr(option, 2);
      inRoi1File = fio_fullpath(pargv[0]);
      inRoi2File = fio_fullpath(pargv[1]);
      nargsused  = 2;
    } else if (!strcmp(option, "--intrc")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      inTrcDir  = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--invox")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      inVoxFile = pargv[0];
      nargsused = 1;
    } else if (!strcmp(option, "--dtbase")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      dtBase    = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--path")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      strcpy(pathName, pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--subj")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      strcpy(subjName, pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--out")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outFile   = fio_fullpath(pargv[0]);
      nargsused = 1;
    } else if (!strcmp(option, "--outvox")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outVoxFile = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (!strcmp(option, "--median")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outMedianFile = fio_fullpath(pargv[0]);
      nargsused     = 1;
    } else if (!strcmp(option, "--ends")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      outEndBase = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (!strcmp(option, "--ref")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      refVolFile = fio_fullpath(pargv[0]);
      nargsused  = 1;
    } else if (!strcmp(option, "--pthr")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      sscanf(pargv[0], "%f", &probThresh);
      nargsused = 1;
    } else if (!strcmp(option, "--fthr")) {
      if (nargc < 1)
        CMDargNErr(option, 1);
      sscanf(pargv[0], "%f", &faThresh);
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
static void print_usage(void) {
  printf("\n");
  printf("USAGE: ./dmri_pathstats\n");
  printf("\n");
  printf("   --intrk <file>:\n");
  printf("     Input trackvis .trk file\n");
  printf("   --rois <file1> <file2>:\n");
  printf("     Input labeling ROIs for .trk file (optional)\n");
  printf("   --intrc <file>:\n");
  printf("     Input tracula directory\n");
  printf("   --dtbase <file>:\n");
  printf("     Base name of input dtifit files (optional)\n");
  printf("   --path <name>:\n");
  printf("     Name of pathway (optional, written to output files)\n");
  printf("   --subj <name>:\n");
  printf("     Name of subject (optional, written to output files)\n");
  printf("   --out <file>:\n");
  printf("     Output text file for overall path measures\n");
  printf("   --outvox <file>:\n");
  printf("     Output text file for voxel-by-voxel measures along path "
         "(optional)\n");
  printf("   --median <file>:\n");
  printf("     Output .trk file of median streamline (optional)\n");
  printf("   --ends   <base>:\n");
  printf("     Base name of output volumes of streamline ends (optional)\n");
  printf("   --ref <file>:\n");
  printf(
      "     Reference volume (needed only if using --ends without --dtbase)\n");
  printf("   --pthr <num>:\n");
  printf("     Lower threshold on path posterior distribution,\n");
  printf("     as a portion of the maximum (range: 0-1, default: 0.2)\n");
  printf("   --fthr <num>:\n");
  printf("     Lower threshold on FA (range: 0-1, default: no threshold)\n");
  printf("\n");
  printf("\n");
  printf("   --debug:     turn on debugging\n");
  printf("   --checkopts: don't run anything, just check options and exit\n");
  printf("   --help:      print out information on how to use this program\n");
  printf("   --version:   print out version and exit\n");
  printf("\n");
}

/* --------------------------------------------- */
static void print_help(void) {
  print_usage();
  printf("\n");
  printf("...\n");
  printf("\n");
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
  if((!inTrcDir.empty()) && (!inTrkFile.empty())) {
    printf("ERROR: cannot specify both .trk file and tracula directory\n");
    exit(1);
  }
  if(inTrcDir.empty() && inTrkFile.empty()) {
    printf("ERROR: must specify input .trk file or tracula directory\n");
    exit(1);
  }
  if(outFile.empty() && outVoxFile.empty() && outMedianFile.empty() && outEndBase.empty()) {
    printf("ERROR: must specify at least one type of output\n");
    exit(1);
  }
  if((!outVoxFile.empty()) && dtBase.empty()) {
    printf("ERROR: must specify dtifit base name for voxel-by-voxel output\n");
    exit(1);
  }
  if((!outMedianFile.empty()) && inTrkFile.empty()) {
    printf("ERROR: must specify input .trk file to use --median\n");
    exit(1);
  }
  if((!outEndBase.empty()) && inTrkFile.empty()) {
    printf("ERROR: must specify input .trk file to use --ends\n");
    exit(1);
  }
  if((!outEndBase.empty()) && refVolFile.empty() && dtBase.empty()) {
    printf("ERROR: must specify reference volume to use --ends\n");
    exit(1);
  }
  if (probThresh < 0 || probThresh > 1) {
    printf("ERROR: probability threshold must be a number between 0 and 1\n");
    exit(1);
  }
  if (faThresh < 0 || faThresh > 1) {
    printf("ERROR: FA threshold must be a number between 0 and 1\n");
    exit(1);
  }
  return;
}

/* --------------------------------------------- */
static void WriteHeader(const std::string OutFile) {
  ofstream fout(OutFile, ios::out);

  fout << "# Title Pathway Statistics" << endl
       << "#"                          << endl
       << "# generating_program "      << Progname << endl
       << "# cvs_version "             << getVersion() << endl
       << "# cmdline "                 << cmdline << endl
       << "# sysname "                 << uts.sysname << endl
       << "# hostname "                << uts.nodename << endl
       << "# machine "                 << uts.machine << endl
       << "# user "                    << VERuser() << endl
       << "# anatomy_type pathway"     << endl
       << "#"                          << endl
       << "# subjectname "             << subjName << endl
       << "# pathwayname "             << pathName << endl
       << "#"                          << endl;

  fout.close();
}

static void dump_options(FILE *fp) {
  fprintf(fp, "\n");
  fprintf(fp, "%s\n", getVersion().c_str());
  fprintf(fp, "cwd %s\n", cwd);
  fprintf(fp, "cmdline %s\n", cmdline);
  fprintf(fp, "sysname  %s\n", uts.sysname);
  fprintf(fp, "hostname %s\n", uts.nodename);
  fprintf(fp, "machine  %s\n", uts.machine);
  fprintf(fp, "user     %s\n", VERuser());

  if (!inTrkFile.empty()) {
    fprintf(fp, "Input .trk file: %s\n", inTrkFile.c_str());
  }
  if (!inRoi1File.empty()) {
    fprintf(fp, "Input end ROI 1: %s\n", inRoi1File.c_str());
    fprintf(fp, "Input end ROI 2: %s\n", inRoi2File.c_str());
  }
  if (!inTrcDir.empty()) {
    fprintf(fp, "Input tracula directory: %s\n", inTrcDir.c_str());
  }
  if (!dtBase.empty()) {
    fprintf(fp, "Input DTI fit base: %s\n", dtBase.c_str());
  }
//  if (pathName)
  fprintf(fp, "Pathway name: %s\n", pathName);
//  if (subjName)
  fprintf(fp, "Subject name: %s\n", subjName);
  if (!outFile.empty()) {
    fprintf(fp, "Output file for overall measures: %s\n", outFile.c_str());
  }
  if (!outVoxFile.empty()) {
    fprintf(fp, "Output file for voxel-by-voxel measures: %s\n", outVoxFile.c_str());
  }
  if (!outMedianFile.c_str()) {
    fprintf(fp, "Output median streamline file: %s\n", outMedianFile.c_str());
  }
  if (!outEndBase.c_str()) {
    fprintf(fp, "Base name of output end point volumes: %s\n", outEndBase.c_str());
  }
  if (!refVolFile.c_str()) {
    fprintf(fp, "Reference for output end point volumes: %s\n", refVolFile.c_str());
  }
  fprintf(fp, "Lower threshold for probability: %f\n", probThresh);
  if (faThresh > 0) {
    fprintf(fp, "Lower threshold for FA: %f\n", faThresh);
  }

  return;
}
