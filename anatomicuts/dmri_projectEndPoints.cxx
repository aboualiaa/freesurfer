/* Author: Alexander Zsikla
 * Advisor: Professor Siless
 * Time: Summer 2019
 * Name: dmri_projectEndPoints.cxx
 *
 * Description:
 * Changes the endpoints of a streamline to a different value and save it to a
 * new overlay file
 *
 */

const int ENDPOINT_VALUE = 1;

// Libraries
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

// Input Splicing
#include "GetPot.h"

// TRK Loading
#include "ClusterTools.h"
#include "EuclideanMembershipFunction.h"
#include "LabelPerPointVariableLengthVector.h"
#include "PolylineMeshToVTKPolyDataFilter.h"
#include "TrkVTKPolyDataFilter.txx"
#include "itkArray.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkImage.h"
#include "itkPolylineCell.h"
#include <cmath>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>

// Surface Loading
#include "colortab.h"
#include "fsSurface.h"
#include "fsenv.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkImage.h"
#include "itkSmoothingQuadEdgeMeshFilter.h"
#include "itkTriangleCell.h"
#include "itkVTKPolyDataWriter.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include <map>
#include <set>

#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkDecimatePro.h"
#include "vtkFillHolesFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTriangle.h"
#include "vtkTriangleFilter.h"

#include "macros.h"
#include "mri.h"
#include "mrisurf.h"
#include "vtkCurvatures.h"
#include "vtkDelaunay3D.h"
#include "vtkKdTreePointLocator.h"

// Helper Functions
vtkIdType which_ID(double n1, double n2, vtkIdType ID1, vtkIdType ID2);
vtkSmartPointer<vtkPolyData> FSToVTK(MRIS *surf);

