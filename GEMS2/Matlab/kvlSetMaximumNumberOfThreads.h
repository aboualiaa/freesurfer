#include "itkMultiThreaderBase.h"
#include "kvlMatlabObjectArray.h"
#include "kvlMatlabRunner.h"

namespace kvl {

class SetMaximumNumberOfThreads : public MatlabRunner {
public:
  /** Smart pointer typedef support. */
  typedef SetMaximumNumberOfThreads Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SetMaximumNumberOfThreads, itk::Object);

  void Run(int nlhs, mxArray *plhs[], int nrhs,
           const mxArray *prhs[]) override {
    // std::cout << "I am " << this->GetNameOfClass()
    //          << " and I'm running! " << std::endl;

    // kvlSetMaximumNumberOfThreads( maximumNumberOfThreads )

    // Make sure input arguments are correct
    if ((nrhs != 1) || !mxIsDouble(prhs[0])) {
      mexErrMsgTxt("Incorrect arguments");
    }

    // Retrieve input
    const int maximumNumberOfThreads = static_cast<int>(*mxGetPr(prhs[0]));
    // std::cout << "maximumNumberOfThreads: " << maximumNumberOfThreads <<
    // std::endl;
    itk::MultiThreaderBase::SetGlobalDefaultNumberOfThreads(
        maximumNumberOfThreads);
  }

protected:
  SetMaximumNumberOfThreads() = default;
  ;
  ~SetMaximumNumberOfThreads() override = default;
  ;

  SetMaximumNumberOfThreads(const Self &); // purposely not implemented
  void operator=(const Self &);            // purposely not implemented

private:
};

} // end namespace kvl
