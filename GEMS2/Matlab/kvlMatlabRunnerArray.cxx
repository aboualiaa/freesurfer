#include "kvlChangeK.h"
#include "kvlClear.h"
#include "kvlCreateImage.h"
#include "kvlCreateMeshCollection.h"
#include "kvlCreateRGBImage.h"
#include "kvlCreateTransform.h"
#include "kvlDrawJacobianDeterminant.h"
#include "kvlEvaluateMeshPosition.h"
#include "kvlEvaluateMeshPositionWithEntropy.h"
#include "kvlGetAlphasInMeshNodes.h"
#include "kvlGetAverageAtlasMeshPositionCostAndGradientCalculator.h"
#include "kvlGetCostAndGradientCalculator.h"
#include "kvlGetCroppedRegion.h"
#include "kvlGetImageBuffer.h"
#include "kvlGetMesh.h"
#include "kvlGetMeshNodePositions.h"
#include "kvlGetOptimizer.h"
#include "kvlGetTransformMatrix.h"
#include "kvlMatlabRunnerArray.h"
#include "kvlRasterizeAtlasMesh.h"
#include "kvlReadCroppedImage.h"
#include "kvlReadImage.h"
#include "kvlReadMeshCollection.h"
#include "kvlScaleMesh.h"
#include "kvlScaleMeshCollection.h"
#include "kvlSetAlphasInMeshNodes.h"
#include "kvlSetImageBuffer.h"
#include "kvlSetKOfMeshCollection.h"
#include "kvlSetMaximumNumberOfThreads.h"
#include "kvlSetMeshCollectionPositions.h"
#include "kvlSetMeshNodePositions.h"
#include "kvlSmoothImageBuffer.h"
#include "kvlSmoothMesh.h"
#include "kvlSmoothMeshCollection.h"
#include "kvlStepOptimizer.h"
#include "kvlTransformMeshCollection.h"
#include "kvlWriteImage.h"
#include "kvlWriteMeshCollection.h"
#include "kvlWriteRGBImage.h"

namespace kvl {

MatlabRunnerArray::Pointer MatlabRunnerArray::m_Instance = nullptr;

//
//
//
MatlabRunnerArray ::MatlabRunnerArray() {
  m_Array.emplace_back(ReadCroppedImage::New().GetPointer());
  m_Array.emplace_back(GetImageBuffer::New().GetPointer());
  m_Array.emplace_back(SetImageBuffer::New().GetPointer());
  m_Array.emplace_back(ReadMeshCollection::New().GetPointer());
  m_Array.emplace_back(GetMesh::New().GetPointer());
  m_Array.emplace_back(RasterizeAtlasMesh::New().GetPointer());
  m_Array.emplace_back(SmoothImageBuffer::New().GetPointer());
  m_Array.emplace_back(GetAlphasInMeshNodes::New().GetPointer());
  m_Array.emplace_back(SetAlphasInMeshNodes::New().GetPointer());
  m_Array.emplace_back(SmoothMeshCollection::New().GetPointer());
  m_Array.emplace_back(ScaleMeshCollection::New().GetPointer());
  m_Array.emplace_back(WriteImage::New().GetPointer());
  m_Array.emplace_back(CreateImage::New().GetPointer());
  m_Array.emplace_back(Clear::New().GetPointer());
  m_Array.emplace_back(SetMaximumNumberOfThreads::New().GetPointer());
  m_Array.emplace_back(ScaleMesh::New().GetPointer());
  m_Array.emplace_back(SmoothMesh::New().GetPointer());
  m_Array.emplace_back(ReadImage::New().GetPointer());
  m_Array.emplace_back(GetTransformMatrix::New().GetPointer());
  m_Array.emplace_back(GetMeshNodePositions::New().GetPointer());
  m_Array.emplace_back(SetMeshCollectionPositions::New().GetPointer());
  m_Array.push_back(WriteMeshCollection::New().GetPointer());
  m_Array.emplace_back(TransformMeshCollection::New().GetPointer());
  m_Array.emplace_back(SetKOfMeshCollection::New().GetPointer());
  m_Array.emplace_back(CreateTransform::New().GetPointer());
  m_Array.emplace_back(EvaluateMeshPositionWithEntropy::New().GetPointer());
  m_Array.emplace_back(SetMeshNodePositions::New().GetPointer());
  m_Array.emplace_back(ChangeK::New().GetPointer());
  m_Array.emplace_back(CreateRGBImage::New().GetPointer());
  m_Array.emplace_back(WriteRGBImage::New().GetPointer());
  m_Array.emplace_back(GetCroppedRegion::New().GetPointer());
  m_Array.emplace_back(GetCostAndGradientCalculator::New().GetPointer());
  m_Array.emplace_back(GetAverageAtlasMeshPositionCostAndGradientCalculator::New().GetPointer());
  m_Array.emplace_back(EvaluateMeshPosition::New().GetPointer());
  m_Array.emplace_back(GetOptimizer::New().GetPointer());
  m_Array.emplace_back(StepOptimizer::New().GetPointer());
  m_Array.emplace_back(CreateMeshCollection::New().GetPointer());
  m_Array.emplace_back(DrawJacobianDeterminant::New().GetPointer());
}

//
// Return the single instance of the MatlabRunnerArray
//
MatlabRunnerArray::Pointer MatlabRunnerArray ::New() {

  if (MatlabRunnerArray::m_Instance == nullptr) {
    MatlabRunnerArray::m_Instance = new MatlabRunnerArray;

    // Remove extra reference from construction.
    MatlabRunnerArray::m_Instance->UnRegister();
  }

  return MatlabRunnerArray::m_Instance;
}

//
//
//
bool MatlabRunnerArray ::Run(const std::string &runnerName, int nlhs,
                             mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  bool executedCorrectRunner = false;

  for (auto it = m_Array.begin();
       it != m_Array.end(); ++it) {

    if (runnerName.compare(it->GetPointer()->GetNameOfClass()) == 0) {
      // std::cout << "Found a runner with name " << runnerName
      //          << " : running it!" << std::endl;
      it->GetPointer()->Run(nlhs, plhs, nrhs, prhs);
      executedCorrectRunner = true;
      break;
    }
  }

  return executedCorrectRunner;
}

} // end namespace kvl
