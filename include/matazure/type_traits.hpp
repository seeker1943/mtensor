#pragma once

#include <matazure/bool_pack.hpp>
#include <matazure/config.hpp>
#include <matazure/integer_sequence.hpp>

namespace matazure {

struct linear_index {};
struct array_index {};

struct host_t {};
struct device_t {};
struct local_t {};

struct pinned {};
struct unpinned {};

struct aligned {};

/// define a generical compile time zero
template <typename _T>
struct zero;

/// @todo: define one
template <typename _T>
struct one;

/// special for most type, value() return 0 directly;
template <typename _Type>
struct zero {
    MATAZURE_GENERAL static constexpr _Type value() { return _Type{0}; };
};

// typedef float _f32x4_t __attribute__((vector_size(16)));
// template <>
// struct zero<_f32x4_t> {
//     static _f32x4_t value() {
//         _f32x4_t re = {0.0f, 0.0f, 0.0f, 0.0f};
//         return re;
//     }
// };

/// forward declare of tensor_expression
template <typename _Tensor>
class tensor_expression;

/// a type traits to get the argument type and result type of a functor
template <typename _Func>
struct function_traits : public function_traits<decltype(&_Func::operator())> {};

/// implements
template <typename _ClassType, typename _ReturnType, typename... _Args>
struct function_traits<_ReturnType (_ClassType::*)(_Args...) const> {
    enum { arguments_size = sizeof...(_Args) };

    typedef _ReturnType result_type;

    template <int_t _index>
    struct arguments {
        typedef typename std::tuple_element<_index, std::tuple<_Args...>>::type type;
    };
};

/// special for the tensor_expression models, they are tensor.
template <typename _Type>
struct is_tensor : bool_constant<std::is_base_of<tensor_expression<_Type>, _Type>::value> {};

template <typename _Type>
struct _Is_linear_array;

///
template <typename _Type>
struct is_linear_array : public _Is_linear_array<remove_cv_t<_Type>> {};

/// specail for the tensor liked type, they are linear array tensor.
template <typename _Type>
struct _Is_linear_array : bool_constant<is_tensor<_Type>::value> {};

// are tag
#define MATAZURE_DEFINE_ARE_TAG(name, tag_name, tag)                       \
    template <typename... _Tensor>                                         \
    struct name;                                                           \
    template <>                                                            \
                                                                           \
    struct name<> : bool_constant<true> {};                                \
                                                                           \
    template <typename _Tensor, typename... _OtherTensors>                 \
    struct name<_Tensor, _OtherTensors...>                                 \
        : bool_constant<is_same<typename _Tensor::tag_name, tag>::value && \
                        name<_OtherTensors...>::value> {};

MATAZURE_DEFINE_ARE_TAG(are_host_memory, runtime_type, host_t)
MATAZURE_DEFINE_ARE_TAG(are_device_memory, runtime_type, device_t)
MATAZURE_DEFINE_ARE_TAG(are_linear_index, index_type, linear_index)
MATAZURE_DEFINE_ARE_TAG(are_array_index, index_type, array_index)

// none tag
#define MATAZURE_DEFINE_NONE_TAG(name, tag_name, tag)                       \
    template <typename... _Tensor>                                          \
    struct name;                                                            \
    template <>                                                             \
                                                                            \
    struct name<> : bool_constant<true> {};                                 \
                                                                            \
    template <typename _Tensor, typename... _OtherTensors>                  \
    struct name<_Tensor, _OtherTensors...>                                  \
        : bool_constant<!is_same<typename _Tensor::tag_name, tag>::value && \
                        name<_OtherTensors...>::value> {};

MATAZURE_DEFINE_NONE_TAG(none_host_memory, runtime_type, host_t)
MATAZURE_DEFINE_NONE_TAG(none_device_memory, runtime_type, device_t)
MATAZURE_DEFINE_NONE_TAG(none_are_linear_index, index_type, linear_index)
MATAZURE_DEFINE_NONE_TAG(none_array_access, index_type, array_index)

}  // namespace matazure
