#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "GetPot.h"
#include "OrientationPlanesFromParcellationFilter.h"

int main(int argc, char *argv[]) {
  enum { Dimension = 3 };
  using PixelType = double;
  using ImageType = itk::Image<PixelType, Dimension>;

  GetPot cl(argc, const_cast<char **>(argv));
  if (cl.size() == 1 || cl.search(2, "--help", "-h")) {
    std::cout << "Usage: " << std::endl;
    std::cout << argv[0] << " -i input -o output -bb" << std::endl;
    return -1;
  }
  const char *segFile             = cl.follow("", "-i");
  const char *outputFile          = cl.follow("", "-o");
  bool        bb                  = cl.search("-bb");
  using ImageReaderType           = itk::ImageFileReader<ImageType>;
  ImageReaderType::Pointer reader = ImageReaderType::New();
  reader->SetFileName(segFile);
  reader->Update();

  OrientationPlanesFromParcellationFilter<ImageType, ImageType>::Pointer
      filter =
          OrientationPlanesFromParcellationFilter<ImageType, ImageType>::New();
  filter->SetBabyMode(bb);
  filter->SetInput(reader->GetOutput());
  filter->Update();

  std::cout << "up " << filter->GetUpDown() << std::endl;
  std::cout << "front " << filter->GetFrontBack() << std::endl;
  std::cout << "left " << filter->GetLeftRight() << std::endl;

  using ImageWriterType           = itk::ImageFileWriter<ImageType>;
  ImageWriterType::Pointer writer = ImageWriterType::New();
  writer->SetInput(filter->GetOutput());
  writer->SetFileName(outputFile);
  writer->Update();

  return 0;
}
