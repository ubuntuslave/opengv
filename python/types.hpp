#ifndef __TYPES_H__
#define __TYPES_H__

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include <algorithm>
#include <vector>
#include <iostream>


namespace pyopengv {

namespace bp = boost::python;

namespace bpn = boost::python::numpy;
typedef bpn::ndarray ndarray;

template <typename T>
bp::object bpn_array_from_data(const T *data, int shape0) {
  bp::tuple shape = bp::make_tuple(shape0);
  bpn::dtype dtype =  bpn::dtype::get_builtin<T>();
  bpn::ndarray res = bpn::empty(shape, dtype);
  std::copy(data, data + shape0, reinterpret_cast<T*>(res.get_data()));
  return res;
}

template <typename T>
bp::object bpn_array_from_data(const T *data, int shape0, int shape1) {
  bp::tuple shape = bp::make_tuple(shape0, shape1);
  bpn::dtype dtype =  bpn::dtype::get_builtin<T>();
  bpn::ndarray res = bpn::empty(shape, dtype);
  std::copy(data, data + shape0 * shape1, reinterpret_cast<T*>(res.get_data()));
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
      : contiguous_(bpn::from_object(
            array,
            bpn::dtype::get_builtin<T>(),
            0,
            0,
            bpn::ndarray::C_CONTIGUOUS)) {}

  PyArrayContiguousView(const bp::object &object)
      : contiguous_(bpn::from_object(
            object,
            bpn::dtype::get_builtin<T>(),
            0,
            0,
            bpn::ndarray::C_CONTIGUOUS)) {}

  ~PyArrayContiguousView() {}

  const T *data() const {
    return reinterpret_cast<const T *>(contiguous_.get_data());
  }

  int ndim() const {
    return contiguous_.get_nd();
  }

  int shape(int dim) const {
    return contiguous_.shape(dim);
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
  ndarray contiguous_;
};

}

#endif // __TYPES_H__
