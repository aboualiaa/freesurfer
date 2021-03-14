#ifndef OrientationPlanesFromParcellationFilter_h
#define OrientationPlanesFromParcellationFilter_h

#include "itkConstNeighborhoodIterator.h"
#include "itkImageToImageFilter.h"
#include "mri.h"
#include "vtkCenterOfMass.h"
#include "vtkMath.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include <vnl/vnl_cross.h>
#include <vnl/vnl_vector.h>

template <class TInputImage, class TOutputImage>
class OrientationPlanesFromParcellationFilter
    : public itk::ImageToImageFilter<TInputImage, TOutputImage> {
public:
  using Self       = OrientationPlanesFromParcellationFilter;
  using Superclass = itk::ImageToImageFilter<TInputImage, TOutputImage>;

  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  using InputImageType  = TInputImage;
  using OutputImageType = TOutputImage;

  itkNewMacro(Self);
  using VectorType = itk::Vector<float, 3>;
  VectorType GetLeftRight() { return m_LeftRight; }

  VectorType GetUpDown() { return m_UpDown; }

  VectorType GetFrontBack() { return m_FrontBack; }

  void SetBabyMode(bool bb) { this->m_baby = bb; }

  float DistanceToMidline(double x, double y, double z);
  void  GeneratePlanes(MRI *image);

protected:
  void GenerateData() override;

private:
  bool                  m_baby = false;
  itk::Vector<float, 3> m_LeftRight;
  itk::Vector<float, 3> m_UpDown;
  itk::Vector<float, 3> m_FrontBack;
  itk::Vector<float, 3> m_centerLR;
};
#include "OrientationPlanesFromParcellationFilter.txx"
#endif
