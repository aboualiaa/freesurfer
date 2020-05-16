/**
 * @brief 2D slice view
 *
 */
/*
 * Original Author: Ruopeng Wang
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
#include "RenderView2D.h"
#include "LayerCollection.h"
#include "LayerLineProfile.h"
#include "LayerMRI.h"
#include "LayerSurface.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
// #undef isfinite
#include "Annotation2D.h"
#include "Contour2D.h"
#include "Cursor2D.h"
#include "Interactor2DMeasure.h"
#include "Interactor2DNavigate.h"
#include "Interactor2DPointSetEdit.h"
#include "Interactor2DROIEdit.h"
#include "Interactor2DVolumeCrop.h"
#include "Interactor2DVoxelEdit.h"
#include "LayerPropertyMRI.h"
#include "MyUtils.h"
#include "Region2DRectangle.h"
#include "VolumeCropper.h"
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkImageActor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>

RenderView2D::RenderView2D(QWidget *parent) : RenderView(parent) {
  m_renderer->GetActiveCamera()->ParallelProjectionOn();
  m_contour2D    = new Contour2D(this);
  m_cursor2D     = new Cursor2D(this);
  m_annotation2D = new Annotation2D(this);
  m_selection2D  = new Region2DRectangle(this);
  m_selection2D->SetEnableStats(false);
  connect(m_cursor2D, SIGNAL(Updated()), this, SLOT(RequestRedraw()));
  connect(m_annotation2D, SIGNAL(Updated()), this, SLOT(RequestRedraw()));
  connect(this, SIGNAL(ViewChanged()), this, SLOT(Update2DOverlay()));

  m_interactorNavigate     = new Interactor2DNavigate(this);
  m_interactorMeasure      = new Interactor2DMeasure(this);
  m_interactorVoxelEdit    = new Interactor2DVoxelEdit(this);
  m_interactorROIEdit      = new Interactor2DROIEdit(this);
  m_interactorPointSetEdit = new Interactor2DPointSetEdit(this);
  m_interactorVolumeCrop   = new Interactor2DVolumeCrop(this);
  connect(m_interactorMeasure, SIGNAL(Error(QString)), this,
          SLOT(OnInteractorError(QString)));
  connect(m_interactorVoxelEdit, SIGNAL(Error(QString)), this,
          SLOT(OnInteractorError(QString)));
  connect(m_interactorROIEdit, SIGNAL(Error(QString)), this,
          SLOT(OnInteractorError(QString)));
  connect(m_interactorPointSetEdit, SIGNAL(Error(QString)), this,
          SLOT(OnInteractorError(QString)));
  connect(m_interactorNavigate, SIGNAL(CursorLocationClicked()), this,
          SIGNAL(CursorLocationClicked()));
  SetInteractionMode(IM_Navigate);
  m_dPreSlicePosition = -1e10;
}

void RenderView2D::SetViewPlane(int nPlane) {
  m_nViewPlane = nPlane;
  m_contour2D->SetPlane(nPlane);
}

void RenderView2D::SetInteractionMode(int nMode) {
  RenderView::SetInteractionMode(nMode);

  switch (nMode) {
  case IM_Measure:
    m_interactor = m_interactorMeasure;
    break;
  case IM_VoxelEdit:
  case IM_ReconEdit:
    m_interactor = m_interactorVoxelEdit;
    break;
  case IM_ROIEdit:
    m_interactor = m_interactorROIEdit;
    break;
  case IM_PointSetEdit:
    m_interactor = m_interactorPointSetEdit;
    break;
  case IM_VolumeCrop:
    m_interactor = m_interactorVolumeCrop;
    break;
  default:
    m_interactor = m_interactorNavigate;
    break;
  }
}

void RenderView2D::RefreshAllActors(bool bForScreenShot) {
  MainWindow *       mainwnd = MainWindow::GetMainWindow();
  SettingsScreenshot setting = mainwnd->GetScreenShotSettings();
  LayerCollection *  lc      = mainwnd->GetLayerCollection("MRI");
  m_renderer->RemoveAllViewProps();
  lc->Append2DProps(m_renderer, m_nViewPlane);

  LayerMRI *mri = NULL;
  if (!lc->IsEmpty()) {
    mri = (LayerMRI *)lc->GetLayer(0);
    mri->Remove2DProps(m_renderer, m_nViewPlane);

    m_renderer->AddViewProp(m_contour2D->GetActor());

    mri->Append2DProps(m_renderer, m_nViewPlane);

    // add annotation
    if (!bForScreenShot || !setting.HideCoords) {
      m_annotation2D->AppendAnnotations(m_renderer);
    }
    if (!bForScreenShot || !setting.HideScaleBar) {
      m_annotation2D->AppendAnnotations(m_renderer, true);
    }

    m_selection2D->AppendProp(m_renderer);

    // add scalar bar
    m_renderer->AddViewProp(m_actorScalarBar);
  }

  if (!mainwnd->IsEmpty()) {
    if (!bForScreenShot || !setting.HideCursor) {
      m_cursor2D->AppendActor(m_renderer);
    }
  }

  mainwnd->GetLayerCollection("ROI")->Append2DProps(m_renderer, m_nViewPlane);
  mainwnd->GetLayerCollection("FCD")->Append2DProps(m_renderer, m_nViewPlane);
  mainwnd->GetLayerCollection("Surface")->Append2DProps(m_renderer,
                                                        m_nViewPlane);
  mainwnd->GetLayerCollection("PointSet")
      ->Append2DProps(m_renderer, m_nViewPlane);
  mainwnd->GetLayerCollection("Supplement")
      ->Append2DProps(m_renderer, m_nViewPlane);

  mainwnd->GetVolumeCropper()->Append2DProps(m_renderer, m_nViewPlane);

  // add regions
  for (int i = 0; i < m_regions.size(); i++) {
    m_regions[i]->AppendProp(m_renderer);
  }

  // add focus frame
  if (!bForScreenShot) {
    m_renderer->AddViewProp(m_actorFocusFrame);
  }

  if (!lc->IsEmpty()) {
    double *orig = lc->GetWorldOrigin();
    double *size = lc->GetWorldSize();

    m_renderer->ResetCameraClippingRange(orig[0], orig[0] + size[0], orig[1],
                                         orig[1] + size[1], orig[2],
                                         orig[2] + size[2]);
  }
  //  m_renderer->ResetCameraClippingRange();
  RenderView::RefreshAllActors(bForScreenShot);
}

void RenderView2D::UpdateViewByWorldCoordinate() {
  vtkCamera *cam = m_renderer->GetActiveCamera();
  double     wcenter[3];
  for (int i = 0; i < 3; i++) {
    wcenter[i] = m_dWorldOrigin[i] + m_dWorldSize[i] / 2;
  }
  cam->SetFocalPoint(wcenter);
  double len =
      qMax(m_dWorldSize[0], qMax(m_dWorldSize[1], m_dWorldSize[2])) * 5;
  switch (m_nViewPlane) {
  case 0:
    cam->SetPosition(wcenter[0] + len, wcenter[1], wcenter[2]);
    cam->SetViewUp(0, 0, 1);
    break;
  case 1:
    cam->SetPosition(wcenter[0], wcenter[1] + len, wcenter[2]);
    cam->SetViewUp(0, 0, 1);
    break;
  case 2:
    cam->SetPosition(wcenter[0], wcenter[1], wcenter[2] - len);
    break;
  }
  //  m_renderer->ResetCameraClippingRange();
  cam->SetParallelScale(
      qMax(qMax(m_dWorldSize[0], m_dWorldSize[1]), m_dWorldSize[2]) / 2);
}

void RenderView2D::UpdateAnnotation() {
  m_annotation2D->Update(m_renderer, m_nViewPlane);
  RequestRedraw();
}

void RenderView2D::UpdateMouseRASPosition(int posX, int posY) {
  LayerCollection *lc = MainWindow::GetMainWindow()->GetLayerCollection("MRI");
  if (!lc) {
    return;
  }

  double pos[3];
  MousePositionToRAS(posX, posY, pos);

  lc->SetCurrentRASPosition(pos);
}

void RenderView2D::UpdateCursorRASPosition(int posX, int posY,
                                           bool bSnapToVertex) {
  LayerCollection *lc = MainWindow::GetMainWindow()->GetLayerCollection("MRI");
  if (!lc) {
    return;
  }

  double pos[3];
  MousePositionToRAS(posX, posY, pos);

  if (bSnapToVertex) {
    QList<Layer *> layers = MainWindow::GetMainWindow()->GetLayers("Surface");
    foreach (Layer *layer, layers) {
      LayerSurface *surf = qobject_cast<LayerSurface *>(layer);
      if (surf->IsVisible()) {
        int nVertex = surf->GetVertexIndexAtTarget(pos, NULL);
        if (nVertex >= 0) {
          if (surf->GetTargetAtVertex(nVertex, pos))
            break;
        }
      }
    }
  }

  MainWindow::GetMainWindow()->SetSlicePosition(pos);
  lc->SetCursorRASPosition(pos);
}

void RenderView2D::Update2DOverlay() {
  m_cursor2D->Update();
  m_selection2D->Update();
  for (int i = 0; i < m_regions.size(); i++) {
    m_regions[i]->Update();
  }

  double slicePos[3];
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetSlicePosition(
      slicePos);
  m_contour2D->UpdateSliceLocation(slicePos[m_nViewPlane]);
}

void RenderView2D::OnSlicePositionChanged(bool bCenter) {
  double slicePos[3];
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetSlicePosition(
      slicePos);

  if (m_dPreSlicePosition != slicePos[m_nViewPlane]) {
    for (int i = 0; i < m_regions.size(); i++) {
      m_regions[i]->UpdateSlicePosition(m_nViewPlane, slicePos[m_nViewPlane]);
    }

    ResetCameraClippingRange();
    m_dPreSlicePosition = slicePos[m_nViewPlane];
  }
  m_cursor2D->SetPosition(slicePos);
  Update2DOverlay();
  UpdateAnnotation();

  if (bCenter) {
    double x, y, z;
    WorldToViewport(slicePos[0], slicePos[1], slicePos[2], x, y, z);
#if VTK_MAJOR_VERSION > 7
    if (devicePixelRatio() > 1) {
      x /= devicePixelRatio();
      y /= devicePixelRatio();
    }
#endif
    if (!rect().contains(QPoint(x, y)))
      this->CenterAtCursor();
  }

  RenderView::OnSlicePositionChanged(bCenter);
}

void RenderView2D::CenterAtCursor() {
  double slicePos[3];
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetSlicePosition(
      slicePos);
  PanToWorld(slicePos);
  Update2DOverlay();
  UpdateAnnotation();
  RequestRedraw();
}

void RenderView2D::MousePositionToRAS(int posX, int posY, double *pos) {
  pos[0] = posX;
  pos[1] = rect().height() - posY;
  pos[2] = 0;
#if VTK_MAJOR_VERSION > 7
  if (devicePixelRatio() > 1) {
    pos[0] = pos[0] * devicePixelRatio();
    pos[1] = pos[1] * devicePixelRatio();
  }
#endif
  m_renderer->ViewportToNormalizedViewport(pos[0], pos[1]);
  m_renderer->NormalizedViewportToView(pos[0], pos[1], pos[2]);
  m_renderer->ViewToWorld(pos[0], pos[1], pos[2]);

  double slicePos[3];
  MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetSlicePosition(
      slicePos);
  pos[m_nViewPlane] = slicePos[m_nViewPlane];
}

LayerMRI *RenderView2D::GetFirstNonLabelVolume() {
  QList<Layer *> layers =
      MainWindow::GetMainWindow()->GetLayerCollection("MRI")->GetLayers();
  for (int i = 0; i < layers.size(); i++) {
    LayerMRI *layer = (LayerMRI *)layers[i];
    if (layer->IsVisible() &&
        layer->GetProperty()->GetColorMap() != LayerPropertyMRI::LUT) {
      return layer;
    }
  }
  return NULL;
}

void RenderView2D::StartSelection(int nX, int nY) {
  m_selection2D->SetRect(nX, nY, 1, 1);
  m_selection2D->Show(true);
}

void RenderView2D::UpdateSelection(int nX, int nY) {
  m_selection2D->SetBottomRight(nX, nY);
}

void RenderView2D::StopSelection() {
  m_selection2D->Show(false);
  QList<Layer *> layers = MainWindow::GetMainWindow()->GetSelectedLayers("MRI");
  if (layers.size() < 2) {
    LayerMRI *sel_mri =
        (LayerMRI *)(layers.isEmpty()
                         ? MainWindow::GetMainWindow()->GetActiveLayer("MRI")
                         : layers[0]);
    if (sel_mri && !sel_mri->IsWindowAdjustable())
      sel_mri = NULL;
    QList<Layer *> vols = MainWindow::GetMainWindow()->GetLayers("MRI");
    for (int i = 0; i < vols.size(); i++) {
      LayerMRI *mri = (LayerMRI *)vols[i];
      if (mri->IsWindowAdjustable()) {
        if (sel_mri == NULL || sel_mri == mri || mri->IsObscuring()) {
          layers.clear();
          layers << mri;
          break;
        }
      }
    }
  }

  for (int i = 0; i < layers.size(); i++) {
    LayerMRI *layer = qobject_cast<LayerMRI *>(layers[i]);
    if (layer) {
      double range[2], m_dPt0[3], m_dPt2[3];
      m_selection2D->GetWorldPoint(0, m_dPt0);
      m_selection2D->GetWorldPoint(2, m_dPt2);
      int nColorMap = layer->GetProperty()->GetColorMap();
      if (layer->IsVisible() && nColorMap != LayerPropertyMRI::LUT &&
          nColorMap != LayerPropertyMRI::DirectionCoded &&
          layer->GetVoxelValueRange(m_dPt0, m_dPt2, m_nViewPlane, range)) {
        switch (nColorMap) {
        case LayerPropertyMRI::Grayscale:
          layer->GetProperty()->SetMinMaxGrayscaleWindow(range[0], range[1]);
          break;
        case LayerPropertyMRI::Heat:
          layer->GetProperty()->SetHeatScale(
              range[0], (range[1] - range[0]) / 2, range[1]);
          break;
        default:
          layer->GetProperty()->SetMinMaxGenericThreshold(range[0], range[1]);
          break;
        }
      }
    }
  }
}

Region2D *RenderView2D::GetRegion(int nX, int nY, int *index_out) {
  for (int i = m_regions.size() - 1; i >= 0; i--) {
    if (m_regions[i]->Contains(nX, nY, index_out)) {
      return m_regions[i];
    }
  }
  return NULL;
}

void RenderView2D::AddRegion(Region2D *region) {
  for (int i = 0; i < m_regions.size(); i++) {
    if (m_regions[i] == region) {
      return;
    }
  }

  m_regions.push_back(region);
  emit RegionSelected(region);
  RefreshAllActors();
}

void RenderView2D::DeleteRegion(Region2D *region) {
  for (int i = 0; i < m_regions.size(); i++) {
    if (m_regions[i] == region) {
      m_regions.erase(m_regions.begin() + i);
      emit RegionRemoved(region);
      delete region;
      RefreshAllActors();
      return;
    }
  }
}

void RenderView2D::MoveSlice(int nStep) {
  MainWindow *     mainWnd   = MainWindow::GetMainWindow();
  LayerCollection *lc_mri    = mainWnd->GetLayerCollection("MRI");
  double *         voxelSize = lc_mri->GetWorldVoxelSize();
  LayerMRI *       mri = qobject_cast<LayerMRI *>(lc_mri->GetActiveLayer());
  if (mri) {
    if (!mri->IsVisible()) {
      for (int i = 0; i < lc_mri->GetNumberOfLayers(); i++) {
        if (lc_mri->GetLayer(i)->IsVisible()) {
          mri = qobject_cast<LayerMRI *>(lc_mri->GetLayer(i));
          break;
        }
      }
    }
    voxelSize = mri->GetWorldVoxelSize();
  }
  int nPlane = GetViewPlane();
  mainWnd->OffsetSlicePosition(nPlane, voxelSize[nPlane] * nStep);
  lc_mri->SetCursorRASPosition(lc_mri->GetSlicePosition());
  UpdateAnnotation();
}

void RenderView2D::SyncZoomTo(RenderView2D *view) {
  m_renderer->GetActiveCamera()->SetParallelScale(
      view->m_renderer->GetActiveCamera()->GetParallelScale());
  // PanToWorld( GetCursor2D()->GetPosition() );
  EnsureCursor2DVisible();
  Update2DOverlay();
  UpdateAnnotation();
  RequestRedraw();
}

bool RenderView2D::EnsureCursor2DVisible() {
  double *pos = GetCursor2D()->GetPosition();
  double  x = pos[0], y = pos[1], z = pos[2];
  m_renderer->WorldToView(x, y, z);
  m_renderer->ViewToNormalizedViewport(x, y, z);
  if (x < 0 || x > 1 || y < 0 || y > 1) {
    PanToWorld(pos);
    return true;
  } else {
    return false;
  }
}

void RenderView2D::ZoomAtCursor(int nX, int nY, double factor) {
  double pos[3];
  pos[0] = nX;
  pos[1] = rect().height() - nY;
  pos[2] = 0;
  m_renderer->ViewportToNormalizedViewport(pos[0], pos[1]);
  m_renderer->NormalizedViewportToView(pos[0], pos[1], pos[2]);
  m_renderer->ViewToWorld(pos[0], pos[1], pos[2]);

  // first move the click point to the center of viewport
  PanToWorld(pos);

  // then zoom
  m_renderer->GetActiveCamera()->Zoom(factor);
  Update2DOverlay();
  UpdateAnnotation();
  RequestRedraw();
  EmitZooming();
}

void RenderView2D::resizeEvent(QResizeEvent *event) {
  RenderView::resizeEvent(event);

  Update2DOverlay();
  UpdateAnnotation();
}

void RenderView2D::ShowCoordinateAnnotation(bool bShow) {
  m_annotation2D->Show(bShow);
}

bool RenderView2D::GetShowCoordinateAnnotation() {
  return m_annotation2D->IsVisible();
}

void RenderView2D::SetAutoScaleText(bool b) {
  m_bAutoScaleText = b;
  m_annotation2D->SetAutoScaleText(b);
  for (int i = 0; i < m_regions.size(); i++) {
    m_regions[i]->SetAutoScaleText(b);
  }
  RequestRedraw();
}

void RenderView2D::SetTextSize(int nsize) {
  m_nTextSize = nsize;
  m_annotation2D->SetTextSize(nsize);
  for (int i = 0; i < m_regions.size(); i++) {
    m_regions[i]->SetTextSize(nsize);
  }
  RequestRedraw();
}

bool RenderView2D::SetSliceNumber(int nNum) {
  LayerCollection *lc_mri =
      MainWindow::GetMainWindow()->GetLayerCollection("MRI");

  LayerMRI *mri = (LayerMRI *)lc_mri->GetActiveLayer();
  if (!mri) {
    return false;
  }

  vtkImageData *imagedata = mri->GetImageData();
  int           nPlane    = GetViewPlane();
  int *         dim       = imagedata->GetDimensions();
  if (nNum < 0 || nNum >= dim[nPlane]) {
    return false;
  }

  int    slice[3];
  double pos[3];
  lc_mri->GetSlicePosition(pos);
  mri->TargetToRAS(pos, pos);
  mri->RASToOriginalIndex(pos, slice);
  QString ostr       = mri->GetOrientationString();
  int     nOrigPlane = nPlane;
  char    ch[3][3]   = {"RL", "AP", "IS"};
  for (int i = 0; i < 3; i++) {
    if (ostr[i] == ch[nPlane][0] || ostr[i] == ch[nPlane][1]) {
      nOrigPlane = i;
      break;
    }
  }
  slice[nOrigPlane] = nNum;
  mri->OriginalIndexToRAS(slice, pos);
  mri->RASToTarget(pos, pos);

  MainWindow::GetMainWindow()->SetSlicePosition(pos);
  lc_mri->SetCursorRASPosition(pos);
  return true;
}

void RenderView2D::TriggerContextMenu(QMouseEvent *event) {
  QMenu          menu;
  bool           bShowBar = this->GetShowScalarBar();
  MainWindow *   mainwnd  = MainWindow::GetMainWindow();
  QList<Layer *> layers   = mainwnd->GetLayers("MRI");
  Region2D *     reg      = GetRegion(event->x(), event->y());
  if (reg) {
    QAction *act = new QAction("Duplicate", this);
    act->setData(QVariant::fromValue((QObject *)reg));
    connect(act, SIGNAL(triggered()), this, SLOT(OnDuplicateRegion()));
    menu.addAction(act);
  }
  if (layers.size() > 1) {
    QMenu *       menu2 = menu.addMenu("Show Color Bar");
    QActionGroup *ag    = new QActionGroup(this);
    ag->setExclusive(true);
    foreach (Layer *layer, layers) {
      QAction *act = new QAction(layer->GetName(), this);
      act->setCheckable(true);
      act->setChecked(bShowBar && layer == m_layerScalarBar);
      act->setData(QVariant::fromValue((QObject *)layer));
      menu2->addAction(act);
      ag->addAction(act);
    }
    connect(ag, SIGNAL(triggered(QAction *)), this,
            SLOT(SetScalarBarLayer(QAction *)));
  }

  if (!layers.isEmpty()) {
    if (!menu.actions().isEmpty() && layers.size() == 1)
      menu.addSeparator();

    if (layers.size() == 1) {
      LayerMRI *mri = (LayerMRI *)layers.first();
      double    val = mri->GetVoxelValue(mri->GetSlicePosition());
      QAction * act =
          new QAction(QString("Copy Voxel Value  (%1)").arg(val), this);
      act->setProperty("voxel_value", val);
      connect(act, SIGNAL(triggered()), SLOT(OnCopyVoxelValue()));
      menu.addAction(act);
    } else {
      QMenu *menu2 = menu.addMenu("Copy Voxel Value");
      foreach (Layer *layer, layers) {
        LayerMRI *mri = (LayerMRI *)layer;
        double    val = mri->GetVoxelValue(layer->GetSlicePosition());
        QAction * act = new QAction(
            layer->GetName() + "  (" + QString::number(val) + ")", this);
        act->setProperty("voxel_value", val);
        connect(act, SIGNAL(triggered()), SLOT(OnCopyVoxelValue()));
        menu2->addAction(act);
      }
    }
  }

  LayerSurface *surf = (LayerSurface *)mainwnd->GetActiveLayer("Surface");
  if (surf && surf->IsContralateralPossible()) {
    if (!menu.actions().isEmpty())
      menu.addSeparator();
    QAction *act = new QAction("Go To Contralateral Point", this);
    menu.addAction(act);
    connect(act, SIGNAL(triggered()), mainwnd, SLOT(GoToContralateralPoint()));
  }

  if (!mainwnd->IsEmpty() && mainwnd->GetMainView() == this) {
    menu.addSeparator();
    QAction *action = new QAction("Copy", this);
    connect(action, SIGNAL(triggered(bool)), mainwnd, SLOT(OnCopyView()));
    menu.addAction(action);
    menu.addAction(mainwnd->ui->actionSaveScreenshot);
  }

  if (!menu.actions().isEmpty())
    menu.exec(event->globalPos());
}

void RenderView2D::OnDuplicateRegion() {
  QAction *act = qobject_cast<QAction *>(sender());
  if (!act)
    return;

  Region2D *reg = qobject_cast<Region2D *>(act->data().value<QObject *>());
  if (reg) {
    reg = reg->Duplicate(this);
    if (reg) {
      reg->Offset(5, 5);
      AddRegion(reg);
    }
  }
}

void RenderView2D::OnInteractorError(const QString &msg) {
  QMessageBox::warning(this, "Error", msg);
}

bool RenderView2D::PickLineProfile(int x, int y) {
  QList<LayerLineProfile *> lineprofs;
  QList<Layer *> layers = MainWindow::GetMainWindow()->GetLayers("Supplement");
  foreach (Layer *layer, layers) {
    if (layer->IsTypeOf("LineProfile")) {
      LayerLineProfile *l = qobject_cast<LayerLineProfile *>(layer);
      if (l->GetPlane() == this->m_nViewPlane)
        lineprofs << l;
    }
  }
  if (lineprofs.isEmpty())
    return false;

  foreach (LayerLineProfile *lp, lineprofs) {
    int nId = this->PickCell(lp->GetLineProfileActor(), x, y);
    if (nId >= 0) {
      nId /= 6;
      emit LineProfileIdPicked(lp, nId);
      lp->SetActiveLineId(nId);
      return true;
    }
  }
  return false;
}

void RenderView2D::OnCopyVoxelValue() {
  if (sender())
    QApplication::clipboard()->setText(
        sender()->property("voxel_value").toString());
}
