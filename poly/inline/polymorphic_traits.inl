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

template<class Base, class Derived>
inline typename std::enable_if<
	std::is_copy_constructible<Derived>::value, Base*>::type
	clone(const Base* other) {
	const Derived* temp = polymorphic_traits<Base, Derived>::downcast(other);

	Derived* rslt = new Derived(*temp);
	return static_cast<Base*>(rslt);
}

template<class Base, class Derived>
inline typename std::enable_if<
	!std::is_copy_constructible<Derived>::value, Base*>::type // Note the !
	clone(const Base*) {
	return throw std::runtime_error(
		std::string("poly: poly<") +
		POLY_TYPE_NAME(Base) +
		"> is attempting to copy '" +
		POLY_TYPE_NAME(Derived) +
		"', which is not copy-constructible");
}

} // namespace detail
} // namespace zhukov