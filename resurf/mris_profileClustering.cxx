#include "itkKdTreeBasedKmeansEstimator.h"
#include "itkListSample.h"
#include "itkMinimumDecisionRule.h"
#include "itkSampleClassifierFilter.h"
#include "itkWeightedCentroidKdTreeGenerator.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "GetPot.h"
#include "fsSurface.h"
#include <vnl/vnl_cross.h>

int main(int narg, char *arg[]) {
  constexpr unsigned int Dimension = 3;
  using CoordType                  = float;
  using ImageType                  = itk::Image<CoordType, Dimension>;
  using OutputImageType            = itk::Image<CoordType, 4>;
  using ReaderType                 = itk::ImageFileReader<ImageType>;
  using MeasurementVectorType      = itk::VariableLengthVector<CoordType>;
  using SampleType  = itk::Statistics::ListSample<MeasurementVectorType>;
  using SurfaceType = fs::Surface<CoordType, Dimension>;
  using TreeGeneratorType =
      itk::Statistics::WeightedCentroidKdTreeGenerator<SampleType>;
  using TreeType      = TreeGeneratorType::KdTreeType;
  using EstimatorType = itk::Statistics::KdTreeBasedKmeansEstimator<TreeType>;
  using MembershipFunctionType =
      itk::Statistics::DistanceToCentroidMembershipFunction<
          MeasurementVectorType>;
  using MembershipFunctionPointer = MembershipFunctionType::Pointer;
  using DecisionRuleType          = itk::Statistics::MinimumDecisionRule;
  using ClassifierType = itk::Statistics::SampleClassifierFilter<SampleType>;
  using ClassLabelVectorObjectType = ClassifierType::ClassLabelVectorObjectType;
  using ClassLabelVectorType       = ClassifierType::ClassLabelVectorType;
  using MembershipFunctionVectorObjectType =
      ClassifierType::MembershipFunctionVectorObjectType;
  using MembershipFunctionVectorType =
      ClassifierType::MembershipFunctionVectorType;

  GetPot cl(narg, const_cast<char **>(arg));
  if (cl.size() == 1 || cl.search(2, "--help", "-h")) {
    std::cout << "Usage: " << std::endl;
    std::cout << arg[0]
              << " -s surface -i N image1 .. imageN  -c numClusters -d deep -o "
                 "outputImage -a annot -b surf "
              << std::endl;
    return -1;
  }

  const char *                    surfFilename          = cl.follow("", "-s");
  const char *                    outputImageFilename   = cl.follow("", "-o");
  const char *                    outputSurfaceFilename = cl.follow("", "-b");
  const char *                    annotationFilename    = cl.follow("", "-a");
  unsigned int                    numClusters           = cl.follow(10, "-c");
  unsigned int                    deep                  = cl.follow(10, "-d");
  unsigned int                    imageNumber           = cl.follow(1, "-i");
  std::vector<const char *>       imageFilenames;
  std::vector<ImageType::Pointer> images;
  for (unsigned int i = 0; i < imageNumber; i++) {
    imageFilenames.push_back(cl.next(""));
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(imageFilenames[i]);
    reader->Update();
    images.push_back(reader->GetOutput());
    std::cout << i << std::endl;
  }

  ImageType::Pointer image = images[0];

  MRI_SURFACE *surf;
  surf = MRISread(surfFilename);

  SurfaceType::Pointer surface = SurfaceType::New();
  surface->Load(&*surf);

  MRI *imageFS = MRIread(imageFilenames[0]);

  SampleType::Pointer sample       = SampleType::New();
  unsigned int        vectorLenght = (deep * 2 + 1) * imageNumber;
  std::cout << vectorLenght << std::endl;
  sample->SetMeasurementVectorSize(vectorLenght);

  std::vector<SurfaceType::PointType> points;
  ImageType::RegionType    region3D    = image->GetLargestPossibleRegion();
  ImageType::DirectionType direction3D = image->GetDirection();
  ImageType::SpacingType   spacing3D   = image->GetSpacing();

  OutputImageType::Pointer       output = OutputImageType::New();
  OutputImageType::RegionType    region4D;
  OutputImageType::DirectionType direction;
  OutputImageType::SpacingType   spacing;
  OutputImageType::PointType     origin;

  OutputImageType::SizeType  size;
  OutputImageType::IndexType start;
  direction.SetIdentity();
  for (int i = 0; i < 3; i++) {
    size[i]    = region3D.GetSize()[i];
    start[i]   = region3D.GetIndex()[i];
    spacing[i] = spacing3D[i];
    origin[i]  = image->GetOrigin()[i];
    for (int j = 0; j < 3; j++)
      direction[j][i] = direction3D[j][i];
  }
  start[3] = 0;
  size[3]  = vectorLenght;
  // spacing[3]=vol/3;
  origin[3] = 0;
  region4D.SetSize(size);
  region4D.SetIndex(start);
  output->SetRegions(region4D);
  output->SetDirection(direction);
  output->SetSpacing(spacing);
  output->SetOrigin(origin);
  output->Allocate();
  std::cout << image->GetDirection() << std::endl;
  std::cout << output->GetDirection() << std::endl;
  std::cout << image->GetOrigin() << " " << output->GetOrigin() << std::endl;

  for (SurfaceType::CellsContainerConstIterator itCells =
           surface->GetCells()->Begin();
       itCells != surface->GetCells()->End(); itCells++) {
    SurfaceType::CellType::PointIdIterator pointsIt =
        itCells.Value()->PointIdsBegin();
    SurfaceType::PointType p1    = surface->GetPoint(*pointsIt);
    SurfaceType::PointType p2    = surface->GetPoint(*(++pointsIt));
    SurfaceType::PointType p3    = surface->GetPoint(*(++pointsIt));
    SurfaceType::PointType edge1 = p1 - p2;
    SurfaceType::PointType edge2 = p1 - p3;
    // std::cout << p1 << std::endl;
    vnl_vector<CoordType> normal =
        vnl_cross_3d(edge1.GetVnlVector(), edge2.GetVnlVector());
    normal = normal.normalize();
    SurfaceType::PointType mean;
    mean.Fill(0.0);
    for (int i = 0; i < 3; i++)
      mean[i] = (p1[i] + p2[i] + p3[i]) / 3.0;

    ImageType::IndexType index;

    MeasurementVectorType mv;
    mv.AllocateElements(vectorLenght);
    mv.SetSize(vectorLenght);
    for (unsigned int j = 0; j < images.size(); j++) {
      for (unsigned int d = -deep; d <= deep; d++) {
        SurfaceType::PointType pt;
        for (int i = 0; i < 3; i++)
          pt[i] = mean[i] + d * normal[i];

        double x, y, z;
        MRISsurfaceRASToVoxel(surf, imageFS, pt[0], pt[1], pt[2], &x, &y, &z);
        index[0]                          = x;
        index[1]                          = y;
        index[2]                          = z;
        mv[j * (deep * 2 + 1) + d + deep] = images[j]->GetPixel(index);
      }
    }
    sample->PushBack(mv);
    points.push_back(mean);
  }

  TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();
  treeGenerator->SetSample(sample);
  treeGenerator->SetBucketSize(160);
  treeGenerator->Update();

  EstimatorType::Pointer        estimator = EstimatorType::New();
  EstimatorType::ParametersType initialMeans(vectorLenght * numClusters);
  for (unsigned int c = 0; c < numClusters; c++) {
    for (unsigned int i = 0; i <= vectorLenght; i++)
      initialMeans[c * vectorLenght + i] =
          sample->GetMeasurementVector(c * (sample->Size() / numClusters))[i];
  }
  estimator->SetParameters(initialMeans);
  estimator->SetKdTree(treeGenerator->GetOutput());
  estimator->SetMaximumIteration(200);
  estimator->SetCentroidPositionChangesThreshold(0.0);
  estimator->StartOptimization();

  EstimatorType::ParametersType estimatedMeans = estimator->GetParameters();

  /*for ( unsigned int i = 0 ; i < numClusters ; ++i )
  {
          std::cout << "cluster[" << i << "] " << std::endl;
          std::cout << "    estimated mean : " << estimatedMeans[i] <<
  std::endl;
  }
  */
  DecisionRuleType::Pointer decisionRule = DecisionRuleType::New();
  ClassifierType::Pointer   classifier   = ClassifierType::New();
  classifier->SetDecisionRule(decisionRule);
  classifier->SetInput(sample);
  classifier->SetNumberOfClasses(numClusters);

  ClassLabelVectorObjectType::Pointer classLabelsObject =
      ClassLabelVectorObjectType::New();
  classifier->SetClassLabels(classLabelsObject);

  ClassLabelVectorType &classLabelsVector = classLabelsObject->Get();
  for (unsigned int i = 0; i < numClusters; i++)
    classLabelsVector.push_back(i);

  MembershipFunctionVectorObjectType::Pointer membershipFunctionsObject =
      MembershipFunctionVectorObjectType::New();
  classifier->SetMembershipFunctions(membershipFunctionsObject);

  MembershipFunctionVectorType &membershipFunctionsVector =
      membershipFunctionsObject->Get();

  MembershipFunctionType::CentroidType centroid(
      sample->GetMeasurementVectorSize());
  int                                    index = 0;
  std::vector<MembershipFunctionPointer> functions;
  std::vector<MeasurementVectorType>     clusterMeans;
  for (unsigned int i = 0; i < numClusters; i++) {
    MembershipFunctionPointer membershipFunction =
        MembershipFunctionType::New();
    for (unsigned int j = 0; j < sample->GetMeasurementVectorSize(); j++) {
      centroid[j] = estimatedMeans[index++];
    }
    membershipFunction->SetCentroid(centroid);
    functions.push_back(membershipFunction);
    membershipFunctionsVector.push_back(membershipFunction.GetPointer());
    MeasurementVectorType mean;
    mean.SetSize(vectorLenght);
    mean.Fill(0);
    clusterMeans.push_back(mean);
  }
  classifier->Update();

  const ClassifierType::MembershipSampleType *membershipSample =
      classifier->GetOutput();
  ClassifierType::MembershipSampleType::ConstIterator iter =
      membershipSample->Begin();
  std::vector<int> norm(numClusters, 0);
  while (iter != membershipSample->End()) {
    clusterMeans[iter.GetClassLabel()] += iter.GetMeasurementVector();
    norm[iter.GetClassLabel()]++;
    ++iter;
  }
  for (unsigned int i = 0; i < numClusters; i++) {
    std::cout << clusterMeans[i] / norm[i] << " " << norm[i] << std::endl;
    if (norm[i] > 0)
      clusterMeans[i] /= norm[i];
  }

  iter  = membershipSample->Begin();
  int i = 0;

  COLOR_TABLE *ct;
  int          annot;

  ct       = CTABalloc(numClusters + 1);
  surf->ct = ct;

  while (iter != membershipSample->End()) {

    // MeasurementVectorType meanIntensityDecay =
    // clusterMeans[iter.GetClassLabel()]; ///norm[iter.GetClassLabel()];
    OutputImageType::IndexType index;
    // output->TransformPhysicalPointToIndex(points[i], index);

    double x, y, z;
    MRISsurfaceRASToVoxel(surf, imageFS, points[i][0], points[i][1],
                          points[i][2], &x, &y, &z);
    index[0] = x;
    index[1] = y;
    index[2] = z;
    for (unsigned int j = 0; j < vectorLenght; j++) {
      index[3] = j;
      output->SetPixel(index, iter.GetMeasurementVector()[index[3]]);
    }
    CTABannotationAtIndex(surf->ct, iter.GetClassLabel(), &annot);
    surf->vertices[surface->GetFaces()[i].indexPoint[0]].annotation = annot;
    surf->vertices[surface->GetFaces()[i].indexPoint[1]].annotation = annot;
    surf->vertices[surface->GetFaces()[i].indexPoint[2]].annotation = annot;
    ++iter;
    i++;
  }
  std::cout << " i " << i << " " << surf->nvertices << "  " << surf->nfaces
            << std::endl;

  using WriterType           = itk::ImageFileWriter<OutputImageType>;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputImageFilename);
  writer->SetInput(output);
  writer->Update();

  MRISwriteAnnotation(surf, annotationFilename);
  MRISwrite(surf, outputSurfaceFilename);
  MRISfree(&surf);
}
