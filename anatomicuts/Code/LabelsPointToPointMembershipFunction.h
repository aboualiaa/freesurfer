#ifndef __LabelsPointToPointMembershipFunction_h
#define __LabelsPointToPointMembershipFunction_h

#include "LabelPerPointMembershipFunction.h"

template <class TVector>
class LabelsPointToPointMembershipFunction
    : public LabelPerPointMembershipFunction<TVector> {
public:
  /** Standard class typedefs */
  using Self = LabelsPointToPointMembershipFunction<TVector>;
  using Superclass = LabelPerPointMembershipFunction<TVector>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Strandard macros */
  itkTypeMacro(LabelsPointToPointMembershipFunction,
               LabelPerPointMembershipFunction);
  itkNewMacro(Self);

  /** Typedef alias for the measurement vectors */
  using MeasurementVectorType = TVector;
  using CentroidType = TVector;

  /** Typedef to represent the length of measurement vectors */
  using MeasurementVectorSizeType =
      typename Superclass::MeasurementVectorSizeType;
  void SetLabelsCount(int count) { this->m_labelsCount = count; }
  virtual double Evaluate(const MeasurementVectorType *m1,
                          const MeasurementVectorType *m2) const;

protected:
  LabelsPointToPointMembershipFunction() : Superclass() {}
  virtual ~LabelsPointToPointMembershipFunction() {}

private:
  int m_labelsCount;
};
#include "LabelsPointToPointMembershipFunction.txx"
#endif
