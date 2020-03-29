/**
 * @file  VolumeFilterBoundary.cpp
 * @brief Base VolumeFilterBoundary class.
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2011/10/18 18:13:24 $
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
 *
 */

#include "VolumeFilterBoundary.h"
#include "LayerMRI.h"
#include "vtkSimpleLabelEdgeFilter3D.h"
#include <vtkImageData.h>

VolumeFilterBoundary::VolumeFilterBoundary(LayerMRI *input, LayerMRI *output,
                                           QObject *parent)
    : VolumeFilter(input, output, parent) {}

bool VolumeFilterBoundary::Execute() {
  TriggerFakeProgress(100);
  vtkSmartPointer<vtkSimpleLabelEdgeFilter3D> filter =
      vtkSmartPointer<vtkSimpleLabelEdgeFilter3D>::New();
#if VTK_MAJOR_VERSION > 5
  filter->SetInputData(m_volumeInput->GetImageData());
#else
  filter->SetInput(m_volumeInput->GetImageData());
#endif
  filter->Update();
  m_volumeOutput->GetImageData()->DeepCopy(filter->GetOutput());
  return true;
}
