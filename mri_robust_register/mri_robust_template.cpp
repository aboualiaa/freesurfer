/**
 * @file  mri_robust_template.cpp
 * @brief combine multiple volumes by mean or median
 *
 * Creation of robust template of several volumes together with
 * their linear registration
 */

/*
 * Original Author: Martin Reuter
 * CVS Revision Info:
 *    $Author: mreuter $
 *    $Date: 2016/05/05 21:17:08 $
 *    $Revision: 1.54 $
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

#include "MultiRegistration.h"

#include "timer.h"
#include "version.h"

#define SAT -1 // leave blank, either passed by user or --satit
//#define SAT 4.685 // this is suggested for gaussian noise
//#define SAT 20
#define SSAMPLE -1

// unsigned int printmemusage ()
// {
//   char buf[30];
//   snprintf(buf, 30, "/proc/%u/statm", (unsigned)getpid());
//   FILE* pf = fopen(buf, "r");
//   unsigned size = 0; //       total program size
//   if (pf) {
//     //unsigned resident;//   resident set size
//     //unsigned share;//      shared pages
//     //unsigned text;//       text (code)
//     //unsigned lib;//        library
//     //unsigned data;//       data/stack
//     //unsigned dt;//         dirty pages (unused in Linux 2.6)
//     //fscanf(pf, "%u" /* %u %u %u %u %u"*/, &size/*, &resident, &share,
//     &text, &lib, &data*/); fscanf(pf, "%u" , &size);
//     //DOMSGCAT(MSTATS, std::setprecision(4) << size / (1024.0) << "MB mem
//     used"); cout <<  size / (1024.0) << " MB mem used" << endl; fclose(pf);
//   }
// // while (1)
// // {
// //     if ('n' == getchar())
// //        break;
// // }
//   return size;
// }

struct Parameters {
  std::vector<std::string> mov;
  std::vector<std::string> masks;
  std::string              mean;
  std::vector<std::string> iltas;
  std::vector<std::string> nltas;
  std::vector<std::string> nweights;
  std::vector<std::string> mapmovhdr;
  bool                     fixvoxel;
  bool                     floattype;
  bool                     lta_vox2vox;
  bool                     affine;
  bool                     iscale;
  bool                     iscaleonly;
  bool                     transonly;
  bool                     leastsquares;
  bool                     nomulti;
  int                      iterate;
  double                   epsit;
  double                   sat;
  std::vector<std::string> nwarps;
  int                      debug;
  int                      average;
  int                      inittp;
  bool                     noit;
  bool                     quick;
  int                      subsamplesize;
  bool                     fixtp;
  bool                     satit;
  std::string              conform;
  bool                     doubleprec;
  bool                     oneminusweights;
  std::vector<std::string> iscalein;
  std::vector<std::string> iscaleout;
  int                      finalinterp;
  int                      highit;
  unsigned int             seed;
  bool                     crascenter;
  int                      pairiterate;
  double                   pairepsit;
};

// Initializations:
static struct Parameters P = {std::vector<std::string>(0),
                              std::vector<std::string>(0),
                              "",
                              std::vector<std::string>(0),
                              std::vector<std::string>(0),
                              std::vector<std::string>(0),
                              std::vector<std::string>(0),
                              false,
                              false,
                              false,
                              false,
                              false,
                              false,
                              false,
                              false,
                              false,
                              5,
                              -1.0,
                              SAT,
                              std::vector<std::string>(0),
                              0,
                              1,
                              -1,
                              false,
                              false,
                              SSAMPLE,
                              false,
                              false,
                              "",
                              false,
                              true,
                              std::vector<std::string>(0),
                              std::vector<std::string>(0),
                              SAMPLE_CUBIC_BSPLINE,
                              -1,
                              0,
                              false,
                              5,
                              0.01};

static void printUsage();
static bool parseCommandLine(int argc, char *argv[], Parameters &P);

static char vcid[] =
    "$Id: mri_robust_template.cpp,v 1.54 2016/05/05 21:17:08 mreuter Exp $";
const char *Progname = nullptr;

int getRandomNumber(int start, int end, unsigned int &seed)
// return n in [start,end]
// also return seed if it was -1
{

  if (seed == 0) {
    seed = time(nullptr);
  }

  // initialize random seed:
  srand(seed);

  // generate random number:
  int range = end - start + 1;

  return rand() % range + start;
}

