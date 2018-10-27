#pragma once
#include <type_traits>

namespace pl {
namespace detail {

// is_less_cv and is_more_cv - compare constraints of two types
// const int is more cv than int
// const int is NOT more cv than volatile int
// "more" in this context means that, assuming 
// remove_cv_t<T> is same as remove_cv_t<U>,
// U can be upgraded to T without losing qualifiers.

template <class T, class U>
struct is_less_cv
	: std::integral_constant<bool,
	(std::is_const<T>::value <= std::is_const<U>::value &&
		std::is_volatile<T>::value <= std::is_volatile<U>::value)> {
};

template <class T, class U>
struct is_more_cv
	: std::integral_constant<bool,
	(std::is_const<T>::value >= std::is_const<U>::value &&
	std::is_volatile<T>::value >= std::is_volatile<U>::value)> {
};

// is_stronger_qualified - checks that a type T is a more strongly qualified
// version of U

template <class T, class U>
struct is_stronger_qualified
	: std::integral_constant<bool,
	(std::is_same<
		typename std::remove_cv<T>::type, 
		typename std::remove_cv<U>::type>::value &&
	is_more_cv<T, U>::value)> {
};

template <class T, class U>
struct is_weaker_qualified
	: std::integral_constant<bool,
	(std::is_same<
		typename std::remove_cv<T>::type,
		typename std::remove_cv<U>::type>::value &&
		is_less_cv<T, U>::value)> {
};

template<class T, class R = void>
struct enable_if_defined {
	using type = R;
};

template<class T, class Enable = void>
struct defines_type : std::false_type {
};

template<class T>
struct defines_type<
	T, typename enable_if_defined<typename T::type>::type> : std::true_type {
};

} // namespace detail
} // namespace pl