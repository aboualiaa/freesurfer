/**
 * @file  mri_map_cpdat.c
 * @brief map a control.dat file (manual edits) to another space
 *
 */
/*
 * Original Author: Martin Reuter
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2014/01/21 23:07:41 $
 *    $Revision: 1.6 $
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

#include "ctrpoints.h"
#include "error.h"
#include "version.h"

struct Parameters {
  std::string cpin;
  std::string cpout;
  std::string lta;
  std::string subjectlistfile;
  std::string subject;
  int         ToMNI305;
  int         FromMNI305;
};
static struct Parameters P = {"", "", "", "", "", 0, 0};

static int  get_option(int argc, char *argv[], Parameters &P);
static void usage_exit(int code);

const char *Progname = NULL;
static LTA *LTAloadTalairachXFM(const char *subject);
static LTA *LTAloadTalairachXFMInv(const char *subject);

int main(int argc, char *argv[]) {
  int  nargs;
  LTA *lta = nullptr;

  // Default initialization
  nargs = handleVersionOption(argc, argv, "mri_map_cpdat");
  if (nargs && argc - nargs == 1) {
    exit(0);
  }
  argc -= nargs;
  Progname = argv[0];
  argc--;
  argv++;
  ErrorInit(NULL, NULL, NULL);

  for (; argc > 1 && ISOPTION(*argv[0]); argc--, argv++) {
    nargs = get_option(argc, argv, P);
    argc -= nargs;
    argv += nargs;
  }

  if (P.cpout == "") {
    printf("ERROR: need to specify an output control file\n");
    usage_exit(1);
  }

  if (P.subjectlistfile != "") {
    int     nctrtot;
    MPoint *ctr;
    ctr = GetTalControlPointsSFile(P.subjectlistfile.c_str(), &nctrtot);
    if (ctr == nullptr)
      exit(1);
    printf("Found %d control points total\n", nctrtot);
    MRIwriteControlPoints(ctr, nctrtot, 0, P.cpout.c_str());
    exit(0);
  }

  if (P.cpin == "") {
    printf("ERROR: need to specify an input control file\n");
    usage_exit(1);
  }

  if (P.lta == "" && P.subject == "") {
    printf("ERROR: need to specify an LTA file\n");
    usage_exit(1);
  }

  int count      = 0;
  int useRealRAS = 0;

  // read ctrl points
  MPoint *pArray = MRIreadControlPoints(P.cpin.c_str(), &count, &useRealRAS);
  if (pArray == nullptr)
    exit(1);

  // read lta
  std::cout << "Reading LTA" << std::endl;
  if (P.subject == "")
    lta = LTAread(P.lta.c_str());
  else {
    if (P.ToMNI305)
      lta = LTAloadTalairachXFM(P.subject.c_str());
    if (P.FromMNI305)
      lta = LTAloadTalairachXFMInv(P.subject.c_str());
  }

  if (lta == nullptr)
    exit(1);
  LTAprint(stdout, lta);

  // map ctrl points
  std::cout << "Mapping control points..." << std::endl;
  MPoint *mappedArray =
      MRImapControlPoints(pArray, count, useRealRAS, nullptr, lta);
  // write ctrl points
  std::cout << "Writing control points..." << std::endl;
  MRIwriteControlPoints(mappedArray, count, useRealRAS, P.cpout.c_str());

  // cleanup
  free(pArray);
  free(mappedArray);

  return (0);
}

static int get_option(int argc, char *argv[], Parameters &P) {
  int   nargs = 0;
  char *option;

  option = argv[0] + 1; /* past '-' */
  if (option[0] == '-') {
    option = option + 1; // remove second '-'
  }
  StrUpper(option);

  if (!strcmp(option, "IN")) {
    P.cpin = std::string(argv[1]);
    nargs  = 1;
    std::cout << "--in:  Using " << P.cpin << " as input file." << std::endl;
  } else if (!strcmp(option, "OUT")) {
    P.cpout = std::string(argv[1]);
    nargs   = 1;
    std::cout << "--out: Using " << P.cpout << " as output file." << std::endl;
  } else if (!strcmp(option, "LTA")) {
    P.lta = std::string(argv[1]);
    nargs = 1;
    std::cout << "--lta: Using " << P.lta << " as transform." << std::endl;
  } else if (!strcmp(option, "SLF")) {
    P.subjectlistfile = std::string(argv[1]);
    nargs             = 1;
    std::cout << "--slf: Using " << P.subjectlistfile << " as subjectlist."
              << std::endl;
  } else if (!strcmp(option, "TOMNI305")) {
    P.subject  = std::string(argv[1]);
    P.ToMNI305 = 1;
    nargs      = 1;
  } else if (!strcmp(option, "FROMMNI305")) {
    P.subject    = std::string(argv[1]);
    P.FromMNI305 = 1;
    nargs        = 1;
  } else {
    std::cerr << std::endl
              << std::endl
              << "ERROR: Option: " << argv[0] << " unknown !! " << std::endl
              << std::endl;
    exit(1);
  }

  return (nargs);
}

