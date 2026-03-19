#ifndef __TYPES_H__
#define __TYPES_H__

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <algorithm>
#include <vector>
#include <iostream>


namespace pyopengv {

namespace bp = pybind11;

namespace bpn = pybind11;
typedef bpn::array ndarray;

template <typename T>
bp::object bpn_array_from_data(const T *data, int shape0) {
  auto res = bpn::array_t<T>(shape0);
  auto ptr = static_cast<T *>(res.mutable_data());
  std::copy(data, data + shape0, ptr);
  return res;
}

template <typename T>
bp::object bpn_array_from_data(const T *data, int shape0, int shape1) {
  auto res = bpn::array_t<T>({shape0, shape1});
  auto ptr = static_cast<T *>(res.mutable_data());
  std::copy(data, data + shape0 * shape1, ptr);
  return res;
}

template <typename T>
bp::object bpn_array_from_vector(const std::vector<T> &v) {
  const T *data = v.size() ? &v[0] : NULL;
  return bpn_array_from_data(data, v.size());
}

template<typename T>
class PyArrayContiguousView {
 public:
  PyArrayContiguousView(ndarray &array)
    : contiguous_(to_contiguous(array))
      , buffer_(contiguous_.request()) {}

  PyArrayContiguousView(const bp::object &object)
    : contiguous_(to_contiguous(object.cast<ndarray>()))
      , buffer_(contiguous_.request()) {}

  ~PyArrayContiguousView() {}

  const T *data() const {
    return static_cast<const T *>(buffer_.ptr);
  }

  int ndim() const {
    return buffer_.ndim;
  }

  int shape(int dim) const {
    return static_cast<int>(buffer_.shape[dim]);
  }

  bool valid() const {
    return true;
  }

  T get(size_t i) const {
    return data()[i];
  }

  T get(size_t i, size_t j) const {
    return data()[i * shape(1) + j];
  }

  T get2D(size_t i, size_t k) const {
    return data()[i * shape(1) * shape(2) + k];
  }

 private:
  static bpn::array_t<T, bpn::array::c_style | bpn::array::forcecast>
  to_contiguous(const ndarray &array) {
    auto view = bpn::array_t<T, bpn::array::c_style | bpn::array::forcecast>::ensure(array);
    if (!view) {
      throw bpn::type_error("Unable to convert array to required contiguous dtype");
    }
    return view;
  }

  bpn::array_t<T, bpn::array::c_style | bpn::array::forcecast> contiguous_;
  bpn::buffer_info buffer_;
};

}

#endif // __TYPES_H__
