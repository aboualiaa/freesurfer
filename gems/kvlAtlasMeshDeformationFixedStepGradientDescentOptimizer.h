#ifndef __kvlAtlasMeshDeformationFixedStepGradientDescentOptimizer_h
#define __kvlAtlasMeshDeformationFixedStepGradientDescentOptimizer_h

#include "kvlAtlasMeshDeformationOptimizer.h"

namespace kvl {

/**
 *
 */
class AtlasMeshDeformationFixedStepGradientDescentOptimizer
    : public AtlasMeshDeformationOptimizer {
public:
  /** Standard class typedefs */
  using Self         = AtlasMeshDeformationFixedStepGradientDescentOptimizer;
  using Superclass   = AtlasMeshDeformationOptimizer;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshDeformationFixedStepGradientDescentOptimizer,
               AtlasMeshDeformationOptimizer);

  //
  void SetStepSize(double stepSize) { m_StepSize = stepSize; }

  double GetStepSize() const { return m_StepSize; }

protected:
  AtlasMeshDeformationFixedStepGradientDescentOptimizer();
  virtual ~AtlasMeshDeformationFixedStepGradientDescentOptimizer();

  double FindAndOptimizeNewSearchDirection();

private:
  AtlasMeshDeformationFixedStepGradientDescentOptimizer(
      const Self &);            // purposely not implemented
  void operator=(const Self &); // purposely not implemented

  double m_StepSize;
  double m_LineSearchStopCriterion;
};

} // end namespace kvl

#endif
