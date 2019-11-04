
#ifndef H_ITK_MGH_IMAGE_IO_FACTORY_H
#define H_ITK_MGH_IMAGE_IO_FACTORY_H

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

namespace itk {

class ITK_EXPORT MGHImageIOFactory : public ObjectFactoryBase {
public:
  /** Standard class typedefs **/
  using Self = MGHImageIOFactory;
  using Superclass = ObjectFactoryBase;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Class methods used to interface with the registered factories **/
  virtual const char *GetITKSourceVersion() const;
  virtual const char *GetDescription() const;

  /** Method for class instantiation **/
  itkFactorylessNewMacro(Self);

  /** RTTI (and related methods) **/
  itkTypeMacro(MGHImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type **/
  static void RegisterOneFactory() {
    MGHImageIOFactory::Pointer MGHFactory = MGHImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(MGHFactory);
  }

protected:
  MGHImageIOFactory();
  ~MGHImageIOFactory();

private:
  MGHImageIOFactory(const Self &); // purposely not implemented
  void operator=(const Self &);    // purposely not implemented

}; // end class MGHImageIOFactory

} // end namespace itk

#endif // H_ITK_MGH_IMAGE_IO_FACTORY_H
