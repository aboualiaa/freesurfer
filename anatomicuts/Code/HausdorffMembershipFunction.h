#ifndef __HausdorffMembershipFunction_h
#define __HausdorffMembershipFunction_h

#include "LabelPerPointMembershipFunction.h"
// using namespace itk;
//#include "HausdorffMembershipFunction.txx"

template <class TVector>
class HausdorffMembershipFunction
    : public LabelPerPointMembershipFunction<TVector> {
public:
  /** Standard class typedefs */
  using Self = HausdorffMembershipFunction<TVector>;
  using Superclass = LabelPerPointMembershipFunction<TVector>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Strandard macros */
  itkTypeMacro(HausdorffMembershipFunction, MembershipFunctionBase);
  itkNewMacro(Self);

  /** Typedef alias for the measurement vectors */
  using MeasurementVectorType = TVector;
  using CentroidType = TVector;

  /** Typedef to represent the length of measurement vectors */
  using MeasurementVectorSizeType =
      typename Superclass::MeasurementVectorSizeType;

  /**
   * Method to get probability of an instance. The return value is the
   * value of the density function, not probability. */
  double Evaluate(const MeasurementVectorType *m1,
                  const MeasurementVectorType *m2) const;
  // double Evaluate(const MeasurementVectorType &measurement) const{ std::cout
  // << "not implemented " << std::endl;return -1;};

protected:
  HausdorffMembershipFunction();
  virtual ~HausdorffMembershipFunction() {}
};
#include "HausdorffMembershipFunction.txx"
#endif
