#pragma once
#include <atomic>      // atomic
#include <cstddef>     // ptrdiff_t
#include <typeinfo>    // typeid
#include <string>      // string
#include <stdexcept>   // runtime_error

/*!
\brief   Macro to override typeid for rtti in polymorphic_traits
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
struct polymorphic_traits {
	// Checks ==================================================================
	static_assert(std::is_polymorphic<Base>::value,
		"polymorphic_traits: Base is not polymorphic");

	static_assert(std::is_base_of<Base, Derived>::value,
		"polymorphic_traits: Base is not base of Derived");

	// Static members ==========================================================
	static std::atomic<std::ptrdiff_t> offset;

	static Derived* downcast(Base* ptr);
	static const Derived* downcast(const Base* ptr);
};

template<class Base, class Derived>
inline typename std::enable_if<
	std::is_copy_constructible<Derived>::value, Base*>::type
	clone(const Base* other);

template<class Base, class Derived>
inline typename std::enable_if<
	!std::is_copy_constructible<Derived>::value, Base*>::type // Note the !
	clone(const Base*);

} // namespace detail
} // namespace zhukov

#include "polymorphic_traits.inl"