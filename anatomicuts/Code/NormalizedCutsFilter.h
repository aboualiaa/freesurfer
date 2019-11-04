#ifndef __NormalizedCutsFilter_h
#define __NormalizedCutsFilter_h

#include "itkKdTree.h"
#include "itkKdTreeGenerator.h"
#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkListSample.h"
#include "itkArray.h"
#include "itkVector.h"
//#include "itkImageKmeansModelEstimator.h"
#include "itkImageRegionIteratorWithIndex.h"
//#include "itkKMeansClassifierFilter.h"
#include "itkVariableLengthVector.h"
#include "itkPolylineCell.h"
#include "itkWeightedCentroidKdTreeGenerator.h"
#include "itkMeshToMeshFilter.h"
#include "ThreadedMembershipFunction.h"
#if ITK_VERSION_MAJOR < 4
#include "itkMaximumDecisionRule2.h"
#else
#include "itkMaximumDecisionRule.h"
#endif
using namespace itk;
using namespace Statistics;
template <class TMesh, class TMembershipFunctionType>
class NormalizedCutsFilter : public ProcessObject
//  public MeshToMeshFilter<TInputMesh, TOutputMesh>
{
public:
  using Self = NormalizedCutsFilter<TMesh, TMembershipFunctionType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(NormalizedCutsFilter, ProcessObject);

  using MeshType = TMesh;
  using MeshTraits = typename MeshType::MeshTraits;
  using PointType = typename MeshType::PointType;
  using PixelType = typename MeshType::PixelType;
  using CellType = typename MeshType::CellType;
  using MeshPointerType = typename MeshType::Pointer;

  using ListOfOutputMeshTypePointer = typename std::vector<MeshPointerType>;

  using MeshCellTraits = typename MeshType::CellTraits;
  using MeshCellIdentifier = typename MeshType::CellIdentifier;
  using MeshCellType = typename MeshType::CellType;

  using MeshCellAutoPointer = typename MeshType::CellAutoPointer;
  using MeshPointIdentifier = typename MeshType::PointIdentifier;
  using MeshPointIdIterator = typename MeshCellTraits::PointIdIterator;

  using PointsContainerPointer = typename MeshType::PointsContainerPointer;

  using PointsContainer = typename MeshType::PointsContainer;

  using CellsContainer = typename MeshType::CellsContainer;

  using CellsContainerPointer = typename MeshType::CellsContainerPointer;
  using PolylineCellType = PolylineCell<MeshCellType>;

  using MembershipFunctionType = TMembershipFunctionType;
  using MembershipFunctionPointer = typename MembershipFunctionType::Pointer;
  using MeasurementVectorType =
      typename MembershipFunctionType::MeasurementVectorType;

  using ThreadedMembershipFunctionType =
      ThreadedMembershipFunction<MembershipFunctionType>;

  using SampleType = ListSample<MeasurementVectorType>;
  using TreeGeneratorType = WeightedCentroidKdTreeGenerator<SampleType>;
  using TreeType = typename TreeGeneratorType::KdTreeType;
  using EstimatorType = KdTreeBasedKmeansEstimator<TreeType>;

#if ITK_VERSION_MAJOR < 4
  typedef MaximumDecisionRule2 DecisionRuleType;
#else
  using DecisionRuleType = MaximumDecisionRule;
#endif

  //	typedef KMeansClassifierFilter<SampleType,MembershipFunctionType>
  //ClassifierType;
  //		typedef typename ClassifierType::ClassLabelVectorType
  //ClassLabelVectorType; 	typedef typename
  //ClassifierType::MembershipFunctionVectorType MembershipFunctionVectorType;
  using MembershipFunctionTypePointer =
      typename MembershipFunctionType::Pointer;
  using MembershipFunctionVectorType =
      typename std::vector<MembershipFunctionTypePointer>;

  ListOfOutputMeshTypePointer GetOutput() { return this->m_Output; }

  void SetNumberOfClusters(int n) { this->numberOfClusters = n; }

  int GetNumberOfClusters() { return this->numberOfClusters; }
  void SetNumberOfFibersForEigenDecomposition(int e) {
    this->m_numberOfFibersForEigenDecomposition = e;
  }
  int GetNumberOfFibersForEigenDecomposition() {
    return m_numberOfFibersForEigenDecomposition;
  }

  std::vector<std::string> GetLabels() { return this->labels; }
  void SetLabels(std::vector<std::string> labels) { this->labels = labels; }

  MeshPointerType GetInput() { return this->input; }
  void SetInput(MeshPointerType input) { this->input = input; }
  // void Update();
  virtual void Update();

  itkGetMacro(SigmaCurrents, int);
  //  itkSetMacro( SigmaCurrents, int );
  void SetSigmaCurrents(int s) { this->m_SigmaCurrents = s; }

  itkSetMacro(NumberOfIterations, unsigned int);
  itkGetMacro(NumberOfIterations, unsigned int);

  MembershipFunctionVectorType *GetMembershipFunctionVector() {
    return this->m_membershipFunctions;
  }
  void SetMembershipFunctionVector(MembershipFunctionVectorType *functions) {
    this->m_membershipFunctions = functions;
  }

  std::vector<std::pair<std::string, std::string>> GetClusterIdHierarchy() {
    return this->m_clusterIdHierarchy;
  }

protected:
  std::vector<std::pair<int, int>>
  SelectCentroids(typename SampleType::Pointer samples,
                  const typename MembershipFunctionType::Pointer);
  std::vector<std::pair<int, int>>
  SelectCentroidsParallel(typename SampleType::Pointer samples,
                          const typename MembershipFunctionType::Pointer);
  MeshPointerType input;
  std::vector<std::string> labels;
  ListOfOutputMeshTypePointer m_Output;
  int numberOfClusters;
  NormalizedCutsFilter() {}
  ~NormalizedCutsFilter() {}

  //    virtual void GenerateData (void);
private:
  unsigned int m_NumberOfIterations;
  NormalizedCutsFilter(const Self &);
  std::vector<std::pair<std::string, std::string>> m_clusterIdHierarchy;
  void operator=(const Self &);
  int m_SigmaCurrents;
  int m_numberOfFibersForEigenDecomposition;
  //		void SaveClustersInMeshes(MembershipFunctionVectorType mfv);
  MembershipFunctionVectorType *m_membershipFunctions;
};
#include "NormalizedCutsFilter.txx"
#endif
