#ifndef __kvlAverageAtlasMeshPositionCostAndGradientCalculator_h
#define __kvlAverageAtlasMeshPositionCostAndGradientCalculator_h

#include "kvlAtlasMeshPositionCostAndGradientCalculator.h"

namespace kvl {

/**
 *
 */
class AverageAtlasMeshPositionCostAndGradientCalculator
    : public AtlasMeshPositionCostAndGradientCalculator {
public:
  /** Standard class typedefs */
  using Self         = AverageAtlasMeshPositionCostAndGradientCalculator;
  using Superclass   = AtlasMeshPositionCostAndGradientCalculator;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(AverageAtlasMeshPositionCostAndGradientCalculator,
               AtlasMeshPositionCostAndGradientCalculator);

  //
  void SetPositionsAndKs(
      std::vector<AtlasMesh::PointsContainer::ConstPointer> positions,
      std::vector<double>                                   Ks) {
    m_Positions = positions;
    m_Ks        = Ks;
    m_CachedInternalMeshes.clear();
  }

  //
  void Rasterize(const AtlasMesh *mesh);

protected:
  AverageAtlasMeshPositionCostAndGradientCalculator();
  virtual ~AverageAtlasMeshPositionCostAndGradientCalculator();

private:
  AverageAtlasMeshPositionCostAndGradientCalculator(
      const Self &);            // purposely not implemented
  void operator=(const Self &); // purposely not implemented

  //
  std::vector<AtlasMesh::PointsContainer::ConstPointer> m_Positions;
  std::vector<double>                                   m_Ks;

  std::vector<AtlasMesh::Pointer> m_CachedInternalMeshes;
};

} // end namespace kvl

#endif
