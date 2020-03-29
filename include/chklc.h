/**
 * @file  chklc.h
 * @brief .license file checker
 *
 */
/*
 * Original Author:
 * CVS Revision Info:
 *    $Author: zkaufman $
 *    $Date: 2015/01/13 21:00:09 $
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

#ifndef CHKLC_H
#define CHKLC_H

void chklc();
int  chklc2(char *msg);

#ifndef Darwin
void cmp_glib_version();
#endif

#endif
