#pragma once
#include "copy_policy.hpp"
#include "polymorphic_traits.hpp"

namespace zhukov {
namespace detail {

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

template<class Derived>
inline no_copy::no_copy(const Derived*) {
}

template<class Base>
inline deep_copy<Base>::deep_copy() {
}

template<class Base>
template<class Derived>
inline deep_copy<Base>::deep_copy(const Derived*) :
	copy_construct(&detail::clone<Base, Derived>) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"deep_copy: Base is not base of Derived");
}

template<class Base>
inline Base* deep_copy<Base>::copy(const Base* other) {
	return copy_construct(other);
}

} // namespace zhukov