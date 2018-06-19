#pragma once
#include "copy_policies.hpp"
#include "inheritance_traits.hpp"

namespace pl {
namespace detail {

template<class Base, class Derived>
inline typename std::enable_if<
	std::is_copy_constructible<Derived>::value, Base*>::type
	clone(const Base* other) {
	const Derived* temp = inheritance_traits<Base, Derived>::downcast(other);

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

// class deep_copy =============================================================

template<class Base>
constexpr deep_copy<Base>::deep_copy() noexcept :
	clone_ptr(nullptr) {
}

template<class Base>
template<class Base2>
inline deep_copy<Base>::deep_copy(const deep_copy<Base2>& other) noexcept :
	clone_ptr(other.clone_ptr) {
}

template<class Base>
template<class Derived>
inline deep_copy<Base>::deep_copy(const Derived*) :
	clone_ptr(&detail::clone<Base, Derived>) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"deep_copy: Base is not base of Derived");
}

template<class Base>
inline Base* deep_copy<Base>::operator()(const Base* other) {
	return clone_ptr(other);
}

} // namespace pl