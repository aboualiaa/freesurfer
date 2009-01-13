/**
 * @file  vtkKWQdecApp.h
 * @brief Command line parsing and registry
 *
 * Application code that parses the command line options and passes
 * them to the main window. Also gets registry options pertaining to
 * the window size.
 */
/*
 * Original Author: Kevin Teich
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2009/01/13 02:40:10 $
 *    $Revision: 1.1.2.3 $
 *
 * Copyright (C) 2002-2009,
 * The General Hospital Corporation (Boston, MA). 
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */


#ifndef __vtkKWQdecApp_h
#define __vtkKWQdecApp_h

#include <string>
#include "vtkKWApplication.h"
#include "vtkSmartPointer.h"

class vtkKWDialog;
class vtkKWQdecWindow;
class vtkKWTopLevel;

class vtkKWQdecApp : public vtkKWApplication {

public:

  static vtkKWQdecApp* New ();
  vtkTypeRevisionMacro ( vtkKWQdecApp, vtkKWApplication );

  // Override to show our window.
  virtual void Start ( int argc, char* argv[] );
  
  // Override to clean up.
  virtual int Exit ();

  // Override to add text to 'Help About' box
  virtual void AddAboutText ( ostream &os );

  // Thse are just passed right to the window.
  void LoadDataTable ( const char* ifnDataTable );
  void LoadProjectFile ( const char* ifnProject );
  void LoadSurface ( const char* ifnSurface );
  void LoadGDFFile ( const char* ifnGDF );
  void LoadSurfaceScalars ( const char* ifnScalars );
  void LoadSurfaceCurvatureScalars ( const char* ifnScalars );
  void LoadAnnotation ( const char* ifnAnnotation );
  void LoadSurfaceOverlayScalars ( const char* ifnScalars,
				   const char* ifnColors );
  void LoadLabel ( const char* ifnLabel );

  // Display an error message dialog and log the message.
  virtual void ErrorMessage ( const char* isMessage );
  
  // Show our help dialog.
  void DisplayHelpDialog ( vtkKWTopLevel* iTop );

protected:

  vtkKWQdecApp ();
  ~vtkKWQdecApp ();

  //BTX

  // The main application window.
  vtkSmartPointer<vtkKWQdecWindow> mWindow;

  // Help dialog.
  vtkSmartPointer<vtkKWDialog> mDlogHelp;

  //ETX
};

#endif
