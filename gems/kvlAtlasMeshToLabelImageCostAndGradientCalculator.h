#ifndef __kvlAtlasMeshToLabelImageCostAndGradientCalculator_h
#define __kvlAtlasMeshToLabelImageCostAndGradientCalculator_h

#include "kvlAtlasMeshPositionCostAndGradientCalculator.h"
#include "kvlCompressionLookupTable.h"

namespace kvl {

/**
 *
 */
class AtlasMeshToLabelImageCostAndGradientCalculator
    : public AtlasMeshPositionCostAndGradientCalculator {
public:
  /** Standard class typedefs */
  using Self         = AtlasMeshToLabelImageCostAndGradientCalculator;
  using Superclass   = AtlasMeshPositionCostAndGradientCalculator;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtlasMeshToLabelImageCostAndGradientCalculator,
               AtlasMeshPositionCostAndGradientCalculator);

  // Some typedefs
  using LabelImageType = CompressionLookupTable::ImageType;

  // Set label image
  void SetLabelImage(const LabelImageType *        labelImage,
                     const CompressionLookupTable *lookupTable);

protected:
  AtlasMeshToLabelImageCostAndGradientCalculator();
  virtual ~AtlasMeshToLabelImageCostAndGradientCalculator();

  void AddDataContributionOfTetrahedron(
      const AtlasMesh::PointType &p0, const AtlasMesh::PointType &p1,
      const AtlasMesh::PointType &p2, const AtlasMesh::PointType &p3,
      const AtlasAlphasType &alphasInVertex0,
      const AtlasAlphasType &alphasInVertex1,
      const AtlasAlphasType &alphasInVertex2,
      const AtlasAlphasType &alphasInVertex3, double &priorPlusDataCost,
      AtlasPositionGradientType &gradientInVertex0,
      AtlasPositionGradientType &gradientInVertex1,
      AtlasPositionGradientType &gradientInVertex2,
      AtlasPositionGradientType &gradientInVertex3);

private:
  AtlasMeshToLabelImageCostAndGradientCalculator(
      const Self &);            // purposely not implemented
  void operator=(const Self &); // purposely not implemented

  //
  LabelImageType::ConstPointer         m_LabelImage;
  CompressionLookupTable::ConstPointer m_CompressionLookupTable;
};

} // end namespace kvl

#endif
