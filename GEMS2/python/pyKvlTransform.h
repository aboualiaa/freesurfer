#ifndef GEMS_PYKVLTRANSFORM_H
#define GEMS_PYKVLTRANSFORM_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "kvlCroppedImageReader.h"

using TransformType = kvl::CroppedImageReader::TransformType;
using TransformPointer = TransformType::Pointer;

class KvlTransform {
public:
  // Python accessible
  KvlTransform(const py::array_t<double> &transformMatrix);
  [[nodiscard]] py::array_t<double> AsNumpyArray() const;

  // C++ use only
  TransformPointer m_transform;

  KvlTransform(TransformPointer transform) : m_transform(transform){};
  [[nodiscard]] const TransformPointer GetTransform() const { return m_transform; }
};
#endif // GEMS_PYKVLTRANSFORM_H
