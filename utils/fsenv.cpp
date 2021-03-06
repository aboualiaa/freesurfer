/**
 * @brief Utils to get, set, and print freesurfer environment vars
 *
 */
/*
 * Original Author: Doug Greve
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

#include "fsenv.h"
#include "mri.h"
#include "utils.h"
#include "version.h"
#include <cstdio>
#include <cstdlib>
#include <sys/utsname.h>
#include <unistd.h>

FSENV *FSENVgetenv() {
  FSENV *        fsenv;
  char *         pc, tmpstr[2000];
  struct utsname uts;

  fsenv = (FSENV *)calloc(sizeof(FSENV), 1);

  pc = getenv("FREESURFER_HOME");
  if (pc == nullptr) {
    printf("FREESURFER_HOME not defined\n");
    return (nullptr);
  }
  fsenv->FREESURFER_HOME = strcpyalloc(pc);

  pc = getenv("SUBJECTS_DIR");
  if (pc == nullptr) {
    printf("SUBJECTS_DIR not defined\n");
    return (nullptr);
  }
  fsenv->SUBJECTS_DIR = strcpyalloc(pc);
  fsenv->user         = strcpyalloc(VERuser());

  // Current working directory
  if (!getcwd(tmpstr, 2000)) {
    printf("ERROR: getcwd returned no path");
  }
  fsenv->cwd = strcpyalloc(tmpstr);

  // Kernel information
  uname(&uts);
  fsenv->sysname  = strcpyalloc(uts.sysname);
  fsenv->machine  = strcpyalloc(uts.machine);
  fsenv->hostname = strcpyalloc(uts.nodename);

  // Load the default color table
  sprintf(tmpstr, "%s/FreeSurferColorLUT.txt", fsenv->FREESURFER_HOME);
  fsenv->ctab = CTABreadASCII(tmpstr);
  if (fsenv->ctab == nullptr) {
    printf("ERROR: reading %s\n", tmpstr);
    return (nullptr);
  }

  // Get time and date at the time this function was called
  fsenv->date = VERcurTimeStamp();

  pc = getenv("FREESURFER_TMP_DIR");
  if (pc != nullptr)
    fsenv->tmpdir = strcpyalloc(pc);
  else
    fsenv->tmpdir = strcpyalloc("/tmp");

  // for DWI when dicoms are read
  pc = getenv("FS_DESIRED_BVEC_SPACE");
  if (pc != nullptr) {
    int b;
    sscanf(pc, "%d", &b);
    if (b != BVEC_SPACE_SCANNER && b != BVEC_SPACE_VOXEL) {
      printf("ERROR: FS_DESIRED_BVEC_SPACE = %s, must be %d or %d\n", pc,
             BVEC_SPACE_SCANNER, BVEC_SPACE_VOXEL);
      return (nullptr);
    }
    fsenv->desired_bvec_space = b;
  } else
    fsenv->desired_bvec_space = BVEC_SPACE_VOXEL;

  return (fsenv);
}
/*-----------------------------------------------*/
int FSENVfree(FSENV **ppenv) {
  FSENV *env = *ppenv;
  free(env->FREESURFER_HOME);
  free(env->SUBJECTS_DIR);
  free(env->user);
  free(env->date);
  free(env->cwd);
  free(env->hostname);
  free(env->sysname);
  free(env->machine);
  free(env->tmpdir);
  CTABfree(&env->ctab);
  free(*ppenv);
  *ppenv = nullptr;
  return (0);
}

/*-----------------------------------------------*/
int FSENVprintenv(FILE *fp, FSENV *env) {
  fprintf(fp, "FREESURFER_HOME %s\n", env->FREESURFER_HOME);
  fprintf(fp, "SUBJECTS_DIR %s\n", env->SUBJECTS_DIR);
  fprintf(fp, "user %s\n", env->user);
  fprintf(fp, "date %s\n", env->date);
  fprintf(fp, "cwd %s\n", env->cwd);
  fprintf(fp, "hostname %s\n", env->hostname);
  fprintf(fp, "sysname %s\n", env->sysname);
  fprintf(fp, "machine %s\n", env->machine);
  return (0);
}

/*-----------------------------------------------*/
char *FSENVgetSUBJECTS_DIR() {
  char *pc = getenv("SUBJECTS_DIR");
  if (pc == nullptr) {
    printf("FSENVgetSUBJECTS_DIR: SUBJECTS_DIR not defined\n");
    return (nullptr);
  }
  return strcpyalloc(pc);
}

/*-----------------------------------------------*/
int FSENVsetSUBJECTS_DIR(char *SUBJECTS_DIR) {
  setenv("SUBJECTS_DIR", SUBJECTS_DIR, 1);
  return (0);
}
