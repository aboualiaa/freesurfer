#ifndef __VTKPolyDataToPolylineMeshFilter_h
#define __VTKPolyDataToPolylineMeshFilter_h

#include "itkMesh.h"
#include "itkMeshSource.h"
#include "itkPolylineCell.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

template <class TOutputMesh>
class VTKPolyDataToPolylineMeshFilter : public MeshSource<TOutputMesh> {
public:
  using Self = VTKPolyDataToPolylineMeshFilter<TOutputMesh>;
  using Superclass = MeshSource<TOutputMesh>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(VTKPolyDataToPolylineMeshFilter, MeshSource);

  using OutputMeshType = TOutputMesh;
  using MeshTraits = typename OutputMeshType::MeshTraits;
  using PointType = typename OutputMeshType::PointType;
  using PixelType = typename OutputMeshType::PixelType;

  /** Some convenient typedefs. */
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using CellTraits = typename OutputMeshType::CellTraits;
  using CellIdentifier = typename OutputMeshType::CellIdentifier;
  using CellType = typename OutputMeshType::CellType;
  using CellAutoPointer = typename OutputMeshType::CellAutoPointer;
  using PointIdentifier = typename OutputMeshType::PointIdentifier;
  using PointIdIterator = typename CellTraits::PointIdIterator;

  using PointsContainerPointer =
      typename OutputMeshType::PointsContainerPointer;

  using PointsContainer = typename OutputMeshType::PointsContainer;

  using PolylineCellType = itk::PolylineCell<CellType>;
  using SelfAutoPointer = typename PolylineCellType::SelfAutoPointer;

  void SetVTKPolyData(vtkSmartPointer<vtkPolyData> polydata) {
    m_VTKPolyData = polydata;
  }
  vtkSmartPointer<vtkPolyData> GetVTKPolyData() const { return m_VTKPolyData; }
  void GenerateData2();

protected:
  VTKPolyDataToPolylineMeshFilter();
  ~VTKPolyDataToPolylineMeshFilter() {}
  void PrintSelf(std::ostream &os, Indent indent) const;

  /** Reads the file */
  void GenerateData();

private:
  VTKPolyDataToPolylineMeshFilter(const Self &);
  void operator=(const Self &);

  vtkSmartPointer<vtkPolyData> m_VTKPolyData;
};

#include "VTKPolyDataToPolylineMeshFilter.txx"
#endif
