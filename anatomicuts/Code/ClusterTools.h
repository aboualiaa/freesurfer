#ifndef _ClusterTools_h__
#define _ClusterTools_h__

#include "LabelPerPointVariableLengthVector.h"
#include "LabelsEntropyAndIntersectionMembershipFunction.h"
#include "OrientationPlanesFromParcellationFilter.h"
#include "itkMeshToMeshFilter.h"
#include "vtkSplineFilter.h"
#include <vtkCellArray.h>
#include <vtkCellData.h>

#include "itkDefaultStaticMeshTraits.h"

#include "EuclideanMembershipFunction.h"
#include "LabelPerPointVariableLengthVector.h"
#include "colortab.h"
#include "fsenv.h"

#include "TrkVTKPolyDataFilter.txx"

#include "itkPolylineCell.h"
#include "vtkCellData.h"

#include "PolylineMeshToVTKPolyDataFilter.h"
#include "VTKPolyDataToPolylineMeshFilter.h"
#include <vtkFieldData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

using namespace itk;

enum DirectionsType { ALL = 0, DIAGONAL = 1, STRAIGHT = 2 };

template <class TColorMesh, class TImage, class THistogramMesh>
class ClusterTools : public itk::LightObject //<TColorMesh, TColorMesh>
{
public:
  using Self         = ClusterTools<TColorMesh, TImage, THistogramMesh>;
  using Superclass   = itk::MeshToMeshFilter<TColorMesh, TColorMesh>;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(ClusterTools, LightObject); // MeshToMeshFilter);

  using ColorMeshType    = TColorMesh;
  using ColorMeshPointer = typename ColorMeshType::Pointer;
  using ColorPointType   = typename ColorMeshType::PointType;
  using ColorPixelType   = typename ColorMeshType::PixelType;

  using CellType          = typename ColorMeshType::CellType;
  using CellAutoPointer   = typename CellType::CellAutoPointer;
  using VTKConverterType  = PolylineMeshToVTKPolyDataFilter<ColorMeshType>;
  using MeshConverterType = VTKPolyDataToPolylineMeshFilter<ColorMeshType>;

  using HistogramMeshType    = THistogramMesh;
  using HistogramMeshPointer = typename HistogramMeshType::Pointer;
  using HistogramPointType   = typename HistogramMeshType::PointType;
  using HistogramPixelType   = typename HistogramMeshType::PixelType;
  using HistogramDataType    = typename HistogramMeshType::CellPixelType;
  using MeasurementVectorType =
      LabelPerPointVariableLengthVector<ColorPixelType, HistogramMeshType>;
  using MembershipFunctionType =
      LabelsEntropyAndIntersectionMembershipFunction<MeasurementVectorType>;
  // typedef EuclideanMembershipFunction<MeasurementVectorType>
  // MembershipFunctionType;

  using PointsContainerPointer = typename ColorMeshType::PointsContainerPointer;

  using PointsContainer = typename ColorMeshType::PointsContainer;

  using ImageType    = TImage;
  using ImagePointer = typename ImageType::Pointer;
  using IndexType    = typename ImageType::IndexType;

  void GetPolyDatas(std::vector<std::string>                   files,
                    std::vector<vtkSmartPointer<vtkPolyData>> *polydatas,
                    ImagePointer                               image);
  std::vector<ColorMeshPointer> *
  FixSampleClusters(std::vector<vtkSmartPointer<vtkPolyData>> p, int i);
  std::vector<ColorMeshPointer> *
  PolydataToMesh(std::vector<vtkSmartPointer<vtkPolyData>> p);
  std::vector<HistogramMeshPointer> *
       ColorMeshToHistogramMesh(std::vector<ColorMeshPointer> basicMeshes,
                                ImagePointer                  segmentation,
                                bool removeInterHemispheric);
  void SetDirectionalNeighbors(std::vector<HistogramMeshPointer> *meshes,
                               ImagePointer                       segmentation,
                               std::vector<itk::Vector<float>>    direcciones,
                               bool                               symmetry);

  int   GetAverageStreamline(ColorMeshPointer mesh);
  float GetStandardDeviation(HistogramMeshPointer mesh,
                             int                  averageStreamlineIndex);
  float GetDistance(HistogramMeshPointer mesh, int index_i, int index_j);

  //typedef std::vector<int>                  PointDataType;

  void SaveMesh(ColorMeshPointer mesh, ImagePointer image,
                std::string outputFilename, std::string refFiber);

  int                              SymmetricLabelId(int);
  std::vector<itk::Vector<float>> *GetDirections(DirectionsType dir);

protected:
  ClusterTools() {}
  ~ClusterTools() {}

private:
  ClusterTools(const Self &);
  void operator=(const Self &);
};

#include "ClusterTools.txx"
#endif
