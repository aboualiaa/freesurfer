#ifndef INTERACTOR3DPATHEDIT_H
#define INTERACTOR3DPATHEDIT_H

#include "Interactor3D.h"

class Interactor3DPathEdit : public Interactor3D {
public:
  Interactor3DPathEdit(QObject *parent);

  virtual bool ProcessMouseDownEvent(QMouseEvent *event, RenderView *view);
  //  virtual bool ProcessMouseMoveEvent( QMouseEvent* event, RenderView* view );
  virtual bool ProcessMouseUpEvent(QMouseEvent *event, RenderView *view);
  virtual bool ProcessKeyDownEvent(QKeyEvent *event, RenderView *renderview);

protected:
  bool m_bEditAttempted;
};
#endif // INTERACTOR3DPATHEDIT_H
