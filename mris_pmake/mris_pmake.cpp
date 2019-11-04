/**
 * @file  mris_pmake.cpp
 * @brief Determine the shortest path on a freesurfer brain curvature map.
 *
 *  SYNPOSIS
 *
 *  mris_pmake <--optionsFile [fileName]> <--dir [workingDir]> \
 *              [--listen] [--listenOnPort <port>]
 *
 *  DESCRIPTION
 *
 *  Determine the shortest path along cost function based on the curvature,
 *  sulcal height, and distance using a surface map generated by freesurfer.
 *  It is hoped that such a shortest path will accurate trace the sulcal
 *  fundus between a specified start and end vertex.
 */
/*
 * Original Authors: Rudolph Pienaar / Christian Haselgrove
 * CVS Revision Info:
 *    $Author: rudolph $
 *    $Date: 2013/01/25 16:39:24 $
 *    $Revision: 1.18 $
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "help.h"
#include "scanopt.h"
#include "dijkstra.h"
#include "C_mpmProg.h"
#include "c_SSocket.h"
#include "pstream.h"
#include "general.h"
#include "asynch.h"
#include "c_vertex.h"
#include "c_label.h"
#include "c_surface.h"

// Some global vars...
extern const option longopts[];
stringstream Gsout("");
char *Gpch_Progname;

const char *Progname = Gpch_Progname;

bool Gb_stdout = true; // Global flag controlling output to
                       //+stdout
string G_SELF = "";    // "My" name
string G_VERSION =     // version
    "$Id: mris_pmake.cpp,v 1.18 2013/01/25 16:39:24 rudolph Exp $";
char pch_buffer[65536];

// "Class"-like globals...
s_env st_env;
string str_asynchComms = "HUP";
c_SSocket_UDP_receive *pCSSocketReceive = nullptr;

int main(int argc, char **ppch_argv) {

  /* ----- initializations ----- */
  Gpch_Progname = strrchr(ppch_argv[0], '/');
  Gpch_Progname = (Gpch_Progname == nullptr ? ppch_argv[0] : Gpch_Progname + 1);
  string str_progname(Gpch_Progname);
  G_SELF = str_progname;
  int ret = 0;

  // "Construct" a default environment structure.
  s_env_nullify(st_env);

  // Prior to completely populating the enter st_env structure, we fill in
  // some defaults to "boot strap" the process.
  st_env.str_workingDir = "./";
  st_env.str_optionsFileName = "options.txt";
  st_env.b_surfacesKeepInSync = true; // This allows us to
                                      //+ propagate changes
                                      //+ in the working
                                      //+ surface to the
                                      //+ auxiliary surface.

  // Set the default cost function in the environment
  // -- Cost function is set when mpmOverlay initialized
  // s_env_costFctSet(&st_env, costFunc_defaultDetermine, e_default);

  // Process command line options
  str_asynchComms = commandLineOptions_process(argc, ppch_argv, st_env);

  // The main functional and event processing loop
  while (str_asynchComms != "TERM") {

    if (str_asynchComms == "HUP" || str_asynchComms == "RUNPROG") {

      ret = system("echo > lock"); // signal a "lock"
                                   //+ semaphore on
                                   //+ the file system

      if (str_asynchComms == "HUP")
        str_asynchComms = s_env_HUP(st_env, &pCSSocketReceive);

      if (str_asynchComms == "RUNPROG") {
        ULOUT(
            lsprintf(st_env.lw, pch_buffer, "Running embedded program '%s'\n",
                     st_env.vstr_mpmProgName[st_env.empmProg_current].c_str()));
        if (st_env.pCmpmProg)
          st_env.pCmpmProg->run();
        else {
          fprintf(stderr, "Warning -- mpmProg has not been created!\n");
          fprintf(stderr, "Have you run 'ENV mpmProg set <X>'?\n");
        }
        if (st_env.b_exitOnDone)
          str_asynchComms = "TERM";
      }
    }

    // Listen on the socket for asynchronous user evernts
    if (pCSSocketReceive && !st_env.b_exitOnDone) {
      SLOUT("Ready\n");
      ULOUT("Listening for socket comms...\n");
      str_asynchComms = asynchEvent_poll(pCSSocketReceive, 5);
      colsprintf(st_env.lw, st_env.rw, pch_buffer, "COMMS: Received",
                 "[ %s ]\n", str_asynchComms.c_str());
      SLOUT(pch_buffer);
      ULOUT(pch_buffer);
      asynchEvent_process(st_env, str_asynchComms);
      SLOUT(colsprintf(st_env.lw, st_env.rw, pch_buffer, "PROCESSED",
                       "[ %s ]\n", str_asynchComms.c_str()));
    } else {
      // Process does not have listen server or has exitOnDone
      // and can exit (otherwise hangs in infinite wait loop without
      // processing new str_asynchComms).
      str_asynchComms = "TERM";
    }
  }

  delete pCSSocketReceive;
  if (st_env.pcsm_syslog) {
    st_env.pcsm_syslog->timer(eSM_stop);
    SLOUT("Ready\n");
  }
  if (st_env.pcsm_userlog) {
    st_env.pcsm_userlog->timer(eSM_stop);
  }
  ret = system("rm -f lock 2>/dev/null"); // "unlock" semaphore
  if (ret)
    printf("WARNING: 'rm' returned non-zero exit status!\n");
  return EXIT_SUCCESS;

} /* end main() */

/* eof */
