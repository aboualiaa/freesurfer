#ifndef GEMS_PYKVLMESH_H_H
#define GEMS_PYKVLMESH_H_H

#include "itkObject.h"
#include "kvlAtlasMeshCollection.h"
#include "pyKvlTransform.h"
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

namespace py = pybind11;

using MeshCollectionPointer = kvl::AtlasMeshCollection::Pointer;
using MeshPointer = kvl::AtlasMesh::ConstPointer;
using PointSetPointer = kvl::AtlasMesh::PointsContainer *;
using PointSetConstPointer = kvl::AtlasMesh::PointsContainer::ConstPointer;
using PointDataPointer = kvl::AtlasMesh::PointDataContainer *;
using PointDataConstPointer = kvl::AtlasMesh::PointDataContainer::ConstPointer;
using SHAPE_3D = std::vector<unsigned int>;
using SCALE_3D = std::vector<double>;

class KvlMesh {

public:
    // Python accessible
    KvlMesh();
    int PointCount() const;
    py::array_t<double> GetPointSet() const;
    void SetPointSet(const py::array_t<double> &source);
    py::array_t<double> GetAlphas() const;
    void SetAlphas(const py::array_t<double> &source);
    void Scale(const SCALE_3D &scaling);
    py::array_t<uint16_t> RasterizeMesh(std::vector<size_t> size, int classNumber=-1);
    py::array RasterizeValues(std::vector<size_t> size, py::array_t<double, py::array::c_style | py::array::forcecast> values);
    py::array_t<double> FitAlphas( const py::array_t< uint16_t, 
                                                      py::array::f_style | py::array::forcecast >& 
                                   probabilityImageBuffer ) const;


  // C++ Only
  KvlMesh(MeshPointer &aMesh);
  [[nodiscard]] const char *GetNameOfClass() const { return "KvlMesh"; }
  MeshPointer mesh;
};

class KvlMeshCollection {
  MeshCollectionPointer meshCollection;

public:
  // Python accessible
  void Read(const std::string &meshCollectionFileName);
  void Write(const std::string &meshCollectionFileName);
  [[nodiscard]] double GetK() const;
  void SetK(double k);
  [[nodiscard]] unsigned int MeshCount() const;
  KvlMesh *GetMesh(int meshNumber);
  KvlMesh *GetReferenceMesh();
  [[nodiscard]] py::array_t<double> GetReferencePosition() const;
  void SetReferencePosition(const py::array_t<double> &source);
  void SetPositions(const py::array_t<double> &reference,
                    const std::vector<py::array_t<double>> &positions);
  void Construct(const SHAPE_3D &meshSize, const SHAPE_3D &domainSize,
                 double initialStiffness, unsigned int numberOfClasses,
                 unsigned int numberOfMeshes);
  void Transform(const KvlTransform &transform);

  // C++ use only
  KvlMeshCollection();
  [[nodiscard]] const char *GetNameOfClass() const { return "KvlMeshCollection"; }
  MeshCollectionPointer GetMeshCollection() { return meshCollection; }
};

py::array_t<double> PointSetToNumpy(PointSetConstPointer points);
void CopyNumpyToPointSet(PointSetPointer points,
                         const py::array_t<double> &source);
py::array_t<double> AlphasToNumpy(PointDataConstPointer alphas);
void CopyNumpyToPointDataSet(PointDataPointer alphas,
                             const py::array_t<double> &source);
void CreatePointSetFromNumpy(PointSetPointer targetPoints,
                             const py::array_t<double> &source);

#endif // GEMS_PYKVLMESH_H_H
