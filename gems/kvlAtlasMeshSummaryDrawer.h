#ifndef __kvlAtlasMeshSummaryDrawer_h
#define __kvlAtlasMeshSummaryDrawer_h

#include "kvlAtlasMeshRasterizor.h"
#include "itkImage.h"
#include "itkRGBAPixel.h"
#include "kvlCompressionLookupTable.h"

namespace kvl {

/**
 *
 */
class AtlasMeshSummaryDrawer : public AtlasMeshRasterizor {
public:
  /** Standard class typedefs */
  using Self = AtlasMeshSummaryDrawer;
  using Superclass = AtlasMeshRasterizor;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshSummaryDrawer, itk::Object);

  /** Some typedefs */
  using ImageType = itk::Image<itk::RGBAPixel<unsigned char>, 3>;

  /** */
  void SetRegions(const ImageType::RegionType &region) {
    m_Image = ImageType::New();
    m_Image->SetRegions(region);
    m_Image->Allocate();
  }

  //
  void SetCompressionLookupTable(const CompressionLookupTable *lookupTable) {
    m_CompressionLookupTable = lookupTable;
  }

  /** */
  const ImageType *GetImage() const { return m_Image; }

protected:
  AtlasMeshSummaryDrawer();
  virtual ~AtlasMeshSummaryDrawer();

  //
  bool RasterizeTetrahedron(const AtlasMesh *mesh,
                            AtlasMesh::CellIdentifier tetrahedronId,
                            int threadNumber);

private:
  AtlasMeshSummaryDrawer(const Self &); // purposely not implemented
  void operator=(const Self &);         // purposely not implemented

  ImageType::Pointer m_Image;
  CompressionLookupTable::ConstPointer m_CompressionLookupTable;
};

} // end namespace kvl

#endif
