#ifndef __kvlAtlasMeshDeformationLBFGSOptimizer_h
#define __kvlAtlasMeshDeformationLBFGSOptimizer_h

#include "kvlAtlasMeshDeformationOptimizer.h"

namespace kvl {

/**
 *
 */
class AtlasMeshDeformationLBFGSOptimizer
    : public AtlasMeshDeformationOptimizer {
public:
  /** Standard class typedefs */
  using Self = AtlasMeshDeformationLBFGSOptimizer;
  using Superclass = AtlasMeshDeformationOptimizer;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshDeformationLBFGSOptimizer,
               AtlasMeshDeformationOptimizer);

  //
  void SetMaximumMemoryLength(int maximumMemoryLength) {
    m_MaximumMemoryLength = maximumMemoryLength;
  }

  //
  int GetMaximumMemoryLength() const { return m_MaximumMemoryLength; }

protected:
  AtlasMeshDeformationLBFGSOptimizer();
  virtual ~AtlasMeshDeformationLBFGSOptimizer();

  void Initialize();

  double FindAndOptimizeNewSearchDirection();

private:
  AtlasMeshDeformationLBFGSOptimizer(const Self &); // purposely not implemented
  void operator=(const Self &);                     // purposely not implemented

  double m_OldCost;
  AtlasPositionGradientContainerType::Pointer m_OldGradient;
  AtlasPositionGradientContainerType::Pointer m_OldSearchDirection;
  double m_AlphaUsedLastTime;

  std::vector<AtlasPositionGradientContainerType::ConstPointer> m_Ss;
  std::vector<AtlasPositionGradientContainerType::ConstPointer> m_Ys;
  std::vector<double> m_InverseRhos;

  double m_StartDistance;

  int m_MaximumMemoryLength;
};

} // end namespace kvl

#endif