#ifndef kvlTetrahedronInteriorIterator_h
#define kvlTetrahedronInteriorIterator_h

#include "kvlTetrahedronInteriorConstIterator.h"

namespace kvl {

template <typename TPixel>
class TetrahedronInteriorIterator
    : public TetrahedronInteriorConstIterator<TPixel> {
public:
  /** Standard class typedefs. */
  using Self       = TetrahedronInteriorIterator<TPixel>;
  using Superclass = TetrahedronInteriorConstIterator<TPixel>;

  /**
   * Index typedef support. While these were already typdef'ed in the
   * superclass, they need to be redone here for this subclass to compile
   * properly with gcc.
   */
  /** Types inherited from the Superclass */
  using InternalPixelType = typename Superclass::InternalPixelType;
  using PixelType         = typename Superclass::PixelType;
  using ImageType         = typename Superclass::ImageType;
  using PointType         = typename Superclass::PointType;

  /** Constructor */
  TetrahedronInteriorIterator(ImageType *ptr, const PointType &p0,
                              const PointType &p1, const PointType &p2,
                              const PointType &p3);

  /**  */
  PixelType &Value() {
    return *(const_cast<InternalPixelType *>(this->m_Position));
  }

protected:
private:
};

} // end namespace kvl

#include "kvlTetrahedronInteriorIterator.hxx"

#endif
