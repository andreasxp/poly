#pragma once
#include <atomic>      // atomic
#include <cstddef>     // ptrdiff_t
#include <type_traits> // is_base_of, remove_cv

/*!
\brief Macro to override typeid for rtti in poly
Define POLY_CUSTOM_RTTI(type) as a function that returns
your type's name. Do this **before** including poly.hpp
or factory.hpp.
\example #define POLY_CUSTOM_RTTI(...) my_typeid(__VA_ARGS__).name();
\example #define POLY_CUSTOM_RTTI(...) prid<__VA_ARGS__>().name();
\see     https://github.com/andreasxp/prindex
*/
#ifdef POLY_CUSTOM_TYPE_NAME
/// Get name of type
#define POLY_TYPE_NAME POLY_CUSTOM_TYPE_NAME
#else
/// Get name of type
#define POLY_TYPE_NAME(...) typeid(__VA_ARGS__).name()
#endif

namespace pl {
namespace detail {

/// A class that can downcast Base to Derived without dynamic_cast
template <class Base, class Derived>
class inheritance_traits_impl {
	static_assert(std::is_base_of<Base, Derived>::value,
		"inheritance_traits: Base is not base of Derived");
public:
	static void set_offset(const Base* base_ptr, const Derived* derived_ptr);

	static Derived* downcast(Base* ptr);
	static const Derived* downcast(const Base* ptr);

private:
	/// Holds a memory offset that converts Base* to Derived*
	static std::atomic<std::ptrdiff_t> offset;
};

// Inherits inheritance_traits_impl, so every base-derived
// pair has the same offset and members no matter how const
template <class Base, class Derived>
using inheritance_traits = inheritance_traits_impl<
	typename std::remove_cv<Base>::type,
	typename std::remove_cv<Derived>::type>;

} // namespace detail
} // namespace pl

#include "inheritance_traits.inl"