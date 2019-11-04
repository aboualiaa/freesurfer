#ifndef kvlGMMLikelihoodImageFilter_h
#define kvlGMMLikelihoodImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkArray.h"
#include <vector>

namespace kvl {
/**
 *
 * Input is one or more intensity images. Based on bunch of means and
 * covariances, computes likelihoods for each class as output.
 *
 * Design is taken from itkComposeImageFilter. I'm forcing the output to be of
 * the form itk::Image< itk::Array<xxx>, yyy> instead of the more general
 * outputs that itkComposeImageFilter can handle (such as itkVectorImage),
 * because I known in advance most of pixels will be background which I can then
 * encode by assigning an itkArray of size 0 there (saving both memory and the
 * need for some other signal that no information was present to compute the
 * class likelihoods in specific voxels)
 *
 */

template <typename TInputImage>
class GMMLikelihoodImageFilter
    : public itk::ImageToImageFilter<
          TInputImage, itk::Image<itk::Array<typename TInputImage::PixelType>,
                                  TInputImage::ImageDimension>> {
public:
  using Self = GMMLikelihoodImageFilter<TInputImage>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  itkNewMacro(Self);

  itkTypeMacro(GMMLikelihoodImageFilter, itk::ImageToImageFilter);

  itkStaticConstMacro(Dimension, unsigned int, TInputImage::ImageDimension);

  using InputImageType = TInputImage;
  using InputPixelType = typename InputImageType::PixelType;
  using OutputImageType =
      itk::Image<itk::Array<InputPixelType>, TInputImage::ImageDimension>;
  using OutputPixelType = typename OutputImageType::PixelType;
  using OutputPixelValueType =
      typename itk::NumericTraits<OutputPixelType>::ValueType;
  using RegionType = typename InputImageType::RegionType;
  using Superclass = itk::ImageToImageFilter<InputImageType, OutputImageType>;

  /** */
  void SetParameters(const std::vector<vnl_vector<double>> &means,
                     const std::vector<vnl_matrix<double>> &variances,
                     const std::vector<double> &mixtureWeights,
                     const std::vector<int> &numberOfGaussiansPerClass);

protected:
  GMMLikelihoodImageFilter();

  virtual void BeforeThreadedGenerateData();

  virtual void ThreadedGenerateData(const RegionType &outputRegionForThread,
                                    itk::ThreadIdType);

private:
  GMMLikelihoodImageFilter(const Self &);
  void operator=(const Self &);

  std::vector<vnl_vector<double>> m_Means;
  std::vector<std::vector<vnl_matrix<double>>> m_Precisions;
  std::vector<double> m_piTermMultiv;
  std::vector<std::vector<double>> m_OneOverSqrtDetCov;
  std::vector<double> m_MixtureWeights;
  std::vector<int> m_NumberOfGaussiansPerClass;
};

} // namespace kvl

#include "kvlGMMLikelihoodImageFilter.hxx"

#endif
