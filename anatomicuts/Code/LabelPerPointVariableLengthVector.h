#ifndef __LabelPerPointVariableLengthVector_h
#define __LabelPerPointVariableLengthVector_h

#include "itkVariableLengthVector.h"
#include "itkMacro.h"
#include "itkNumericTraits.h"
#include <unordered_map>

template <typename TValueType, class TMesh>
class LabelPerPointVariableLengthVector
    : public VariableLengthVector<TValueType> {
public:
  /** The element type stored at each location in the Array. */
  using ValueType = TValueType;
  using ComponentType = TValueType;
  using RealValueType = typename NumericTraits<ValueType>::RealType;
  using Self = LabelPerPointVariableLengthVector<TValueType, TMesh>;
  using Superclass = VariableLengthVector<TValueType>;
  using MeshType = TMesh;
  using MeshPointerType = typename MeshType::Pointer;
  using ConstMeshPointerType = typename MeshType::ConstPointer;
  using PixelType = typename MeshType::PixelType;                 //??
  using CellType = typename MeshType::CellPixelType;              //??
  using CellAutoPointerType = typename MeshType::CellAutoPointer; //??
  using LabelsMapType = std::unordered_map<int, float>;
  using LabelsDirectionType = typename std::vector<LabelsMapType>;
  LabelPerPointVariableLengthVector() : Superclass() { ; };

  /** Constructor with size. Size can only be changed by assignment */
  LabelPerPointVariableLengthVector(unsigned int dimension)
      : Superclass(dimension) {
    ;
  };
  LabelPerPointVariableLengthVector(ValueType *data, unsigned int sz,
                                    bool LetArrayManageMemory = false)
      : Superclass(data, sz, LetArrayManageMemory){};

  void SetCell(MeshPointerType mesh, int cellID);

  const std::vector<CellType> *GetLabels() const { return &this->m_labels; }
  int GetCellId() const { return this->m_cellId; }
  int GetLength() const { return this->m_length; }
  const LabelsDirectionType GetLabelsPerDirection() const {
    return this->m_labelsPerDirection;
  }
  int GetNumberOfPoints() const { return this->m_numberOfPoints; }
  void Print() const;

private:
  std::vector<CellType> m_labels;
  LabelsDirectionType m_labelsPerDirection;
  int m_cellId;
  int m_numberOfPoints;
  float m_length;
};
#include "LabelPerPointVariableLengthVector.txx"
#endif
