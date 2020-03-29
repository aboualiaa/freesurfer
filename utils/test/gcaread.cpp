/**
 * @file  gcaread.cpp
 * @brief REPLACE_WITH_ONE_LINE_SHORT_DESCRIPTION
 *
 * REPLACE_WITH_LONG_DESCRIPTION_OR_REFERENCE
 */
/*
 * Original Author: REPLACE_WITH_FULL_NAME_OF_CREATING_AUTHOR
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2011/03/02 00:04:55 $
 *    $Revision: 1.3 $
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

//
// gcaread
//

#include <iostream>

extern "C" {
#include "gca.h"
#include "mri.h"

char *Progname = "gcaread";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: gcaread <gcafile>" << std::endl;
    return -1;
  }
  GCA *gca = GCAread(argv[1]);
  if (gca == 0) {
    std::cout << "could not open file " << argv[1] << std::endl;
  } else
    GCAfree(&gca);
}
