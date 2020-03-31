#pragma once

#include <cuda_runtime.h>
#include <matazure/cuda/algorithm.hpp>
#include <matazure/cuda/runtime.hpp>
#include <matazure/tensor.hpp>

#define MATAZURE_IS_D_LAMBDA(X) __nv_is_extended_device_lambda_closure_type(X)
#define MATAZURE_IS_HD_LAMBDA(X) __nv_is_extended_host_device_lambda_closure_type(X)

namespace matazure {
namespace cuda {

#pragma hd_warning_disable
template <typename _Type, int_t _Rank, typename _Layout = column_major_layout<_Rank>>
class tensor : public tensor_expression<tensor<_Type, _Rank, _Layout>> {
   public:
    static_assert(std::is_pod<_Type>::value, "only supports pod type now");

    static const int_t rank = _Rank;
    typedef _Type value_type;
    typedef value_type& reference;
    typedef value_type* pointer;
    typedef linear_index index_type;
    typedef _Layout layout_type;
    typedef device_tag memory_type;

    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL tensor() : tensor(pointi<rank>::zeros()) {}

    MATAZURE_HD_WARNING_DISABLE
    template <typename... _Ext, typename _Tmp = enable_if_t<sizeof...(_Ext) == rank>>
    MATAZURE_GENERAL explicit tensor(_Ext... ext) : tensor(pointi<rank>{ext...}) {}

    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL explicit tensor(pointi<rank> ext)
        : shape_(ext),
          layout_(ext),
          sp_data_(malloc_shared_memory(layout_.stride()[rank - 1])),
          data_(sp_data_.get()) {}

    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL
    explicit tensor(pointi<rank> ext, std::shared_ptr<value_type> sp_data)
        : shape_(ext), layout_(ext), sp_data_(sp_data), data_(sp_data_.get()) {}

    /**
     *
     *
     */
    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL
    explicit tensor(const pointi<rank>& shape, const pointi<rank>& origin_padding,
                    const pointi<rank>& end_padding)
        : shape_(shape),
          layout_(shape, origin_padding, end_padding),
          sp_data_(malloc_shared_memory(layout_.stride()[rank - 1])),
          data_(sp_data_.get()) {}

    MATAZURE_HD_WARNING_DISABLE
    template <typename _VT>
    MATAZURE_GENERAL tensor(const tensor<_VT, _Rank, _Layout>& ts)
        : shape_(ts.shape()), layout_(ts.layout_), sp_data_(ts.shared_data()), data_(ts.data()) {}

    tensor(std::initializer_list<int_t> v) = delete;

    MATAZURE_GENERAL
    shared_ptr<value_type> shared_data() const { return sp_data_; }

    //  template <typename _Idx>
    //  MATAZURE_GENERAL reference operator()(_Idx idx) const {
    //  	static_assert(std::is_same<_Idx, int_t>::value && rank == 1, "only operator []
    //  support access data by pointi"); 	return (*this)[pointi<1>{idx}];
    //  }

    MATAZURE_GENERAL reference operator[](const pointi<rank>& index) const {
        return (*this)[layout_.index2offset(index)];
    }

    template <typename... _Idx>
    MATAZURE_GENERAL reference operator()(_Idx... idx) const {
        return (*this)[pointi<rank>{idx...}];
    }

    MATAZURE_GENERAL reference operator[](int_t i) const { return data_[i]; }

    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL pointi<rank> shape() const { return shape_; }

    MATAZURE_GENERAL pointi<rank> stride() const { return layout_.stride(); }
    MATAZURE_GENERAL int_t size() const { return layout_.stride()[rank - 1]; }

    MATAZURE_GENERAL pointer data() const { return data_; }

    MATAZURE_HD_WARNING_DISABLE
    MATAZURE_GENERAL ~tensor() {}

   private:
    shared_ptr<value_type> malloc_shared_memory(int_t size) {
        decay_t<value_type>* data = nullptr;
        size = size > 0 ? size : 1;
        assert_runtime_success(cudaMalloc(&data, size * sizeof(value_type)));
        return shared_ptr<value_type>(data, [](value_type* ptr) {
            assert_runtime_success(cudaFree(const_cast<decay_t<value_type>*>(ptr)));
        });
    }