static void usage_exit(int code) {
  printf("\n%s <Required Arguments>\n\n", Progname);
  printf("  Maps a control.dat file to a different space using an LTA\n\n");
  printf("  -in  <file>      input  control point txt file\n");
  printf("  -out <file>      output control point txt file\n");
  printf("  -lta <file>      lta transform file to be applied\n");
  printf("  -tomni305 <subject>  get lta from talairach.xfm\n");
  printf("  -frommni305 <subject>  get lta from talairach.xfm\n");
  printf(
      "  -slf subjectlistfile : maps all control points from all subjects "
      "listed in the text/ascii subjectlistfile to mni305 (talairach) space\n");
  printf("  \n");
  exit(code);
}

static LTA *LTAloadTalairachXFM(const char *subject) {
  LTA *lta;
  char tmpstr[2000];
  MRI *fsaorig, *orig;

  sprintf(tmpstr, "%s/%s/mri/transforms/talairach.xfm", getenv("SUBJECTS_DIR"),
          subject);
  lta = LTAread(tmpstr);
  if (lta == nullptr)
    return (nullptr);

  sprintf(tmpstr, "%s/%s/mri/orig.mgz", getenv("SUBJECTS_DIR"), subject);
  orig = MRIreadHeader(tmpstr, MRI_VOLUME_TYPE_UNKNOWN);
  if (orig == nullptr)
    return (nullptr);

  sprintf(tmpstr, "%s/fsaverage/mri/orig.mgz", getenv("SUBJECTS_DIR"));
  fsaorig = MRIreadHeader(tmpstr, MRI_VOLUME_TYPE_UNKNOWN);
  if (fsaorig == nullptr)
    return (nullptr);

  LTAsetVolGeom(lta, orig, fsaorig);

  MRIfree(&orig);
  MRIfree(&fsaorig);

  return (lta);
}

static LTA *LTAloadTalairachXFMInv(const char *subject) {
  LTA *lta;
  char tmpstr[2000];
  MRI *fsaorig, *orig;

  sprintf(tmpstr, "%s/%s/mri/transforms/talairach.xfm", getenv("SUBJECTS_DIR"),
          subject);
  lta = LTAread(tmpstr);
  if (lta == nullptr)
    return (nullptr);
  lta->xforms[0].m_L = MatrixInverse(lta->xforms[0].m_L, lta->xforms[0].m_L);

  sprintf(tmpstr, "%s/%s/mri/orig.mgz", getenv("SUBJECTS_DIR"), subject);
  orig = MRIreadHeader(tmpstr, MRI_VOLUME_TYPE_UNKNOWN);
  if (orig == nullptr)
    return (nullptr);

  sprintf(tmpstr, "%s/fsaverage/mri/orig.mgz", getenv("SUBJECTS_DIR"));
  fsaorig = MRIreadHeader(tmpstr, MRI_VOLUME_TYPE_UNKNOWN);
  if (fsaorig == nullptr)
    return (nullptr);

  LTAsetVolGeom(lta, fsaorig, orig);

  MRIfree(&orig);
  MRIfree(&fsaorig);

  return (lta);
}
