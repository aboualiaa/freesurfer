/**
 * @file  fsenv.h
 * @brief Load, set freesurfer environment variables
 *
 */
/*
 * Original Author: Doug Greve
 * CVS Revision Info:
 *    $Author: greve $
 *    $Date: 2015/04/16 18:49:31 $
 *    $Revision: 1.7 $
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

#ifndef FSENV_H
#define FSENV_H

#include <sys/utsname.h>

#include "colortab.h"

struct FSENV {
  char *FREESURFER_HOME;
  char *SUBJECTS_DIR;
  char *user;             // current user
  char *date;             // current date and time
  char *cwd;              // current working directory
  char *hostname;         // eg, icebox (same as nodename)
  char *sysname;          // eg, Linux
  char *machine;          // eg, i686
  char *tmpdir;           // folder for temporary files
  COLOR_TABLE *ctab;      // FREESURFER_HOME/FreeSurferColorLUT.txt
  int desired_bvec_space; // for DWI
};

const char *FSENVsrcVersion();
FSENV *FSENVgetenv();
int FSENVprintenv(FILE *fp, FSENV *env);
int FSENVsetSUBJECTS_DIR(char *SUBJECTS_DIR);
char *FSENVgetSUBJECTS_DIR();
int FSENVfree(FSENV **ppenv);

#endif // FSENV_H
