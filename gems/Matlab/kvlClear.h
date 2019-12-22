#ifndef _GEMS2_MATLAB_KVLCLEAR_H
#define _GEMS2_MATLAB_KVLCLEAR_H

#include "kvlMatlabObjectArray.h"
#include "kvlMatlabRunner.h"

namespace kvl {

class Clear : public MatlabRunner {
public:
  /** Smart pointer typedef support. */
  using Self         = Clear;
  using Superclass   = itk::Object;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Clear, itk::Object);

  void Run(int /*nlhs*/, mxArray *plhs[], int nrhs,
           const mxArray *prhs[]) override {
    // std::cout << "I am " << this->GetNameOfClass()
    //          << " and I'm running! " << std::endl;

    // kvlClear( handle )

    // Clear all if no handle is given
    if (nrhs == 0) {
      kvl::MatlabObjectArray::GetInstance()->Clear();
      return;
    }

    if (!mxIsInt64(prhs[0])) {
      mexErrMsgTxt("Incorrect arguments");
    }

    // Retrieve input handle
    const int handle = *(static_cast<int *>(mxGetData(prhs[0])));
    // std::cout << "handle: " << handle << std::endl;
    kvl::MatlabObjectArray::GetInstance()->RemoveObject(handle);
  }

protected:
  Clear() = default;
  ;
  ~Clear() override = default;
  ;

  Clear(const Self &);          // purposely not implemented
  void operator=(const Self &); // purposely not implemented

private:
};

} // end namespace kvl

#endif
