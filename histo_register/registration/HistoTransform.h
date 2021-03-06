#ifndef _HISTO_TRANSFORM_H_
#define _HISTO_TRANSFORM_H_
#include "registration/CorresField3D.h"
#include <sbl/image/ImageTransform.h>
#include <sbl/image/MotionField.h>
#include <sbl/math/Geometry.h>
namespace hb {

/*! \file HistoTransform.cc
        \brief The HistoTransform module can be used to project
        points from histology coordinates to MR coordinates
        and from MR coordinates to histology coordinates.
*/

/// The HistoTransform holds the sequence of transformations generated
/// by the histology-to-MR registration commands.
class HistoTransform {
public:
  /// create empty transform;
  HistoTransform() { m_blockOffset = 0; }

  /// load transformation data generated by the registration commands
  bool loadHToB(const String &mrInputPath, const String &mrRegLinPath,
                const String &mrRegPath, const String &histoSplitPath,
                const String &histoOutputPath);
  bool loadBToM(const String &mrInputPath, const String &mrRegLinPath,
                const String &mrRegPath, const String &histoSplitPath,
                const String &histoOutputPath);

  /// compute inverses of mappings; this must be called before projectBackward()
  void computeInversesHToB();
  void computeInversesBToM();

  /// project a point from histology coordinates to MR coordinates
  Point3 projectForward(Point3 point, bool useMrTransform,
                        bool smallHistoCoords, bool verbose) const;

  /// project a point from MR coordinates to histology coordinates
  Point3 projectBackward(Point3 point, bool useMrTransform,
                         bool smallHistoCoords, bool verbose) const;

  /// project between histo (H), block-face (B), and MRI (M) coordinates
  Point3 projectHToB(Point3 point, bool smallHistoCoords, bool verbose) const;
  Point3 projectBToM(Point3 point, bool useMrTransform, bool useNonLinear,
                     bool verbose) const;
  Point3 projectMToB(Point3 point, bool useMrTransform, bool useNonLinear,
                     bool verbose) const;
  Point3 projectBToH(Point3 point, bool smallHistoCoords, bool verbose) const;

  /// get block-face coordinate dimensions
  inline int blockDepth() const { return m_cfSeq.count(); }
  inline int blockWidth() const {
    assertAlways(m_cfSeq.count());
    return m_cfSeq[0].width();
  }
  inline int blockHeight() const {
    assertAlways(m_cfSeq.count());
    return m_cfSeq[0].height();
  }

private:
  // find the index within the motion field array corresponding to the given
  // slice index
  int findHistoArrayIndex(int z) const;

  // mgz to MR slices transform
  AffineTransform3 m_mrTransform;
  AffineTransform3 m_mrTransformInv;

  // linear volume transform
  AffineTransform3 m_regLinTransform;
  AffineTransform3 m_regLinTransformInv;

  // non-linear volume transform
  Array<CorresField3D> m_cfSeq;
  Array<CorresField3D> m_cfSeqInv;

  // non-linear slice transform
  Array<ImageTransform> m_histoTransform;
  Array<ImageTransform> m_histoTransformInv;
  Array<MotionField>    m_mfHisto;
  Array<MotionField>    m_mfHistoInv;
  VectorI               m_histoSliceIndex;
  int                   m_blockOffset;
  VectorD               m_histoShrinkX;
  VectorD               m_histoShrinkY;
};

// register commands, etc. defined in this module
void initHistoTransform();

} // end namespace hb
#endif // _HISTO_TRANSFORM_H_
