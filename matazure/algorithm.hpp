﻿#pragma once

#include <matazure/point.hpp>

namespace matazure {

	template <typename _Func>
	inline void for_index(pointi<1> origin, pointi<1> extent, _Func fun) {
		for (int_t i = origin[0]; i < extent[0]; ++i) {
			fun(pointi<1>{ { i } });
		}
	}

	template <typename _Func>
	inline void for_index(pointi<2> origin, pointi<2> extent, _Func fun) {
		for (int_t j = origin[1]; j < extent[1]; ++j) {
			for (int_t i = origin[0]; i < extent[0]; ++i) {
				fun(pointi<2>{ { i, j } });
			}
		}
	}

	template <typename _Func>
	inline void for_index(pointi<3> origin, pointi<3> extent, _Func fun) {
		for (int_t k = origin[2]; k < extent[2]; ++k) {
			for (int_t j = origin[1]; j < extent[1]; ++j) {
				for (int_t i = origin[0]; i < extent[0]; ++i) {
					fun(pointi<3>{ { i, j, k } });
				}
			}
		}
	}

	template <typename _Func>
	inline void for_index(pointi<4> origin, pointi<4> extent, _Func fun) {
		for (int_t l = origin[3]; l < extent[3]; ++l) {
			for (int_t k = origin[2]; k < extent[2]; ++k) {
				for (int_t j = origin[1]; j < extent[1]; ++j) {
					for (int_t i = origin[0]; i < extent[0]; ++i) {
						fun(pointi<4>{ {i, j, k, l} });
					}
				}
			}
		}
	}

	template <typename _Tensor, typename _Fun>
	inline void for_each(_Tensor ts, _Fun fun, enable_if_t<are_linear_access<_Tensor>::value && none_device_memory<_Tensor>::value>* =0) {
		for (int_t i = 0; i < ts.size(); ++i) {
			fun(ts[i]);
		}
	}

	template <typename _Tensor, typename _Fun>
	inline void for_each(_Tensor ts, _Fun fun, enable_if_t<!are_linear_access<_Tensor>::value && none_device_memory<_Tensor>::value>* = 0) {
		for_index(pointi<_Tensor::dim>::zeor(), ts.extent(), [=](pointi<_Tensor::dim> idx) {
			fun(ts(idx));
		});
	}

	template <typename _Tensor>
	inline void fill(_Tensor ts, typename _Tensor::value_type v, enable_if_t<none_device_memory<_Tensor>::value>* = 0) {
		for_each(ts, [v](typename _Tensor::value_type &x) { x = v;});
	}

	template <typename _T1, typename _T2>
	inline void copy(_T1 lhs, _T2 rhs, enable_if_t<are_linear_access<_T1, _T2>::value && none_device_memory<_T1, _T2>::value>* = 0) {
		for (int_t i = 0, size = rhs.size(); i < size; ++i) {
			rhs[i] = lhs[i];
		}
	}

	template <typename _T1, typename _T2>
	inline void copy(_T1 lhs, _T2 rhs, enable_if_t<!are_linear_access<_T1, _T2>::value && none_device_memory<_T1, _T2>::value>* = 0) {
		for_index(pointi<_T1::dim>::zeros(), lhs.extent(), [=] (pointi<_T1::dim> idx) {
			rhs(idx) = lhs(idx);
		});
	}

	template <typename _T1, typename _T2, typename _TransFun>
	inline void transform(_T1 lhs, _T2 rhs, _TransFun fun, enable_if_t<are_linear_access<_T1, _T2>::value && none_device_memory<_T1, _T2>::value>* = 0) {
		for (int_t i = 0, size = rhs.size(); i < size; ++i) {
			fun(rhs[i], lhs[i]);
		}
	}

	template <typename _T1, typename _T2, typename _TransFun>
	inline void transform(_T1 lhs, _T2 rhs, enable_if_t<!are_linear_access<_T1, _T2>::value && none_device_memory<_T1, _T2>::value>* = 0) {
		for_index(pointi<_T1::dim>::zeros(), lhs.extent(), [=] (pointi<_T1::dim> idx) {
			fun(rhs(idx) = lhs(idx));
		});
	}

	template <typename _Tensor, typename _VT, typename _BinaryOp>
	inline _VT reduce(_Tensor ts, _VT init, _BinaryOp binaryop, enable_if_t<none_device_memory<_Tensor>::value>* = 0) {
		auto re = init;
		for_each(ts, [&re, binaryop](_VT x) {
			re = binaryop(re, x);
		});

		return re;
	}

	template <typename _TS>
	inline typename _TS::value_type sum(_TS ts) {
		typedef typename _TS::value_type value_type;
		return reduce(ts, value_type(0), [=](value_type lhs, value_type rhs) {
			return lhs + rhs;
		});
	}

	template <typename _TS>
	inline typename _TS::value_type prod(_TS ts) {
		typedef typename _TS::value_type value_type;
		return reduce(ts, value_type(1), [=](value_type lhs, value_type rhs) {
			return lhs * rhs;
		});
	}

	template <typename _TS>
	inline typename _TS::value_type mean(_TS ts) {
		return sum(ts) / ts.size();
	}

	template <typename _TS>
	inline typename _TS::value_type max(_TS ts) {
		typedef typename _TS::value_type value_type;
		return reduce(ts, ts[0], [=](value_type lhs, value_type rhs) {
			return rhs > lhs ? rhs : lhs;
		});
	}

	template <typename _TS>
	inline typename _TS::value_type min(_TS ts) {
		typedef typename _TS::value_type value_type;
		return reduce(ts, numeric_limits<value_type>::max(), [=](value_type lhs, value_type rhs) {
			return lhs <= rhs ? lhs : rhs;
		});
	}

}
