#ifndef _HierarchicalClusteringPruner_h_
#define _HierarchicalClusteringPruner_h_

#include "itkMeshToMeshFilter.h"
#include "AppendBundleFilter.h"
enum FiberFormat { VTK = 0, TRK = 1 };

template <class TOutputMesh, class TImageType>
class HierarchicalClusteringPruner : public MeshSource<TOutputMesh> {
public:
  using Self = HierarchicalClusteringPruner<TOutputMesh, TImageType>;
  using Superclass = MeshSource<TOutputMesh>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(HierarchicalClusteringPruner, MeshSource);

  using ImageType = TImageType;
  using OutputMeshType = TOutputMesh;
  using OutputMeshTraits = typename OutputMeshType::MeshTraits;
  using OutputPointType = typename OutputMeshType::PointType;
  using OutputPixelType = typename OutputMeshType::PixelType;

  /** Some convenient typedefs. */
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using OutputCellTraits = typename OutputMeshType::CellTraits;
  using OutputCellIdentifier = typename OutputMeshType::CellIdentifier;
  using OutputCellType = typename OutputMeshType::CellType;
  using OutputCellAutoPointer = typename OutputMeshType::CellAutoPointer;
  using OutputPointIdentifier = typename OutputMeshType::PointIdentifier;
  using OutputPointIdIterator = typename OutputCellTraits::PointIdIterator;

  using OutputPointsContainerPointer =
      typename OutputMeshType::PointsContainerPointer;

  using OutputPointsContainer = typename OutputMeshType::PointsContainer;

  using OutputCellsContainer = typename OutputMeshType::CellsContainer;

  using OutputCellsContainerPointer =
      typename OutputMeshType::CellsContainerPointer;

  using PolylineCellType = int;

  int GetNumberOfClusters() { return this->m_numberOfClusters; }

  void SetNumberOfClusters(int n) { this->m_numberOfClusters = n; }
  void SetHierarchyFilename(std::string filename) {
    this->m_hierarchyFilename = filename;
  }
  std::string GetHierarchyFilename() { return this->m_hierarchyFilename; }
  void SetClustersPath(std::string path) { this->m_clustersPath = path; }
  std::string GetClustersPath() { return this->m_clustersPath; }
  void SetExtension(FiberFormat f) { this->m_fiberFormat = f; }

  FiberFormat GetExtension() { return this->m_fiberFormat; }
  std::vector<vtkSmartPointer<vtkPolyData>> GetOutputBundles() {
    return this->m_outputBundles;
  }
  std::vector<long long> GetClustersIds() { return this->m_clustersIds; }
  void SetReferenceImage(typename ImageType::Pointer image) {
    this->m_referenceImage = image;
  }

protected:
  HierarchicalClusteringPruner();
  ~HierarchicalClusteringPruner(){};

  virtual void GenerateData();

private:
  int m_numberOfClusters;
  std::string m_hierarchyFilename;
  std::string m_clustersPath;
  FiberFormat m_fiberFormat;
  std::vector<vtkSmartPointer<vtkPolyData>> m_outputBundles;
  std::vector<long long> m_clustersIds;
  typename ImageType::Pointer m_referenceImage;
};

#include "HierarchicalClusteringPruner.txx"

#endif