int main(int argc, char *argv[]) {
  {
    std::cout << vcid << std::endl << std::endl;
    // set the environment variable
    //  setenv("SURFER_FRONTDOOR","",1) ;
    // to store mri as chunk in memory:
    //  setenv("FS_USE_MRI_CHUNK","",1) ;

    // Default initialization
    int nargs = handleVersionOption(argc, argv, "mri_robust_template");
    if (nargs && argc - nargs == 1) {
      exit(0);
    }
    argc -= nargs;
    Progname = argv[0];
    argc--;
    argv++;
    ErrorInit(NULL, NULL, NULL);
    //  DiagInit(NULL, NULL, NULL) ;

    if (!parseCommandLine(argc, argv, P)) {
      printUsage();
      exit(1);
    }
    //  if (P.outdir[P.outdir.length()-1] != '/') P.outdir += "/";

    // Timer
    Timer start;
    int   msec, minutes, seconds;
    start.reset();
    ///////////////////////////////////////////////////////////////

    MultiRegistration MR;

    // set parameters
    size_t pos = P.mean.rfind("/"); // position of "." in str
    if (pos != std::string::npos) {
      MR.setOutdir(P.mean.substr(0, pos - 1));
    } else {
      MR.setOutdir("./");
    }
    MR.setDebug(P.debug);
    MR.setRigid(!P.affine);
    MR.setTransonly(P.transonly);
    if (P.iscale)
      std::cout << "Setting iscale ..." << std::endl;
    MR.setIscale(P.iscale);
    if (P.iscaleonly)
      std::cout << "Setting iscaleonly ..." << std::endl;
    MR.setIscaleOnly(P.iscaleonly);
    MR.setNoMulti(P.nomulti);
    MR.setRobust(!P.leastsquares);
    MR.setSaturation(P.sat);
    MR.setSatit(P.satit);

    MR.setFixVoxel(P.fixvoxel);
    MR.setKeepType(!P.floattype);
    MR.setAverage(P.average);
    MR.setDoublePrec(P.doubleprec);
    MR.setSubsamplesize(P.subsamplesize);
    MR.setHighit(P.highit);
    if (P.nweights.size() > 0)
      MR.setBackupWeights(true);
    MR.useCRAS(P.crascenter);

    // init MultiRegistration and load movables
    // int nnin = (int) P.mov.size();
    // assert (P.mov.size() >1);
    // assert (MR.loadMovables(P.mov)==nin);
    int nin = MR.loadMovables(P.mov, P.masks);
    if (nin <= 1) {
      std::cerr << "Could not load movables!" << std::endl;
      exit(1);
    }

    // load initial ltas if set:
    if (P.iltas.size() > 0) {
      const int numLoaded = MR.loadLTAs(P.iltas);
      if (numLoaded != nin) {
        std::cerr << "Error: num LTA files != num movables!" << std::endl;
        exit(1);
      }
    }

    // load initial iscales if set:
    if (P.iscalein.size() > 0) {
      const int numLoaded = MR.loadIntensities(P.iscalein);
      if (numLoaded != nin) {
        std::cerr << "Error: num intensity files != num movables!" << std::endl;
        exit(1);
      }
    }

    // Randomly pick target (default):
    // Only if no inittp is passed and if no init transforms are given
    // In those cases we either fixed the target (--inittp)
    // or we won't need to compute ixforms when they are passed (--ixforms)
    if (P.inittp < 0 && P.iltas.size() == 0) {
      // compute seed based on input images if not set:
      if (P.seed == 0)
        P.seed = MR.getSeed();
      // inittp randomly
      P.inittp = getRandomNumber(1, P.mov.size(), P.seed);
      std::cout << "Will use TP " << P.inittp
                << " as random initial target (seed " << P.seed << " )."
                << std::endl;
    }

    if (P.noit) // no registration to mean space, only initial reg. and
                // averaging
    {
      // if no initial xforms are given, use initialization to median space
      //   by registering everything first to inittp
      //   res 0: up to highest resolution, eps 0.01: accurate
      if (P.iltas.size() == 0 && P.inittp > 0) {
        MR.initialXforms(P.inittp, P.fixtp, 0, 5, 0.01);
      }

      // create template:
      MR.setSampleType(P.finalinterp);
      MR.mapAndAverageMov(0);
    }
    // run registrations
    else if (nin == 2) {

      if (P.iltas.size() == 0 && P.inittp > 0) {
        MR.initialXforms(P.inittp, P.fixtp, 0, 5, 0.01);
      }

      // we are done here, since with 2TP the registration
      // to the first mean space is allready accurate

      // create template:
      MR.setSampleType(P.finalinterp);
      MR.mapAndAverageMov(0);

      //    // here default params are adjusted for just 2 images (if not
      //    passed): if (P.iterate == -1) P.iterate = 5; if (P.epsit <= 0)
      //    P.epsit   = 0.01;
      //    MR.halfWayTemplate(0,P.iterate,P.epsit,P.lta_vox2vox);
    } else {
      // if no initial xforms are given, use initialization to median space
      //   by registering everything first to inittp
      //   a) res 1: up to second highest resolution, eps 0.05: not too accurate
      //   b) res 0: up to highest res., eps 0.01 accurate reg.
      //   turns out accurate b) performs better and saves us
      //   from more global iterations (reg to template) later
      // remains open if subsampling speeds up things w/o increasing iterations
      // later
      if (P.iltas.size() == 0 && P.inittp > 0) {
        MR.initialXforms(P.inittp, P.fixtp, 0, 5, 0.01);
      }
      // MR.initialXforms(P.inittp,1,5,0.05);

      // here default is adjusted for several images (and real mean/median
      // target):
      if (P.iterate == -1) {
        P.iterate = 6;
      }
      if (P.epsit <= 0) {
        P.epsit = 0.03;
      }

      // P.iterate and P.epsit are used for terminating the template iterations
      // while 5 and 0.01 are default for the individual pairwise registrations
      MR.computeTemplate(P.iterate, P.epsit, P.pairiterate, P.pairepsit);
      // if final interp not set in multireg, re-average with new type
      if (P.finalinterp != MR.getSampleType()) {
        MR.setSampleType(P.finalinterp);
        MR.mapAndAverageMov(0);
      }
    }

    std::cout << "Writing final template: " << P.mean << std::endl;
    MR.writeMean(P.mean);
    if (P.conform != "") {
      MR.writeConformMean(P.conform);
    }

    // output transforms and warps
    std::cout << "Writing final transforms (warps etc.)..." << std::endl;
    if (P.nltas.size() > 0) {
      MR.writeLTAs(P.nltas, P.lta_vox2vox, P.mean);
    }
    if (P.nwarps.size() > 0) {
      MR.writeWarps(P.nwarps);
    }
    if (P.mapmovhdr.size() > 0) {
      MR.writeMapMovHdr(P.mapmovhdr);
    }
    if (P.iscaleout.size() > 0) {
      MR.writeIntensities(P.iscaleout);
    }
    if (!P.leastsquares && P.nweights.size() > 0) {
      MR.writeWeights(P.nweights, P.oneminusweights);
    }

    MR.clear();

    ///////////////////////////////////////////////////////////////
    msec    = start.milliseconds();
    seconds = nint((float)msec / 1000.0f);
    minutes = seconds / 60;
    seconds = seconds % 60;
    std::cout << "registration took " << minutes << " minutes and " << seconds
              << " seconds." << std::endl;
    // if (diag_fp) fclose(diag_fp) ;

    std::cout << std::endl
              << " Thank you for using RobustTemplate! " << std::endl;
    std::cout
        << " If you find it useful and use it for a publication, please cite: "
        << std::endl
        << std::endl;
    std::cout
        << " Within-Subject Template Estimation for Unbiased Longitudinal "
           "Image Analysis"
        << std::endl;
    std::cout
        << " M. Reuter, N.J. Schmansky, H.D. Rosas, B. Fischl. NeuroImage 2012."
        << std::endl;
    std::cout << " http://dx.doi.org/10.1016/j.neuroimage.2012.02.084"
              << std::endl;
    std::cout << " http://reuter.mit.edu/papers/reuter-long12.pdf" << std::endl
              << std::endl;
  }
  exit(0);
  return (0);
}

