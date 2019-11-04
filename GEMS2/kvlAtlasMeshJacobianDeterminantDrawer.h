#ifndef __kvlAtlasMeshJacobianDeterminantDrawer_h
#define __kvlAtlasMeshJacobianDeterminantDrawer_h

#include "kvlAtlasMeshRasterizor.h"
#include "itkImage.h"

namespace kvl {

/**
 *
 */
class AtlasMeshJacobianDeterminantDrawer : public AtlasMeshRasterizor {
public:
  /** Standard class typedefs */
  using Self = AtlasMeshJacobianDeterminantDrawer;
  using Superclass = AtlasMeshRasterizor;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshJacobianDeterminantDrawer, itk::Object);

  /** Some typedefs */
  using ImageType = itk::Image<float, 3>;

  /** */
  void SetRegions(const ImageType::RegionType &region) {
    m_Image = ImageType::New();
    m_Image->SetRegions(region);
    m_Image->Allocate();
    m_Image->FillBuffer(0);
  }

  /** */
  const ImageType *GetImage() const { return m_Image; }

protected:
  AtlasMeshJacobianDeterminantDrawer();
  virtual ~AtlasMeshJacobianDeterminantDrawer();

  //
  bool RasterizeTetrahedron(const AtlasMesh *mesh,
                            AtlasMesh::CellIdentifier tetrahedronId,
                            int threadNumber);

private:
  AtlasMeshJacobianDeterminantDrawer(const Self &); // purposely not implemented
  void operator=(const Self &);                     // purposely not implemented

  //
  ImageType::Pointer m_Image;
};

} // end namespace kvl

#endif
