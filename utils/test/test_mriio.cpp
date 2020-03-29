/**
 * @file  test_mriio.cpp
 * @brief mriio test routines
 *
 */
/*
 * Original Author: Y. Tosa
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:56 $
 *    $Revision: 1.4 $
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

#include <iostream>
#include <sstream>
#include <stdexcept>

extern "C" {
#include "NrrdIO.h"
#include "mri.h"
}

const char *Progname = NULL;

#define Assert(x, s)                                                           \
  if (!(x)) {                                                                  \
    stringstream ss;                                                           \
    ss << "Line " << __LINE__ << ": " << s;                                    \
    throw std::runtime_error(ss.str());                                        \
  }

class MriioTester {
public:
  void TestNrrdIO();
};

void MriioTester::TestNrrdIO() {
  std::cerr << "Check that mriNrrdRead reads foolc.nrrd...";
  Assert(MRIreadType("test_mriio_data/foolc.nrrd", NRRD_FILE) != NULL,
         "failed.");
  std::cerr << "passed." << std::endl;
}

int main(int argc, char **argv) {
  // Progname = argv[0];

  std::cerr << "Beginning tests..." << std::endl;

  try {

    MriioTester tester;
    tester.TestNrrdIO();

  } catch (runtime_error &e) {
    std::cerr << "failed " << std::endl
              << "exception: " << e.what() << std::endl;
    exit(1);
  } catch (...) {
    std::cerr << "failed" << std::endl;
    exit(1);
  }

  std::cerr << "Success" << std::endl;

  exit(0);
}