/*----------------------------------------------------------------------
 ----------------------------------------------------------------------*/
#include "mri_robust_template.help.xml.h"
static void printUsage() {
  outputHelpXml(mri_robust_template_help_xml, mri_robust_template_help_xml_len);
}

/*!
 \fn int parseNextCommand(int argc, char **argv)
 \brief Parses the command-line for next command
 \param   argc  number of command line arguments
 \param   argv  pointer to a character pointer
 \param      P  reference to parameters
 \returns       number of used arguments for this command
 */
static int parseNextCommand(int argc, char *argv[], Parameters &P) {
  int   nargs = 0;
  char *option;

  option = argv[0] + 1; // remove '-'
  if (option[0] == '-') {
    option = option + 1; // remove second '-'
  }
  StrUpper(option);

  // cout << " option: " << option << endl;

  if (!strcmp(option, "MOV")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.mov.push_back(std::string(argv[nargs]));
        std::cout << "--mov: Using " << P.mov.back()
                  << " as movable/source volume." << std::endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "    Total: " << nargs << " input volumes" << std::endl;
  }
  //  else if (!strcmp(option, "OUTDIR") )
  //  {
  //     P.outdir = string(argv[1]);
  //     nargs = 1;
  //     cout << "--outdir: Using "<< P.outdir << " as output directory." <<
  //     endl;
  //  }
  else if (!strcmp(option, "TEMPLATE")) {
    P.mean = std::string(argv[1]);
    nargs  = 1;
    std::cout << "--template: Using " << P.mean << " as template output volume."
              << std::endl;
  } else if (!strcmp(option, "LTA")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.nltas.push_back(std::string(argv[nargs]));
        // cout << "Using "<< P.nltas.back() << " as LTA." << endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--lta: Will output LTA transforms" << std::endl;
  } else if (!strcmp(option, "MASKS")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.masks.push_back(std::string(argv[nargs]));
        // cout << "Using "<< P.nltas.back() << " as LTA." << endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--masks: Will use masks on inputs." << std::endl;
  } else if (!strcmp(option, "ISCALEOUT")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.iscaleout.push_back(std::string(argv[nargs]));
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    P.iscale = true;
    std::cout
        << "--iscaleout: Will perform intensity scaling and output results"
        << std::endl;
  } else if (!strcmp(option, "ISCALEIN")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.iscalein.push_back(std::string(argv[nargs]));
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--iscalein: Will use init intensity scales" << std::endl;
  } else if (!strcmp(option, "IXFORMS")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;

        P.iltas.push_back(std::string(argv[nargs]));
        // cout << "Using "<< P.nltas.back() << " as LTA." << endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--ixforms: Will use init XFORMS." << std::endl;
  } else if (!strcmp(option, "AVERAGE")) {
    P.average = atoi(argv[1]);
    nargs     = 1;
    std::cout << "--average: Using method " << P.average
              << " for template computation." << std::endl;
  } else if (!strcmp(option, "VOX2VOX")) {
    P.lta_vox2vox = true;
    std::cout << "--vox2vox: Output transforms as VOX2VOX. " << std::endl;
  } else if (!strcmp(option, "AFFINE") || !strcmp(option, "A")) {
    P.affine = true;
    std::cout << "--affine: Enabling affine transform!" << std::endl;
  } else if (!strcmp(option, "ISCALE") || !strcmp(option, "I")) {
    P.iscale = true;
    std::cout << "--iscale: Enabling intensity scaling!" << std::endl;
  } else if (!strcmp(option, "TRANSONLY")) {
    P.transonly = true;
    std::cout << "--transonly: Using only translation!" << std::endl;
  } else if (!strcmp(option, "ISCALEONLY")) {
    P.iscale     = true;
    P.iscaleonly = true;
    P.transonly  = false;
    P.affine     = false;
    std::cout << "--iscaleonly: Computing only global scaling!" << std::endl;
  } else if (!strcmp(option, "NOMULTI")) {
    P.nomulti = true;
    std::cout << "--nomulti: process highest resolution only!" << std::endl;
  } else if (!strcmp(option, "LEASTSQUARES") || !strcmp(option, "L")) {
    P.leastsquares = true;
    std::cout << "--leastsquares: Using standard least squares (non-robust)!"
              << std::endl;
  } else if (!strcmp(option, "MAXIT")) {
    P.iterate = atoi(argv[1]);
    nargs     = 1;
    std::cout << "--maxit: Performing maximal " << P.iterate
              << " iterations on each resolution." << std::endl;
  } else if (!strcmp(option, "HIGHIT")) {
    P.highit = atoi(argv[1]);
    nargs    = 1;
    std::cout << "--highit: Performing maximal " << P.highit
              << " iterations on highest resolution" << std::endl;
  } else if (!strcmp(option, "EPSIT")) {
    P.epsit = atof(argv[1]);
    nargs   = 1;
    std::cout << "--epsit: Stop iterations when change is less than " << P.epsit
              << " . " << std::endl;
  } else if (!strcmp(option, "PAIRMAXIT")) {
    P.pairiterate = atoi(argv[1]);
    nargs         = 1;
    std::cout << "--pairmaxit: Performing maximal " << P.pairiterate
              << " iterations on each resolution for individual pairwise "
                 "registrations."
              << std::endl;
  } else if (!strcmp(option, "PAIREPSIT")) {
    P.pairepsit = atof(argv[1]);
    nargs       = 1;
    std::cout
        << "--pairepsit: Stop individual pairwise iterations when change is "
           "less than "
        << P.pairepsit << " . " << std::endl;
  } else if (!strcmp(option, "SAT")) {
    P.sat = atof(argv[1]);
    nargs = 1;
    std::cout << "--sat: Using saturation " << P.sat << " in M-estimator!"
              << std::endl;
  } else if (!strcmp(option, "SUBSAMPLE")) {
    P.subsamplesize = atoi(argv[1]);
    nargs           = 1;
    if (P.subsamplesize >= 0) {
      std::cout << "--subsample: Will subsample if size is larger than "
                << P.subsamplesize << " on all axes!" << std::endl;
    } else {
      std::cout << "--subsample -1: Will not subsample on any scale!"
                << std::endl;
    }
  } else if (!strcmp(option, "DEBUG")) {
    P.debug = 1;
    nargs   = 0;
    std::cout << "--debug: Will output debug info and files!" << std::endl;
  } else if (!strcmp(option, "NOIT")) {
    P.noit = true;
    nargs  = 0;
    std::cout << "--noit: Will output only first template (no iterations)!"
              << std::endl;
  } else if (!strcmp(option, "FIXTP")) {
    P.fixtp = true;
    nargs   = 0;
    std::cout << "--fixtp: Will map everything to init TP!" << std::endl;
  } else if (!strcmp(option, "SATIT")) {
    P.satit = true;
    nargs   = 0;
    std::cout << "--satit: Will estimate SAT iteratively!" << std::endl;
  } else if (!strcmp(option, "DOUBLEPREC")) {
    P.doubleprec = true;
    nargs        = 0;
    std::cout << "--doubleprec: Will perform algorithm with double precision "
                 "(higher mem usage)!"
              << std::endl;
  } else if (!strcmp(option, "WEIGHTS")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.nweights.push_back(std::string(argv[nargs]));
        // cout << "Using "<< P.nweights.back() << " as weights volume." <<
        // endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--weights: Will output weights in target space" << std::endl;
  } else if (!strcmp(option, "WARP") || !strcmp(option, "MAPMOV")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.nwarps.push_back(std::string(argv[nargs]));
        // cout << "Using "<< P.nwarps.back() << " as weights volume." << endl;
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--mapmov: Will save mapped movables/sources!" << std::endl;
  } else if (!strcmp(option, "MAPMOVHDR")) {
    nargs = 0;
    do {
      option = argv[nargs + 1];
      if (option[0] != '-') {
        nargs++;
        P.mapmovhdr.push_back(std::string(argv[nargs]));
      }
    } while (nargs + 1 < argc && option[0] != '-');
    assert(nargs > 0);
    std::cout << "--mapmovhdr: Will save header-adjusted movables!"
              << std::endl;
  } else if (!strcmp(option, "TEST")) {
    std::cout << "--test: TEST-MODE " << std::endl;
    RegRobust R;
    R.testRobust(argv[2], atoi(argv[1]));
    nargs = 2;
    exit(0);
  } else if (!strcmp(option, "CONFORM")) {
    P.conform = argv[1];
    nargs     = 1;
    std::cout
        << "--conform: Will output conform template (256^3 and 1mm voxels)!"
        << std::endl;
  }
  //   else if (!strcmp(option, "CONFORM") )
  //   {
  //     P.fixvoxel = true;
  //     nargs = 0 ;
  //     cout << "Will conform images to 256^3 and voxels to 1mm!" << endl;
  //   }
  else if (!strcmp(option, "FLOATTYPE")) {
    P.floattype = true;
    nargs       = 0;
    std::cout
        << "--floattype: Use float images internally (independent of input)!"
        << std::endl;
  } else if (!strcmp(option, "FINALNEAREST")) {
    P.finalinterp = SAMPLE_NEAREST;
    nargs         = 0;
    std::cout << "--finalnearest: Use nearest neighbor interpolation for final "
                 "average!"
              << std::endl;
  } else if (!strcmp(option, "INITTP")) {
    P.inittp = atoi(argv[1]);
    nargs    = 1;
    if (P.inittp == 0) {
      std::cout << "--inittp 0: No initialization, construct first mean from "
                   "original TPs"
                << std::endl;
    } else {
      std::cout << "--inittp: Using TP " << P.inittp
                << " as target for initialization" << std::endl;
    }
  } else if (!strcmp(option, "ONEMINUSW")) {
    P.oneminusweights = false;
    nargs             = 0;
    std::cout
        << "--oneminusw: Will output 1-weights (zero=outlier), as in earlier "
           "versions!"
        << std::endl;
  } else if (!strcmp(option, "SEED")) {
    P.seed = atoi(argv[1]);
    nargs  = 1;
    std::cout << "--seed: Will use random seed " << P.seed << std::endl;
  } else if (!strcmp(option, "CRAS")) {
    P.crascenter = true;
    nargs        = 0;
    std::cout << "--cras: Will center template at avgerage CRAS!" << std::endl;
  } else if (!stricmp(option, "HELP") || !stricmp(option, "USAGE") ||
             !stricmp(option, "h") || !stricmp(option, "u")) {
    printUsage();
    exit(0);
  } else {
    std::cerr << std::endl
              << std::endl
              << "ERROR: Option " << argv[0] << " unknown !!! " << std::endl
              << std::endl;
    printUsage();
    exit(1);
  }

  fflush(stdout);

  return (nargs);
}

