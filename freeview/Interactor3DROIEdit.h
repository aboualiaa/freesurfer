/**
 * @file  Interactor3DROIEdit.h
 * @brief Interactor for navigating in 3D render view.
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2016/12/11 16:04:03 $
 *    $Revision: 1.2 $
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
 *
 */

#ifndef Interactor3DROIEdit_h
#define Interactor3DROIEdit_h

#include "Interactor3D.h"

class Interactor3DROIEdit : public Interactor3D {
public:
  Interactor3DROIEdit(QObject *parent);

  virtual bool ProcessMouseDownEvent(QMouseEvent *event, RenderView *view);
  virtual bool ProcessMouseMoveEvent(QMouseEvent *event, RenderView *view);
  virtual bool ProcessMouseUpEvent(QMouseEvent *event, RenderView *view);

protected:
  bool m_bEditing;
  int  m_nPrevVertex;
};

#endif
