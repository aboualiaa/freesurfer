#ifndef __kvlHistogrammer_h
#define __kvlHistogrammer_h

#include "kvlAtlasMeshRasterizor.h"
#include "itkImage.h"

namespace kvl {

/**
 *
 */
class Histogrammer : public AtlasMeshRasterizor {
public:
  /** Standard class typedefs */
  using Self = Histogrammer;
  using Superclass = AtlasMeshRasterizor;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Histogrammer, itk::Object);

  /** Some typedefs */
  using ImageType = itk::Image<float, 3>;
  using BinnedImageType = itk::Image<int, 3>;
  using HistogramType = std::vector<std::vector<double>>;
  using ConditionalIntensityDistributionType = std::vector<double>;

  /** */
  void SetImage(const ImageType *image) {
    m_Image = image;
    m_BinnedImage = nullptr;
    m_NumberOfBins = 0;
  }

  /** */
  int GetNumberOfBins() const { return m_NumberOfBins; }

  /** */
  void SetConditionalIntensityDistributions(
      const std::vector<ConditionalIntensityDistributionType>
          &conditionalIntensityDistributions) {
    if (m_BinnedImage) {
      // Check if number of bins has changed. If so, forgot cached binned image
      if (conditionalIntensityDistributions[0].size() != m_NumberOfBins) {
        m_BinnedImage = nullptr;
        m_NumberOfBins = 0;
      }
    }

    m_ConditionalIntensityDistributions = conditionalIntensityDistributions;
  }

  /** */
  const std::vector<ConditionalIntensityDistributionType> &
  GetConditionalIntensityDistributions() const {
    return m_ConditionalIntensityDistributions;
  }

  /** */
  const HistogramType &GetHistogram() const { return m_Histogram; }

  /** */
  double GetMinLogLikelihood() const { return m_MinLogLikelihood; }

  /** */
  const BinnedImageType *GetBinnedImage() const { return m_BinnedImage; }

  /** */
  void Rasterize(const AtlasMesh *mesh);

protected:
  Histogrammer();
  virtual ~Histogrammer();

  //
  bool RasterizeTetrahedron(const AtlasMesh *mesh,
                            AtlasMesh::CellIdentifier tetrahedronId,
                            int threadNumber);

private:
  Histogrammer(const Self &);   // purposely not implemented
  void operator=(const Self &); // purposely not implemented

  //
  void ComputeRobustRange(const ImageType *image, double &robustMin,
                          double &robustMax);

  //
  void UpdateBinnedImage();

  //
  ImageType::ConstPointer m_Image;
  std::vector<ConditionalIntensityDistributionType>
      m_ConditionalIntensityDistributions;
  BinnedImageType::Pointer m_BinnedImage;
  int m_NumberOfBins;
  HistogramType m_Histogram;
  double m_MinLogLikelihood;

  //
  std::vector<HistogramType> m_ThreadSpecificHistograms;
  std::vector<double> m_ThreadSpecificMinLogLikelihoods;
};

} // end namespace kvl

#endif
