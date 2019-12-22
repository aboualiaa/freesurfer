#ifndef _GEMS2_MATLAB_KVLCHANGEK_H
#define _GEMS2_MATLAB_KVLCHANGEK_H

#include "kvlAtlasMesh.h"
#include "kvlMatlabObjectArray.h"
#include "kvlMatlabRunner.h"
#include "mex.h"

namespace kvl {

class ChangeK : public MatlabRunner {
public:
  /** Smart pointer typedef support. */
  using Self         = ChangeK;
  using Superclass   = itk::Object;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ChangeK, itk::Object);

  void Run(int nlhs, mxArray *plhs[], int nrhs,
           const mxArray *prhs[]) override {
    // std::cout << "I am " << this->GetNameOfClass()
    //          << " and I'm running! " << std::endl;

    // kvlChangeK( mesh, scaleFactor,label )
    mexPrintf("Right one");

    // Make sure input arguments are correct
    if ((nrhs != 3) || (nlhs != 0) || !mxIsDouble(prhs[1])) {
      mexErrMsgTxt("Incorrect arguments");
    }

    // Retrieve input arguments
    const int meshHandle = *(static_cast<int *>(mxGetData(prhs[0])));
    itk::Object::ConstPointer object =
        kvl::MatlabObjectArray::GetInstance()->GetObject(meshHandle);
    // if ( typeid( *object ) != typeid( kvl::AtlasMesh ) )
    if (strcmp(typeid(*object).name(),
               typeid(kvl::AtlasMesh).name()) !=
        0) // Eugenio: MAC compatibility
    {
      mexErrMsgTxt("Not an atlas mesh object");
    }
    kvl::AtlasMesh::ConstPointer constMesh =
        dynamic_cast<const kvl::AtlasMesh *>(object.GetPointer());
    kvl::AtlasMesh::Pointer mesh =
        const_cast<kvl::AtlasMesh *>(constMesh.GetPointer());

    double scaleFactor    = *(mxGetPr(prhs[1]));
    const int classNumber = static_cast<int>(*(mxGetPr(prhs[2])));

    if (scaleFactor < 1) {
      scaleFactor = scaleFactor - 1;
    }

    else if (scaleFactor > 1) {
      scaleFactor = scaleFactor - 1;
    } else {
      scaleFactor = 1;
    }

    // std::cout << "mesh: " << mesh.GetPointer() << std::endl;
    // std::cout << "scaleFactor: " << scaleFactor << std::endl;

    // Get the alpha from each tetrahedra node corresponding to the given label
    // and scale the K by the scaling factor times the probability
    for (AtlasMesh::CellsContainer::Iterator cellIt = mesh->GetCells()->Begin();
         cellIt != mesh->GetCells()->End(); ++cellIt) {

      AtlasMesh::CellType *cell = cellIt.Value();
      if (cell->GetType() == AtlasMesh::CellType::TETRAHEDRON_CELL) {

        AtlasMesh::CellType::PointIdIterator pit = cell->PointIdsBegin();
        AtlasAlphasType alphas0 =
            mesh->GetPointData()->ElementAt(*pit).m_Alphas;
        ++pit;
        AtlasAlphasType alphas1 =
            mesh->GetPointData()->ElementAt(*pit).m_Alphas;
        ++pit;
        AtlasAlphasType alphas2 =
            mesh->GetPointData()->ElementAt(*pit).m_Alphas;
        ++pit;
        AtlasAlphasType alphas3 =
            mesh->GetPointData()->ElementAt(*pit).m_Alphas;
        ++pit;
        mesh->GetCellData()
            ->ElementAt(cellIt->Index())
            .m_ReferenceVolumeTimesK *=
            (1 +
             (scaleFactor / 4) * (alphas0[classNumber] + alphas1[classNumber] +
                                  alphas2[classNumber] + alphas3[classNumber]));
      }
    }
  }

protected:
  ChangeK() = default;
  ;
  ~ChangeK() override = default;
  ;

  ChangeK(const Self &);        // purposely not implemented
  void operator=(const Self &); // purposely not implemented

private:
};

} // end namespace kvl

#endif
