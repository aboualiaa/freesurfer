//
// Created by Ahmed Abou-Aliaa on 22.02.20.
//

#include "immintrin.h"
#include "mri.h"

#include <limits>

namespace fs::mri {

constexpr auto uchar_min = std::numeric_limits<unsigned_char>::min();
constexpr auto uchar_max = std::numeric_limits<unsigned_char>::max();
constexpr auto short_min = std::numeric_limits<short>::min();
constexpr auto int_min   = std::numeric_limits<int>::min();
constexpr auto long_min  = std::numeric_limits<long>::min();
constexpr auto float_min = std::numeric_limits<float>::min();
constexpr auto short_max = std::numeric_limits<short>::max();
constexpr auto int_max   = std::numeric_limits<int>::max();
constexpr auto long_max  = std::numeric_limits<long>::max();
constexpr auto float_max = std::numeric_limits<float>::max();

using semi_new_vox_getter = float (&)(const MRI *, int, int, int, int);
using semi_new_vox_setter = int (&)(MRI *, int, int, int, int, float);
using new_vox_getter = float (&)(const MRI *, size_t);
using new_vox_setter = int (&)(MRI *, size_t, float);

template <typename T>
inline __attribute__((always_inline)) auto
semi_new_vox_getter_chunked(const MRI *mri, int column, int row, int slice,
                            int frame) -> float {

  return (float)*((T *)mri->chunk + column + row * mri->vox_per_row +
                  slice * mri->vox_per_slice + frame * mri->vox_per_vol);
}

template <typename T>
inline __attribute__((always_inline)) auto
new_vox_getter_chunked(const MRI *mri, size_t index) -> float {
  return (float)*((T *)mri->chunk + index);
}

inline auto get_typed_semi_new_vox_getter_chunked(MRI *mri) -> semi_new_vox_getter {

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
auto __attribute__((always_inline))
semi_new_vox_getter_non_chunked(const MRI *mri, int column, int row, int slice,
                                int frame) -> float {
  return ((float)(((T *)mri->slices[slice + (frame)*mri->depth][row])[column]));
}

template <typename T>
inline __attribute__((always_inline)) auto
semi_new_vox_setter_chunked(MRI *mri, int column, int row, int slice, int frame,
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
inline __attribute__((always_inline)) auto
new_vox_setter_chunked(MRI *mri, size_t index, float voxval) -> int {
  if (voxval < std::numeric_limits<T>::min()) {
    voxval = std::numeric_limits<T>::min();
  }
  if (voxval > std::numeric_limits<T>::max()) {
    voxval = std::numeric_limits<T>::max();
  }

  *((T *)mri->chunk + index) = nint(voxval);
  return (0);
}

template <typename T>
inline __attribute__((always_inline)) auto
semi_new_vox_setter_non_chunked(MRI *mri, int column, int row, int slice,
                                int frame, float voxval) -> int {
  // clipping
  switch (mri->type) {
  case MRI_UCHAR:
    if (voxval < uchar_min)
      voxval = uchar_min;
    if (voxval > uchar_max)
      voxval = uchar_max;
    break;
  case MRI_SHORT:
    if (voxval < short_min)
      voxval = short_min;
    if (voxval > short_max)
      voxval = short_max;
    break;
  case MRI_INT:
    if (voxval < int_min)
      voxval = int_min;
    if (voxval > int_max)
      voxval = int_max;
    break;
  case MRI_LONG:
    if (voxval < long_min)
      voxval = long_min;
    if (voxval > long_max)
      voxval = long_max;
    break;
  case MRI_FLOAT:
    if (voxval < float_min)
      voxval = float_min;
    if (voxval > float_max)
      voxval = float_max;
    break;
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

inline auto get_typed_semi_new_vox_setter_chunked(MRI *mri) -> semi_new_vox_setter {

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
