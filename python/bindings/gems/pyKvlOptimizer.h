#ifndef GEMS_PYKVLOPTIMIZER_H
#define GEMS_PYKVLOPTIMIZER_H

#include "kvlAtlasMeshCollection.h"
#include "pyKvlImage.h"

#include "kvlAtlasMeshDeformationConjugateGradientOptimizer.h"
#include "kvlAtlasMeshDeformationFixedStepGradientDescentOptimizer.h"
#include "kvlAtlasMeshDeformationGradientDescentOptimizer.h"
#include "kvlAtlasMeshDeformationLBFGSOptimizer.h"

#include "pyKvlCalculator.h"
#include "pyKvlMesh.h"
#include "pyKvlNumpy.h"
#include "pybind11/pybind11.h"

namespace py = pybind11;

using ImageType    = itk::Image<float, 3>;
using ImagePointer = ImageType::Pointer;

class KvlOptimizer {
  kvl::AtlasMeshDeformationOptimizer::Pointer optimizer;

public:
  KvlOptimizer(std::string typeName, const KvlMesh &mesh,
               const KvlCostAndGradientCalculator &calculator,
               std::map<std::string, double>       arguments) {
    switch (typeName[0]) {
      //            case 'F':
      //            {
      //                std::cout << "FixedStepGradientDescent" << std::endl;
      //                AtlasMeshDeformationFixedStepGradientDescentOptimizer::Pointer
      //                myOptimizer
      //                        =
      //                        AtlasMeshDeformationFixedStepGradientDescentOptimizer::New();
      //                myOptimizer->SetStepSize( 1.0 );
      //                optimizer = myOptimizer;
      //                break;
      //            }
      //            case 'G':
      //            {
      //                std::cout << "GradientDescent" << std::endl;
      //                AtlasMeshDeformationGradientDescentOptimizer::Pointer
      //                myOptimizer
      //                        =
      //                        AtlasMeshDeformationGradientDescentOptimizer::New();
      //                optimizer = myOptimizer;
      //                break;
      //            }
      //            case 'C':
      //            {
      //                std::cout << "ConjugateGradient" << std::endl;
      //                AtlasMeshDeformationConjugateGradientOptimizer::Pointer
      //                myOptimizer
      //                        =
      //                        AtlasMeshDeformationConjugateGradientOptimizer::New();
      //                optimizer = myOptimizer;
      //                break;
      //            }
    case 'L': {
      std::cout << "L-BFGS" << std::endl;
      kvl::AtlasMeshDeformationLBFGSOptimizer::Pointer myOptimizer =
          kvl::AtlasMeshDeformationLBFGSOptimizer::New();
      optimizer = myOptimizer;
      break;
    }
    default: {
      throw std::invalid_argument("Optimizer type not supported.");
    }
    }

    // Parse additional options. Format is always [ 'someString', double ]
    for (auto const &arg : arguments) {

      const std::string optionName  = arg.first;
      const double      optionValue = arg.second;

      switch (optionName[0]) {
      case 'V': {
        std::cout << "Verbose: " << optionValue << std::endl;
        if (optionValue) {
          optimizer->SetVerbose(true);
        }

        break;
      }
      case 'M': {
        if (optionName.substr(0, 8) == "MaximalD") {
          std::cout << "MaximalDeformationStopCriterion: " << optionValue
                    << std::endl;
          optimizer->SetMaximalDeformationStopCriterion(optionValue);
        } else if (optionName.substr(0, 8) == "MaximumN") {
          const int maximumNumberOfIterations = static_cast<int>(optionValue);
          std::cout << "MaximumNumberOfIterations: "
                    << maximumNumberOfIterations << std::endl;
          optimizer->SetMaximumNumberOfIterations(maximumNumberOfIterations);
        } else {
          std::ostringstream errorStream;
          errorStream << "optionName: " << optionName << " not understood";
          throw std::invalid_argument(errorStream.str().c_str());
          break;
        } // End figuring out which "M" you mean

        break;
      }
      case 'L': {
        std::cout << "LineSearchMaximalDeformationIntervalStopCriterion: "
                  << optionValue << std::endl;
        optimizer->SetLineSearchMaximalDeformationIntervalStopCriterion(
            optionValue);
        break;
      }
      case 'B': {
        kvl::AtlasMeshDeformationLBFGSOptimizer::Pointer myOptimizer =
            dynamic_cast<kvl::AtlasMeshDeformationLBFGSOptimizer *>(
                optimizer.GetPointer());
        if (myOptimizer) {
          const int maximumMemoryLength = static_cast<int>(optionValue);
          std::cout << "BFGS-MaximumMemoryLength: " << maximumMemoryLength
                    << std::endl;
          myOptimizer->SetMaximumMemoryLength(maximumMemoryLength);
        } else {
          std::cout << "BFGS-MaximumMemoryLength only applies to BFGS optimizer"
                    << std::endl;
        }

        break;
      }
      default: {
        std::ostringstream errorStream;
        errorStream << "optionName: " << optionName << " not understood";
        throw std::invalid_argument(errorStream.str().c_str());
      }
      }
    }
    kvl::AtlasMesh::ConstPointer constMesh = mesh.mesh;
    kvl::AtlasMesh::Pointer      mutableMesh =
        const_cast<kvl::AtlasMesh *>(constMesh.GetPointer());
    // Pass the mesh and calculator to it
    optimizer->SetMesh(mutableMesh);
    optimizer->SetCostAndGradientCalculator(calculator.calculator);
  }
  std::pair<double, double> StepOptimizer() {
    const double maximalDeformation = optimizer->Step();
    const double minLogLikelihoodTimesPrior =
        optimizer->GetMinLogLikelihoodTimesPrior();
    return {minLogLikelihoodTimesPrior, maximalDeformation};
  }
};
#endif // GEMS_PYKVLOPTIMIZER_H
