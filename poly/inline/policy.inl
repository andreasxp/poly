#pragma once
#include "policy.hpp"
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

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::compound(const compound<Cloner2, Deleter2>& other) :
	Cloner(static_cast<Cloner2&>(other)),
	Deleter(static_cast<Deleter2&>(other)) {
}

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::compound(compound<Cloner2, Deleter2>&& other) :
	Cloner(std::move(static_cast<Cloner2&>(other))),
	Deleter(std::move(static_cast<Deleter2&>(other))) {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	!std::is_constructible<Cloner, const T*>::value &&
	!std::is_constructible<Deleter, const T*>::value,
	detail::tag<0>*>::type>
inline compound<Cloner, Deleter>::compound(const T*) :
	Cloner(),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	!std::is_constructible<Cloner, const T*>::value &&
	 std::is_constructible<Deleter, const T*>::value,
	detail::tag<1>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	 std::is_constructible<Cloner, const T*>::value &&
	!std::is_constructible<Deleter, const T*>::value,
	detail::tag<2>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(ptr),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	std::is_constructible<Cloner, const T*>::value &&
	std::is_constructible<Deleter, const T*>::value,
	detail::tag<3>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(ptr),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T>
inline auto compound<Cloner, Deleter>::clone(const T* ptr)
->decltype(Cloner::operator()(ptr)) {
	return Cloner::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline void compound<Cloner, Deleter>::destroy(T* ptr) {
	Deleter::operator()(ptr);
}

// class deep_copy =============================================================

template<class Base>
constexpr deep_copy<Base>::deep_copy() noexcept :
	copy_construct (nullptr) {
}

template<class Base>
template<class Base2>
inline deep_copy<Base>::deep_copy(const deep_copy<Base2>& other) noexcept :
	copy_construct (other.copy_construct) {
}

template<class Base>
template<class Derived>
inline deep_copy<Base>::deep_copy(const Derived*) :
	copy_construct(&detail::clone<Base, Derived>) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"deep_copy: Base is not base of Derived");
}

template<class Base>
inline Base* deep_copy<Base>::operator()(const Base* other) {
	return copy_construct(other);
}

} // namespace pl