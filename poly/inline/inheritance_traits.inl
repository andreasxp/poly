#pragma once
#include "inheritance_traits.hpp"

namespace zhukov {
namespace detail {

template <class Base, class Derived>
std::atomic<std::ptrdiff_t> inheritance_traits_impl<Base, Derived>::offset;

template<class Base, class Derived>
inline void inheritance_traits_impl<Base, Derived>::
set_offset(const Base* base_ptr, const Derived* derived_ptr) {
	offset = 
		reinterpret_cast<const unsigned char*>(derived_ptr) -
		reinterpret_cast<const unsigned char*>(base_ptr);
}

template<class Base, class Derived>
inline Derived* inheritance_traits_impl<Base, Derived>::downcast(Base* ptr) {
	return reinterpret_cast<Derived*>(
		reinterpret_cast<unsigned char*>(ptr) + offset);
}

template<class Base, class Derived>
inline const Derived* inheritance_traits_impl<Base, Derived>::downcast(const Base* ptr) {
	return reinterpret_cast<const Derived*>(
		reinterpret_cast<const unsigned char*>(ptr) + offset);
}

} // namespace detail
} // namespace zhukov