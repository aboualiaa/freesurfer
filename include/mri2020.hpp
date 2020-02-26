//
// Created by Ahmed Abou-Aliaa on 22.02.20.
//

#ifndef MRI2020_HPP
#define MRI2020_HPP

#include "immintrin.h"
#include "mri.h"

#include <limits>

#define FS_ALWAYS_INLINE inline __attribute__((always_inline))

namespace fs::mri {

using semi_new_vox_getter = float (&)(const MRI *, int, int, int, int);
using semi_new_vox_setter = int (&)(MRI *, int, int, int, int, float);
using new_vox_getter      = float (&)(const MRI *, size_t);
using new_vox_setter      = int (&)(MRI *, size_t, float);

template <typename T>
FS_ALWAYS_INLINE auto semi_new_vox_getter_chunked(const MRI *mri, int column,
                                                  int row, int slice, int frame)
    -> float {

  return (float)*((T *)mri->chunk + column + row * mri->vox_per_row +
                  slice * mri->vox_per_slice + frame * mri->vox_per_vol);
}

template <typename T>
FS_ALWAYS_INLINE auto new_vox_getter_chunked(const MRI *mri, size_t index)
    -> float {
  return (float)*((T *)mri->chunk + index);
}

inline auto get_typed_semi_new_vox_getter_chunked(MRI *mri)
    -> semi_new_vox_getter {

  static semi_new_vox_getter char_func =
      fs::mri::semi_new_vox_getter_chunked<unsigned char>;
  static semi_new_vox_getter int_func =
      fs::mri::semi_new_vox_getter_chunked<int>;
  static semi_new_vox_getter long_func =
      fs::mri::semi_new_vox_getter_chunked<long>;
  static semi_new_vox_getter float_func =
      fs::mri::semi_new_vox_getter_chunked<float>;
  static semi_new_vox_getter short_func =
      fs::mri::semi_new_vox_getter_chunked<short>;

  switch (mri->type) {
  case MRI_UCHAR:
    return char_func;
  case MRI_INT:
    return int_func;
  case MRI_LONG:
    return long_func;
  case MRI_FLOAT:
    return float_func;
  case MRI_SHORT:
    return short_func;
  }
}

inline auto get_typed_new_vox_getter_chunked(MRI *mri) -> new_vox_getter {

  static new_vox_getter char_func =
      fs::mri::new_vox_getter_chunked<unsigned char>;
  static new_vox_getter int_func   = fs::mri::new_vox_getter_chunked<int>;
  static new_vox_getter long_func  = fs::mri::new_vox_getter_chunked<long>;
  static new_vox_getter float_func = fs::mri::new_vox_getter_chunked<float>;
  static new_vox_getter short_func = fs::mri::new_vox_getter_chunked<short>;

  switch (mri->type) {
  case MRI_UCHAR:
    return char_func;
  case MRI_INT:
    return int_func;
  case MRI_LONG:
    return long_func;
  case MRI_FLOAT:
    return float_func;
  case MRI_SHORT:
    return short_func;
  }
}

template <typename T>
FS_ALWAYS_INLINE auto semi_new_vox_getter_non_chunked(const MRI *mri,
                                                      int column, int row,
                                                      int slice, int frame)
    -> float {
  return ((float)(((T *)mri->slices[slice + (frame)*mri->depth][row])[column]));
}

template <typename T>
FS_ALWAYS_INLINE auto semi_new_vox_setter_chunked(MRI *mri, int column, int row,
                                                  int slice, int frame,
                                                  float voxval) -> int {
  if (voxval < std::numeric_limits<T>::min()) {
    voxval = std::numeric_limits<T>::min();
  }
  if (voxval > std::numeric_limits<T>::max()) {
    voxval = std::numeric_limits<T>::max();
  }

  *((T *)mri->chunk + column + row * mri->vox_per_row +
    slice * mri->vox_per_slice + frame * mri->vox_per_vol) = nint(voxval);

  return (0);
}

template <typename T>
FS_ALWAYS_INLINE auto new_vox_setter_chunked(MRI *mri, size_t index,
                                             float voxval) -> int {
  if (voxval < std::numeric_limits<T>::min()) {
    voxval = std::numeric_limits<T>::min();
  }
  if (voxval > std::numeric_limits<T>::max()) {
    voxval = std::numeric_limits<T>::max();
  }

  *((T *)mri->chunk + index) = voxval;
  return (0);
}

template <typename T>
FS_ALWAYS_INLINE auto semi_new_vox_setter_non_chunked(MRI *mri, int column,
                                                      int row, int slice,
                                                      int frame, float voxval)
    -> int {
  // clipping
  if (voxval < std::numeric_limits<T>::min()) {
    voxval = std::numeric_limits<T>::min();
  }
  if (voxval > std::numeric_limits<T>::max()) {
    voxval = std::numeric_limits<T>::max();
  }

  switch (mri->type) {
  case MRI_UCHAR:
    MRIseq_vox(mri, column, row, slice, frame) = nint(voxval);
    break;
  case MRI_SHORT:
    MRISseq_vox(mri, column, row, slice, frame) = nint(voxval);
    break;
  case MRI_RGB:
  case MRI_INT:
    MRIIseq_vox(mri, column, row, slice, frame) = nint(voxval);
    break;
  case MRI_LONG:
    MRILseq_vox(mri, column, row, slice, frame) = nint(voxval);
    break;
  case MRI_FLOAT:
    MRIFseq_vox(mri, column, row, slice, frame) = voxval;
    break;
  default:
    return (1);
    break;
  }
  return (0);
}

inline auto get_typed_semi_new_vox_setter_chunked(MRI *mri)
    -> semi_new_vox_setter {

  static semi_new_vox_setter char_func =
      fs::mri::semi_new_vox_setter_chunked<unsigned char>;
  static semi_new_vox_setter int_func =
      fs::mri::semi_new_vox_setter_chunked<int>;
  static semi_new_vox_setter long_func =
      fs::mri::semi_new_vox_setter_chunked<long>;
  static semi_new_vox_setter float_func =
      fs::mri::semi_new_vox_setter_chunked<float>;
  static semi_new_vox_setter short_func =
      fs::mri::semi_new_vox_setter_chunked<short>;

  switch (mri->type) {
  case MRI_UCHAR:
    return char_func;
  case MRI_INT:
    return int_func;
  case MRI_LONG:
    return long_func;
  case MRI_FLOAT:
    return float_func;
  case MRI_SHORT:
    return short_func;
  }
}

inline auto get_typed_new_vox_setter_chunked(MRI *mri) -> new_vox_setter {

  static new_vox_setter char_func =
      fs::mri::new_vox_setter_chunked<unsigned char>;
  static new_vox_setter int_func   = fs::mri::new_vox_setter_chunked<int>;
  static new_vox_setter long_func  = fs::mri::new_vox_setter_chunked<long>;
  static new_vox_setter float_func = fs::mri::new_vox_setter_chunked<float>;
  static new_vox_setter short_func = fs::mri::new_vox_setter_chunked<short>;

  switch (mri->type) {
  case MRI_UCHAR:
    return char_func;
  case MRI_INT:
    return int_func;
  case MRI_LONG:
    return long_func;
  case MRI_FLOAT:
    return float_func;
  case MRI_SHORT:
    return short_func;
  }
}

} // namespace fs::mri

#endif
