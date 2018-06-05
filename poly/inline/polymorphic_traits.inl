#pragma once
#include "polymorphic_traits.hpp"

namespace zhukov {
namespace detail {

template <class Base, class Derived>
std::atomic<std::ptrdiff_t> polymorphic_traits<Base, Derived>::offset;

template<class Base, class Derived>
inline Derived* polymorphic_traits<Base, Derived>::downcast(Base* ptr) {
	return reinterpret_cast<Derived*>(
		reinterpret_cast<unsigned char*>(ptr) + offset);
}

template<class Base, class Derived>
inline const Derived* polymorphic_traits<Base, Derived>::downcast(const Base* ptr) {
	return reinterpret_cast<const Derived*>(
		reinterpret_cast<const unsigned char*>(ptr) + offset);
}

} // namespace detail
} // namespace zhukov