   private:
    const pointi<rank> shape_;
    const layout_type layout_;
#pragma hd_warning_disable
    const shared_ptr<value_type> sp_data_;
    const pointer data_;
};

template <typename _TensorSrc, typename _TensorDst>
inline void mem_copy(_TensorSrc ts_src, _TensorDst cts_dst,
                     enable_if_t<!are_host_memory<_TensorSrc, _TensorDst>::value &&
                                 is_same<typename _TensorSrc::layout_type,
                                         typename _TensorDst::layout_type>::value>* = nullptr) {
    MATAZURE_STATIC_ASSERT_VALUE_TYPE_MATCHED(_TensorSrc, _TensorDst);

    assert_runtime_success(cudaMemcpy(cts_dst.data(), ts_src.data(),
                                      sizeof(typename _TensorDst::value_type) * ts_src.size(),
                                      cudaMemcpyDefault));
}

template <typename _TensorSrc, typename _TensorSymbol>
inline void copy_symbol(_TensorSrc src, _TensorSymbol& symbol_dst) {
    assert_runtime_success(cudaMemcpyToSymbol(
        symbol_dst, src.data(), src.size() * sizeof(typename _TensorSrc::value_type)));
}

inline void device_synchronize() { assert_runtime_success(cudaDeviceSynchronize()); }

template <typename _ValueType, int_t _Rank>
inline void memset(tensor<_ValueType, _Rank> ts, int v) {
    assert_runtime_success(cudaMemset(ts.shared_data().get(), v, ts.size() * sizeof(_ValueType)));
}

inline void MATAZURE_DEVICE sync_threads() { __syncthreads(); }

template <typename _Type, int_t _Rank, typename _Layout>
inline tensor<_Type, _Rank, _Layout> mem_clone(tensor<_Type, _Rank, _Layout> ts, device_tag) {
    tensor<decay_t<_Type>, _Rank, _Layout> ts_re(ts.shape());
    mem_copy(ts, ts_re);
    return ts_re;
}

template <typename _Type, int_t _Rank, typename _Layout>
inline tensor<_Type, _Rank, _Layout> mem_clone(tensor<_Type, _Rank, _Layout> ts) {
    return mem_clone(ts, device_tag{});
}

template <typename _Type, int_t _Rank, typename _Layout>
inline tensor<_Type, _Rank, _Layout> mem_clone(matazure::tensor<_Type, _Rank, _Layout> ts,
                                               device_tag) {
    tensor<decay_t<_Type>, _Rank, _Layout> ts_re(ts.shape());
    mem_copy(ts, ts_re);
    return ts_re;
}

template <typename _Type, int_t _Rank, typename _Layout>
inline matazure::tensor<_Type, _Rank, _Layout> mem_clone(tensor<_Type, _Rank, _Layout> ts,
                                                         host_tag) {
    matazure::tensor<decay_t<_Type>, _Rank, _Layout> ts_re(ts.shape());
    mem_copy(ts, ts_re);
    return ts_re;
}

template <typename _ValueType, int_t _Rank, typename _Layout, int_t _OutDim,
          typename _OutLayout = _Layout>
inline auto reshape(cuda::tensor<_ValueType, _Rank, _Layout> ts, pointi<_OutDim> ext,
                    _OutLayout* = nullptr) -> cuda::tensor<_ValueType, _OutDim, _OutLayout> {
    /// TODO: assert size equal
    cuda::tensor<_ValueType, _OutDim, _OutLayout> re(ext, ts.shared_data());
    return re;
}

#ifndef MATAZURE_DISABLE_MATRIX_VECTOR_ALIAS
template <typename _ValueType, typename _Layout = column_major_layout<1>>
using vector = tensor<_ValueType, 1, _Layout>;
template <typename _ValueType, typename _Layout = column_major_layout<2>>
using matrix = tensor<_ValueType, 2, _Layout>;
template <typename _ValueType, typename _Layout = column_major_layout<3>>
using volume = tensor<_ValueType, 3, _Layout>;
#endif

template <int_t _Rank, typename _Layout = column_major_layout<_Rank>>
using tensorb = tensor<byte, _Rank, column_major_layout<_Rank>>;
template <int_t _Rank, typename _Layout = column_major_layout<_Rank>>
using tensors = tensor<short, _Rank, column_major_layout<_Rank>>;
template <int_t _Rank, typename _Layout = column_major_layout<_Rank>>
using tensori = tensor<int_t, _Rank, column_major_layout<_Rank>>;
template <int_t _Rank, typename _Layout = column_major_layout<_Rank>>
using tensorf = tensor<float, _Rank, column_major_layout<_Rank>>;
template <int_t _Rank, typename _Layout = column_major_layout<_Rank>>
using tensord = tensor<double, _Rank, column_major_layout<_Rank>>;

namespace __walkaround {

using tensor1b = tensor<byte, 1>;
using tensor2b = tensor<byte, 2>;
using tensor3b = tensor<byte, 3>;
using tensor4b = tensor<byte, 4>;

using tensor1s = tensor<short, 1>;
using tensor2s = tensor<short, 2>;
using tensor3s = tensor<short, 3>;
using tensor4s = tensor<short, 4>;

using tensor1us = tensor<unsigned short, 1>;
using tensor2us = tensor<unsigned short, 2>;
using tensor3us = tensor<unsigned short, 4>;
using tensor4us = tensor<unsigned short, 4>;

using tensor1i = tensor<int, 1>;
using tensor2i = tensor<int, 2>;
using tensor3i = tensor<int, 3>;
using tensor4i = tensor<int, 4>;

using tensor1ui = tensor<unsigned int, 1>;
using tensor2ui = tensor<unsigned int, 2>;
using tensor3ui = tensor<unsigned int, 3>;
using tensor4ui = tensor<unsigned int, 4>;

using tensor1l = tensor<long, 1>;
using tensor2l = tensor<long, 2>;
using tensor3l = tensor<long, 3>;
using tensor4l = tensor<long, 4>;

using tensor1ul = tensor<unsigned long, 1>;
using tensor2ul = tensor<unsigned long, 2>;
using tensor3ul = tensor<unsigned long, 3>;
using tensor4ul = tensor<unsigned long, 4>;

using tensor1f = tensor<float, 1>;
using tensor2f = tensor<float, 2>;
using tensor3f = tensor<float, 3>;
using tensor4f = tensor<float, 4>;

using tensor1d = tensor<double, 1>;
using tensor2d = tensor<double, 1>;
using tensor3d = tensor<double, 1>;
using tensor4d = tensor<double, 1>;

}  // namespace __walkaround

}  // cuda

using cuda::mem_clone;
using cuda::mem_copy;
using cuda::reshape;

}  // namespace matazure