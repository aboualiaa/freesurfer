#ifndef __PolylineMeshToVTKPolyDataFilter_h
#define __PolylineMeshToVTKPolyDataFilter_h

#include "itkPolylineCell.h"
#include "itkProcessObject.h"
#include <vtkSmartPointer.h>

#include "vtkCellArray.h"
#include "vtkCellData.h"

class vtkPolyData;

template <class TMesh>
class PolylineMeshToVTKPolyDataFilter : public itk::ProcessObject {
public:
  using Self         = PolylineMeshToVTKPolyDataFilter<TMesh>;
  using Superclass   = ProcessObject;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(PolylineMeshToVTKPolyDataFilter, ProcessObject);

  using MeshType   = TMesh;
  using MeshTraits = typename MeshType::MeshTraits;
  using PointType  = typename MeshType::PointType;
  using PixelType  = typename MeshType::PixelType;

  /** Some convenient typedefs. */
  using MeshPointer     = typename MeshType::Pointer;
  using CellTraits      = typename MeshType::CellTraits;
  using CellIdentifier  = typename MeshType::CellIdentifier;
  using CellType        = typename MeshType::CellType;
  using CellAutoPointer = typename MeshType::CellAutoPointer;
  using PointIdentifier = typename MeshType::PointIdentifier;
  using PointIdIterator = typename CellTraits::PointIdIterator;

  using PointsContainerPointer = typename MeshType::PointsContainerPointer;

  using PointsContainer = typename MeshType::PointsContainer;

  using PolylineCellType = itk::PolylineCell<CellType>;
  using SelfAutoPointer  = typename PolylineCellType::SelfAutoPointer;

  void SetInput(MeshType *mesh);

  vtkSmartPointer<vtkPolyData> GetOutputPolyData() { return m_Output; }

  virtual void Update() { this->GenerateData(); }
  void         SetColor(unsigned char color[]) { this->m_color = color; }

protected:
  PolylineMeshToVTKPolyDataFilter();
  ~PolylineMeshToVTKPolyDataFilter();

  virtual void GenerateData();

private:
  PolylineMeshToVTKPolyDataFilter(const Self &);
  void                         operator=(const Self &);
  unsigned char *              m_color;
  vtkSmartPointer<vtkPolyData> m_Output;
};

#include "PolylineMeshToVTKPolyDataFilter.txx"
#endif
