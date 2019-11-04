#ifndef __fsSurface_h
#define __fsSurface_h

#include <array>

#include "itkMesh.h"
#include "itkTriangleCell.h"
#include "vtkSmartPointer.h"

#include "mrisurf.h"

using namespace itk;
namespace fs {
struct Face {
  std::array<int, 3> indexPoint;
  float area;
};
struct Edge {
  std::array<int, 2> indexPoint;
  float length;
};

template <typename TValueType, unsigned int VDimension = 3>
class Surface : public itk::Mesh<TValueType, VDimension> {
public:
  using ValueType = TValueType;
  using Self = Surface<TValueType, VDimension>;
  using Pointer = SmartPointer<Self>;
  using Superclass = itk::Mesh<TValueType>;

  using PointsContainer = typename Superclass::PointsContainer;
  using PointType = typename Superclass::PointType;
  using PointIdentifier = typename Superclass::PointIdentifier;
  using CellType = typename Superclass::CellType;
  using CellPointer = typename CellType::CellAutoPointer;

  using TriangleType = itk::TriangleCell<CellType>;
  // typedef typename TriangleType::Pointer TrianglePointer;
  itkNewMacro(Self);
  void Load(MRI_SURFACE *surf);
  MRI_SURFACE *GetFSSurface(MRI_SURFACE *surf);
  std::vector<PointType> GetAdjacentPoints(int idPoint) const;
  std::vector<Face> GetFaces() { return this->faces; }

private:
  std::vector<Face> faces;
  std::vector<Edge> edges;
  std::set<std::pair<int, int>> setEdges;
  std::map<int, std::array<int, 2>> edgePerVertex;
  // std::map<int, std::set<int>> facePerVertex;
  void AddFace(int idPoint1, int idPoint2, int idPoint3);
  void AddEdge(int idPoint1, int idPoint2);
};

#include "fsSurface.txx"

} // namespace fs
#endif
