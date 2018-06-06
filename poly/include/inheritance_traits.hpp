#pragma once
#include <atomic>      // atomic
#include <cstddef>     // ptrdiff_t
#include <typeinfo>    // typeid
#include <string>      // string
#include <stdexcept>   // runtime_error
#include <type_traits> // is_base_of

/*!
\brief   Macro to override typeid for rtti in poly
\details Define POLY_CUSTOM_RTTI(type) as a function that returns
your type's name. Do this **before** including poly.hpp
or poly_factory.hpp.
\example #define POLY_CUSTOM_RTTI(...) my_typeid(__VA_ARGS__).name();
\example #define POLY_CUSTOM_RTTI(...) prid<__VA_ARGS__>().name();
\see     https://github.com/andreasxp/prindex
*/
#ifdef POLY_CUSTOM_TYPE_NAME
#define POLY_TYPE_NAME POLY_CUSTOM_TYPE_NAME
#else
/// Get name of type
#define POLY_TYPE_NAME(...) typeid(__VA_ARGS__).name()
#endif

namespace zhukov {
namespace detail {

template <class Base, class Derived>
struct inheritance_traits {
	static_assert(std::is_base_of<Base, Derived>::value,
		"inheritance_traits: Base is not base of Derived");

	// Static members ==========================================================
	static std::atomic<std::ptrdiff_t> offset;

	static Derived* downcast(Base* ptr);
	static const Derived* downcast(const Base* ptr);
};

} // namespace detail
} // namespace zhukov

#include "inheritance_traits.inl"