int main(int narg, char *arg[]) {
  GetPot gp(narg, const_cast<char **>(arg));

  // Checking for correct parameters
  if ((gp.size() <= 8) or (gp.search(2, "--help", "-h"))) {
    std::cerr
        << "Usage: " << std::endl
        << arg[0]
        << " -i streamlineFile.trk -sl surfaceFile_lh.orig -sr "
           "surfaceFile_rh.orig"
        << std::endl
        << "                       -ol left_overlayFile -or right_overlayFile"
        << std::endl
        << "		       -ri reference_Image" << std::endl;

    return EXIT_FAILURE;
  }

  // Declaration of Variables for Program to Function
  // TRK file Definition
  enum { Dimension = 3 };
  using PixelType                            = int;
  const unsigned int PointDimension          = 3;
  using PointDataType                        = std::vector<int>;
  const unsigned int MaxTopologicalDimension = 3;
  using CoordinateType                       = double;
  using InterpolationWeightType              = double;
  using MeshTraits =
      itk::DefaultStaticMeshTraits<PointDataType, PointDimension,
                                   MaxTopologicalDimension, CoordinateType,
                                   InterpolationWeightType, PointDataType>;
  using HistogramMeshType = itk::Mesh<PixelType, PointDimension, MeshTraits>;

  using ImageType = itk::Image<float, 3>;

  using ColorMeshType    = itk::Mesh<PixelType, PointDimension>;
  using PointType        = ColorMeshType::PointType;
  using CellType         = ColorMeshType::CellType;
  using PolylineCellType = itk::PolylineCell<CellType>;
  using CellAutoPointer  = ColorMeshType::CellAutoPointer;
  using ClusterToolsType =
      ClusterTools<ColorMeshType, ImageType, HistogramMeshType>;

  // Surface file Definition
  using CoordType = float;
  using SurfType  = fs::Surface<CoordType, Dimension>;

  // Input Parsing
  std::vector<std::string> TRKFile;
  TRKFile.push_back(gp.follow("Could not find TRK file", "-i"));
  const char *surfaceFileL = gp.follow("Could not find Surface File", "-sl");
  const char *surfaceFileR = gp.follow("Could not find Surface File", "-sr");
  const char *overlayFileL = gp.follow("Could not find Overlay File", "-ol");
  const char *overlayFileR = gp.follow("Could not find Overlay File", "-or");
  const char *refImage     = gp.follow("Could not find Reference Image", "-ri");

  // Reading in the Image
  // ITK Version
  using ImageReaderType            = itk::ImageFileReader<ImageType>;
  ImageReaderType::Pointer readerS = ImageReaderType::New();
  readerS->SetFileName(refImage);
  readerS->Update();
  ImageType::Pointer volume = readerS->GetOutput();

  // FS Version
  MRI *image = MRIread(refImage);

  // Outputting the Files to Ensure the correct files were input
  std::cerr << std::endl
            << "TRK File:           " << TRKFile.at(0) << std::endl
            << "Left Surface File:  " << surfaceFileL << std::endl
            << "Left Overlay File:  " << overlayFileL << std::endl
            << "Right Surface File: " << surfaceFileR << std::endl
            << "Right Overlay File: " << overlayFileR << std::endl
            << "Reference Image:    " << refImage << std::endl;

  // Loading the TRK files into a mesh
  std::vector<ColorMeshType::Pointer> *     meshes;
  std::vector<vtkSmartPointer<vtkPolyData>> polydatas;

  ClusterToolsType::Pointer clusterTools = ClusterToolsType::New();
  clusterTools->GetPolyDatas(TRKFile, &polydatas, volume);
  meshes = clusterTools->PolydataToMesh(polydatas);

  // Loading the Surface files and initialization of the KdTree
  // Left Hemisphere
  MRI_SURFACE *surfL;
  surfL = MRISread(surfaceFileL);

  vtkSmartPointer<vtkPolyData> surfVTK_L = FSToVTK(surfL);

  vtkSmartPointer<vtkKdTreePointLocator> surfTreeL =
      vtkSmartPointer<vtkKdTreePointLocator>::New();
  surfTreeL->SetDataSet(surfVTK_L);
  surfTreeL->BuildLocator();

  // Right Hemisphere
  MRI_SURFACE *surfR;
  surfR = MRISread(surfaceFileR);

  vtkSmartPointer<vtkPolyData> surfVTK_R = FSToVTK(surfR);

  vtkSmartPointer<vtkKdTreePointLocator> surfTreeR =
      vtkSmartPointer<vtkKdTreePointLocator>::New();
  surfTreeR->SetDataSet(surfVTK_R);
  surfTreeR->BuildLocator();

  // Variables able to hold a point in two different types
  PointType point;
  point.Fill(0);
  double point_array[3];

  // Initialization of a streamline
  ColorMeshType::Pointer                  input = (*meshes)[0];
  ColorMeshType::CellsContainer::Iterator inputCellIt =
      input->GetCells()->Begin();

  // Cycles through the points of the streamlines
  for (; inputCellIt != input->GetCells()->End(); ++inputCellIt) {
    CellType::PointIdIterator it = inputCellIt.Value()->PointIdsBegin();
    input->GetPoint(*it, &point);

    ImageType::IndexType index;
    volume->TransformPhysicalPointToIndex(point, index);

    MRIvoxelToSurfaceRAS(image, index[0], index[1], index[2], &point_array[0],
                         &point_array[1], &point_array[2]);

    // Finds closest point and sets value equal to ENDPOINT_VALUE
    double    distL, distR;
    vtkIdType Left_ID =
        surfTreeL->FindClosestPointWithinRadius(1000, point_array, distL);
    vtkIdType Right_ID =
        surfTreeR->FindClosestPointWithinRadius(1000, point_array, distR);
    vtkIdType ID = which_ID(distL, distR, Left_ID, Right_ID);

    if (ID == Left_ID)
      surfL->vertices[ID].curv = ENDPOINT_VALUE;
    else
      surfR->vertices[ID].curv = ENDPOINT_VALUE;

    // Finding last point in the stream
    for (; it != inputCellIt.Value()->PointIdsEnd(); it++)
      input->GetPoint(*it, &point);

    volume->TransformPhysicalPointToIndex(point, index);

    MRIvoxelToSurfaceRAS(image, index[0], index[1], index[2], &point_array[0],
                         &point_array[1], &point_array[2]);

    Left_ID = surfTreeL->FindClosestPointWithinRadius(1000, point_array, distL);
    Right_ID =
        surfTreeR->FindClosestPointWithinRadius(1000, point_array, distR);
    ID = which_ID(distL, distR, Left_ID, Right_ID);

    if (ID == Left_ID)
      surfL->vertices[ID].curv = ENDPOINT_VALUE;
    else
      surfR->vertices[ID].curv = ENDPOINT_VALUE;
  }

  MRISwriteCurvature(surfL, overlayFileL);
  MRISwriteCurvature(surfR, overlayFileR);

  return 0;
}

//
// Converts a surface to a VTK
//
vtkSmartPointer<vtkPolyData> FSToVTK(MRIS *surf) {
  vtkSmartPointer<vtkPoints>    points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> triangles =
      vtkSmartPointer<vtkCellArray>::New();

  for (int i = 0; i < surf->nvertices; i++)
    points->InsertNextPoint(surf->vertices[i].x, surf->vertices[i].y,
                            surf->vertices[i].z);

  for (int i = 0; i < surf->nfaces; i++) {
    vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();

    for (int j = 0; j < 3; j++)
      triangle->GetPointIds()->SetId(j, surf->faces[i].v[j]);

    triangles->InsertNextCell(triangle);
  }

  vtkSmartPointer<vtkPolyData> vtkSurface = vtkSmartPointer<vtkPolyData>::New();
  vtkSurface->SetPoints(points);
  vtkSurface->SetPolys(triangles);

  return vtkSurface;
}

/* Function: which_ID
 * Input: the two distances and the two vertice IDs
 * Return: whichever vertice is closer to the point
 * Does: Compares the two distances and returns the vertice of the shorter
 * distance
 */
vtkIdType which_ID(double n1, double n2, vtkIdType ID1, vtkIdType ID2) {
  if (n1 < n2)
    return ID1;

  return ID2;
}
