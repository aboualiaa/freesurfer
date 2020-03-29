#ifndef __LabelsHistogramMembershipFunction_h
#define __LabelsHistogramMembershipFunction_h

#include "LabelPerPointMembershipFunction.h"

template <class TVector>
class LabelsHistogramMembershipFunction
    : public LabelPerPointMembershipFunction<TVector> {
public:
  /** Standard class typedefs */
  using Self         = LabelsHistogramMembershipFunction<TVector>;
  using Superclass   = LabelPerPointMembershipFunction<TVector>;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Strandard macros */
  itkTypeMacro(LabelsHistogramMembershipFunction, MembershipFunctionBase);
  itkNewMacro(Self);

  /** Typedef alias for the measurement vectors */
  using MeasurementVectorType = TVector;
  using CentroidType          = TVector;

  /** Typedef to represent the length of measurement vectors */
  using MeasurementVectorSizeType =
      typename Superclass::MeasurementVectorSizeType;

  virtual double Evaluate(const MeasurementVectorType *m1,
                          const MeasurementVectorType *m2) const;

protected:
  LabelsHistogramMembershipFunction() : Superclass(){};
  virtual ~LabelsHistogramMembershipFunction() {}

private:
};

#endif
