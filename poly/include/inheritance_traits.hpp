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

namespace pl {
namespace detail {

template <class Base, class Derived>
class inheritance_traits_impl {
public:
	static void set_offset(const Base* base_ptr, const Derived* derived_ptr);

	static Derived* downcast(Base* ptr);
	static const Derived* downcast(const Base* ptr);

private:
	static std::atomic<std::ptrdiff_t> offset;
};

// Inherits inheritance_traits_impl, so every base-derived
// pair has the same offset and members no matter how const
template <class Base, class Derived>
class inheritance_traits : public inheritance_traits_impl<
	typename std::decay<Base>::type,
	typename std::decay<Derived>::type> {
	static_assert(std::is_base_of<Base, Derived>::value,
		"inheritance_traits: Base is not base of Derived");
};

} // namespace detail
} // namespace pl

#include "inheritance_traits.inl"