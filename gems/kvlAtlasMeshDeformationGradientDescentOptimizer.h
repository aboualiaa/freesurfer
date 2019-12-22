#ifndef __kvlAtlasMeshDeformationGradientDescentOptimizer_h
#define __kvlAtlasMeshDeformationGradientDescentOptimizer_h

#include "kvlAtlasMeshDeformationOptimizer.h"

namespace kvl {

/**
 *
 */
class AtlasMeshDeformationGradientDescentOptimizer
    : public AtlasMeshDeformationOptimizer {
public:
  /** Standard class typedefs */
  using Self = AtlasMeshDeformationGradientDescentOptimizer;
  using Superclass = AtlasMeshDeformationOptimizer;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshDeformationGradientDescentOptimizer,
               AtlasMeshDeformationOptimizer);

protected:
  AtlasMeshDeformationGradientDescentOptimizer();
  virtual ~AtlasMeshDeformationGradientDescentOptimizer();

  void Initialize();

  double FindAndOptimizeNewSearchDirection();

private:
  AtlasMeshDeformationGradientDescentOptimizer(
      const Self &);            // purposely not implemented
  void operator=(const Self &); // purposely not implemented

  double m_OldCost;
  AtlasPositionGradientContainerType::Pointer m_OldGradient;
  AtlasPositionGradientContainerType::Pointer m_OldSearchDirection;
  double m_AlphaUsedLastTime;

  double m_StartDistance;
};

} // end namespace kvl

#endif
