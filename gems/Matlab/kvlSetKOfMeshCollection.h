#include "kvlAtlasMesh.h"
#include "kvlMatlabObjectArray.h"
#include "kvlMatlabRunner.h"

namespace kvl {

class SetKOfMeshCollection : public MatlabRunner {
public:
  /** Smart pointer typedef support. */
  typedef SetKOfMeshCollection          Self;
  typedef itk::Object                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(SetKOfMeshCollection, itk::Object);

  void Run(int nlhs, mxArray *plhs[], int nrhs,
           const mxArray *prhs[]) override {
    // std::cout << "I am " << this->GetNameOfClass()
    //          << " and I'm running! " << std::endl;

    // kvlSetKOfMeshCollection( meshCollection, K )

    // Make sure input arguments are correct
    if ((nrhs != 2) || !mxIsInt64(prhs[0])) {
      mexErrMsgTxt("Incorrect arguments");
    }

    // Retrieve mesh collection
    const int meshCollectionHandle = *(static_cast<int *>(mxGetData(prhs[0])));
    itk::Object::ConstPointer object =
        kvl::MatlabObjectArray::GetInstance()->GetObject(meshCollectionHandle);
    // if ( typeid( *object ) != typeid( kvl::AtlasMeshCollection ) )
    if (strcmp(typeid(*object).name(),
               typeid(kvl::AtlasMeshCollection)
                   .name())) // Eugenio: MAC compatibility
    {
      mexErrMsgTxt("Not an atlas mesh collection object");
    }
    AtlasMeshCollection::ConstPointer constMeshCollection =
        static_cast<const AtlasMeshCollection *>(object.GetPointer());
    AtlasMeshCollection::Pointer meshCollection =
        const_cast<kvl::AtlasMeshCollection *>(
            constMeshCollection.GetPointer());

    // Retrieve K
    const auto K = static_cast<float>(*(mxGetPr(prhs[1])));

    //
    meshCollection->SetK(K);
  }

protected:
  SetKOfMeshCollection() = default;
  ;
  ~SetKOfMeshCollection() override = default;
  ;

  SetKOfMeshCollection(const Self &); // purposely not implemented
  void operator=(const Self &);       // purposely not implemented

private:
};

} // end namespace kvl
