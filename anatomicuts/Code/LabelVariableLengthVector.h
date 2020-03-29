#ifndef __LabelVariableLengthVector_h
#define __LabelVariableLengthVector_h

#include "itkMacro.h"
#include "itkNumericTraits.h"
#include "itkVariableLengthVector.h"
#include <vnl/algo/vnl_determinant.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_transpose.h>
#include <vnl/vnl_vector.h>

template <typename TValueType, class TMesh>
class LabelVariableLengthVector : public VariableLengthVector<TValueType> {
public:
  /** The element type stored at each location in the Array. */
  using ValueType           = TValueType;
  using ComponentType       = TValueType;
  using RealValueType       = typename NumericTraits<ValueType>::RealType;
  using Self                = LabelVariableLengthVector<TValueType, TMesh>;
  using Superclass          = VariableLengthVector<TValueType>;
  using MeshType            = TMesh;
  using MeshPointerType     = typename MeshType::Pointer;
  using CellType            = typename MeshType::CellType;        //??
  using CellAutoPointerType = typename MeshType::CellAutoPointer; //??

  LabelVariableLengthVector() : Superclass() { ; };

  /** Constructor with size. Size can only be changed by assignment */
  LabelVariableLengthVector(unsigned int dimension) : Superclass(dimension) {
    ;
  };
  LabelVariableLengthVector(ValueType *data, unsigned int sz,
                            bool LetArrayManageMemory = false)
      : Superclass(data, sz, LetArrayManageMemory){};
  void SetCell(MeshPointerType mesh, int cellID);
  void Print() const {};

private:
};

#endif