/*!
 \fn int parseCommandLine(int argc, char **argv)
 \brief Parses the command-line
 \param   argc  number of command line arguments
 \param   argv  pointer to a character pointer
 \param      P  reference to parameters
 \returns       if all necessary parameters were set
 */
static bool parseCommandLine(int argc, char *argv[], Parameters &P) {
  int nargs;
  int n = argc;

  if (n == 0) {
    printUsage();
    exit(1);
  }

  for (; argc > 0 && ISOPTION(*argv[0]); argc--, argv++) {
    nargs = parseNextCommand(argc, argv, P);
    argc -= nargs;
    argv += nargs;
  }

  bool ntest = true;

  if (P.mov.size() < 1) {
    ntest = false;
    std::cerr
        << "ERROR: Specify at least 2 inputs with --mov (or one 4D input)!"
        << std::endl;
    exit(1);
  }
  if (P.mean == "") {
    ntest = false;
    std::cerr << "ERROR: Specify output image with --template !" << std::endl;
    exit(1);
  }
  if (P.nwarps.size() > 0 && P.mov.size() != P.nwarps.size()) {
    ntest = false;
    std::cerr << "ERROR: Number of filenames for --warp should equal number of "
                 "inputs!"
              << std::endl;
    exit(1);
  }
  if (P.mapmovhdr.size() > 0 && P.mov.size() != P.mapmovhdr.size()) {
    ntest = false;
    std::cerr << "ERROR: Number of filenames for --mapmovhdr should equal "
                 "number of inputs!"
              << std::endl;
    exit(1);
  }
  if (P.nltas.size() > 0 && P.mov.size() != P.nltas.size()) {
    ntest = false;
    std::cerr
        << "ERROR: Number of filenames for --lta should equal number of inputs!"
        << std::endl;
    exit(1);
  }
  if (P.iltas.size() > 0 && P.mov.size() != P.iltas.size()) {
    ntest = false;
    std::cerr
        << "ERROR: Number of filenames for --ixforms should equal number of "
           "inputs!"
        << std::endl;
    exit(1);
  }
  if (P.nweights.size() > 0 && P.mov.size() != P.nweights.size()) {
    ntest = false;
    std::cerr
        << "ERROR: Number of filenames for --weights should equal number of "
           "inputs!"
        << std::endl;
    exit(1);
  }
  if (P.iscalein.size() > 0 && P.iltas.size() != P.iscalein.size()) {
    ntest = false;
    std::cerr << "ERROR: Number of filenames for --iscalein should agree with "
                 "number of init LTAs (--ixforms)!"
              << std::endl;
    exit(1);
  }
  if (P.iscaleout.size() > 0 && P.mov.size() != P.iscaleout.size()) {
    ntest = false;
    std::cerr
        << "ERROR: Number of filenames for --iscaleout should equal number of "
           "inputs!"
        << std::endl;
    exit(1);
  }
  if (n > 0 && !P.satit && P.sat <= 0 && !(P.noit && P.iltas.size() > 0)) {
    ntest = false;
    std::cerr
        << "ERROR: Specify either --satit or set sensitivity manually with "
           "--sat <real> !"
        << std::endl;
    exit(1);
  }

  return (ntest);
}
