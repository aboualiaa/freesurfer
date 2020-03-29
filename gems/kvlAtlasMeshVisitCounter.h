#ifndef __kvlAtlasMeshVisitCounter_h
#define __kvlAtlasMeshVisitCounter_h

#include "itkImage.h"
#include "kvlAtlasMeshRasterizor.h"

namespace kvl {

/**
 *
 */
class AtlasMeshVisitCounter : public AtlasMeshRasterizor {
public:
  /** Standard class typedefs */
  using Self         = AtlasMeshVisitCounter;
  using Superclass   = AtlasMeshRasterizor;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshVisitCounter, itk::Object);

  /** Some typedefs */
  using ImageType = itk::Image<int, 3>;

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
  AtlasMeshVisitCounter();
  virtual ~AtlasMeshVisitCounter();

  //
  bool RasterizeTetrahedron(const AtlasMesh *         mesh,
                            AtlasMesh::CellIdentifier tetrahedronId,
                            int                       threadNumber);

private:
  AtlasMeshVisitCounter(const Self &); // purposely not implemented
  void operator=(const Self &);        // purposely not implemented

  //
  ImageType::Pointer m_Image;
};

} // end namespace kvl

#endif
