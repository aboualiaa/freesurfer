#ifndef _GEMS2_MATLAB_KVLWRITERGBIMAGE_H
#define _GEMS2_MATLAB_KVLWRITERGBIMAGE_H

#include "kvlMatlabRunner.h"
#include "kvlMatlabObjectArray.h"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRGBAPixel.h"

namespace kvl {

class WriteRGBImage : public MatlabRunner {
public:
  /** Smart pointer typedef support. */
  using Self         = WriteRGBImage;
  using Superclass   = itk::Object;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  typedef itk::AffineTransform<double, 3> TransformType;
  using RGBAPixelType = itk::RGBAPixel<unsigned char>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(WriteRGBImage, itk::Object);

  void Run(int /*nlhs*/, mxArray * /*plhs*/[], int nrhs,
           const mxArray *prhs[]) override {
    std::cout << "I am " << this->GetNameOfClass() << " and I'm running! "
              << std::endl;

    // kvlWriteImage( image, fileName, transform )

    // Make sure input arguments are correct
    if ((nrhs < 2) || !mxIsInt64(prhs[0]) || !mxIsChar(prhs[1])) {
      mexErrMsgTxt("Incorrect arguments");
    }

    // Retrieve input arguments
    std::cout << "Here! " << std::endl;
    const int imageHandle      = *(static_cast<int *>(mxGetData(prhs[0])));
    const std::string fileName = mxArrayToString(prhs[1]);
    std::cout << "Here! " << std::endl;

    // Retrieve the image
    itk::Object::Pointer object =
        kvl::MatlabObjectArray::GetInstance()->GetObject(imageHandle);
    typedef itk::Image<RGBAPixelType, 3> RGBAImageType;
    std::cout << "Here! " << std::endl;

    // if ( typeid( *object ) != typeid( RGBAImageType ) )
    if (strcmp(typeid(*object).name(),
               typeid(RGBAImageType).name()) != 0 !=
        0) // Eugenio: MAC compatibility
    {
      mexErrMsgTxt("image doesn't refer to the correct ITK object type");
    }

    std::cout << "Here! " << std::endl;

    RGBAImageType::Pointer RGBAimage =
        dynamic_cast<RGBAImageType *>(object.GetPointer());

    std::cout << "Here! " << std::endl;

    // If transform is given, retrieve and apply it
    if (nrhs > 2) {
      // Retrieve the transform
      if (!mxIsInt64(prhs[2])) {
        mexErrMsgTxt("Incorrect arguments");
      }
      using TransformType       = CroppedImageReader::TransformType;
      const int transformHandle = *(static_cast<int *>(mxGetData(prhs[2])));
      object =
          kvl::MatlabObjectArray::GetInstance()->GetObject(transformHandle);
      // if ( typeid( *object ) != typeid( TransformType ) )
      if (strcmp(typeid(*object).name(),
                 typeid(TransformType).name()) != 0 !=
          0) // Eugenio: MAC compatibility
      {
        mexErrMsgTxt("transform doesn't refer to the correct ITK object type");
      }
      TransformType::ConstPointer transform =
          static_cast<const TransformType *>(object.GetPointer());

      // In order not to modify the original image, we create a new one. The
      // proper way of doing this would be to only copy the header information
      // and of course not the pixel intensities, but I'm too lazy now to figure
      // out how to do it in ITK
      // typedef itk::CastImageFilter< ImageType, ImageType >  CasterType;

      typedef itk::CastImageFilter<RGBAImageType, RGBAImageType> RGBACasterType;
      RGBACasterType::Pointer RGBAcaster = RGBACasterType::New();
      RGBAcaster->SetInput(RGBAimage);
      RGBAcaster->Update();
      RGBAimage = RGBAcaster->GetOutput();

      RGBAImageType::PointType newRGBAOrigin;
      RGBAImageType::SpacingType newRGBASpacing;
      RGBAImageType::DirectionType newRGBADirection;

      std::cout << "Print out transformation matrix: " << std::endl;

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          std::cout << "matrix[" << i << "][" << j
                    << "]:" << transform->GetMatrix()[i][j] << std::endl;
        }
      }

      for (int i = 0; i < 3; i++) {
        // Offset part
        newRGBAOrigin[i] = transform->GetOffset()[i];

        // For every column, determine norm (which will be voxel spacing), and
        // normalize direction
        double normOfColumn = 0.0;
        for (int j = 0; j < 3; j++) {
          normOfColumn += pow(transform->GetMatrix()[j][i], 2);
        }
        normOfColumn      = sqrt(normOfColumn);
        newRGBASpacing[i] = normOfColumn;
        std::cout << "newSpacing[" << i << "]: " << newRGBASpacing[i]
                  << std::endl;
        for (int j = 0; j < 3; j++) {
          newRGBADirection[j][i] = transform->GetMatrix()[j][i] / normOfColumn;
        }
      }
      RGBAimage->SetOrigin(newRGBAOrigin);
      RGBAimage->SetSpacing(newRGBASpacing);
      RGBAimage->SetDirection(newRGBADirection);

    } // End test if transform is given

    // Write it out
    using RGBAWriterType               = itk::ImageFileWriter<RGBAImageType>;
    RGBAWriterType::Pointer RGBAwriter = RGBAWriterType::New();
    RGBAwriter->SetInput(RGBAimage);
    RGBAwriter->SetFileName(fileName.c_str());
    RGBAwriter->Update();
    std::cout << "Wrote image to file " << fileName << std::endl;
  }

protected:
  WriteRGBImage()           = default;
  ~WriteRGBImage() override = default;

  WriteRGBImage(const Self &);  // purposely not implemented
  void operator=(const Self &); // purposely not implemented

private:
};

} // end namespace kvl

#endif
