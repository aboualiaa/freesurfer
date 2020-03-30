#ifndef __kvlAtlasMeshMultiAlphaDrawer_h
#define __kvlAtlasMeshMultiAlphaDrawer_h

#include "itkImage.h"
#include "kvlAtlasMeshRasterizor.h"

namespace kvl {

class AtlasMeshMultiAlphaDrawer : public AtlasMeshRasterizor {
public:
  /** Standard class typedefs */
  using Self         = AtlasMeshMultiAlphaDrawer;
  using Superclass   = AtlasMeshRasterizor;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshMultiAlphaDrawer, itk::Object);

  /** Some typedefs */
  using ImageType = itk::Image<AtlasAlphasType, 3>;

  /** */
  void SetRegions(const ImageType::RegionType &region) {
    m_Image = ImageType::New();
    m_Image->SetRegions(region);
    m_Image->Allocate();
  }

  /** */
  const ImageType *GetImage() const { return m_Image; }

  //
  void Rasterize(const AtlasMesh *mesh);

protected:
  AtlasMeshMultiAlphaDrawer();
  virtual ~AtlasMeshMultiAlphaDrawer();

  //
  bool RasterizeTetrahedron(const AtlasMesh *         mesh,
                            AtlasMesh::CellIdentifier tetrahedronId,
                            int                       threadNumber);

private:
  AtlasMeshMultiAlphaDrawer(const Self &); // purposely not implemented
  void operator=(const Self &);            // purposely not implemented

  ImageType::Pointer m_Image;
};

} // end namespace kvl

#endif
