#pragma once
#include "poly.hpp"

namespace zhukov {
namespace detail {
// Black magic ahead

template <class Base, class Derived>
typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	std::is_copy_constructible<Derived>::value, 
	Base*>::type 
	clone(const void* derived_ptr) {
	Derived* ptr = new Derived(*static_cast<const Derived*>(derived_ptr));

	return static_cast<Base*>(ptr);
}

template <class Base, class Derived>
typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	!std::is_copy_constructible<Derived>::value, // Note the !
	Base*>::type
	clone(const void* /*other (unused)*/) {
	return throw std::runtime_error(
		std::string("poly: poly<") +
		POLY_TYPE_NAME(Base) +
		"> is attempting to copy '" +
		POLY_TYPE_NAME(Derived) +
		"', which is not copy-constructible");
}

} // namespace detail

// =============================================================================
// Definitions =================================================================

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base>
constexpr poly<Base>::poly() noexcept :
	data(nullptr),
	copy_construct(nullptr) {
}

template<class Base>
constexpr poly<Base>::poly(const poly& other) :
	data(nullptr),
	copy_construct(other.copy_construct) {

	if (other) {
		data.reset(copy_construct(other.data.get()));
	}
}

template<class Base>
inline poly<Base>& poly<Base>::operator=(const poly& other) {
	copy_construct = other.copy_construct;

	if (other) {
		data.reset(copy_construct(other.data.get()));
	}
	
	return *this;
}

// From an object ----------------------------------------------------------

template<class Base>
template<class Derived, class>
constexpr poly<Base>::poly(const Derived& obj) :
	data(new Derived(obj)),
	copy_construct(&detail::clone<Base, Derived>) {
}

template<class Base>
template<class Derived, class>
constexpr poly<Base>::poly(Derived&& obj) :
	data(new Derived(std::move(obj))),
	copy_construct(&detail::clone<Base, Derived>) {
}

// Observers ===============================================================
template<class Base>
template<class T>
constexpr bool poly<Base>::is() const noexcept {
	return data.get() != nullptr && typeid(*data.get()) == typeid(T);
}

template<class Base>
inline poly<Base>::operator bool() const noexcept {
	return static_cast<bool>(data);
}

// Member access ===========================================================
template<class Base>
inline poly<Base>::operator Base&() {
	return *data;
}

template<class Base>
inline poly<Base>::operator const Base&() const {
	return *data;
}

template<class Base>
inline Base& poly<Base>::get() {
	return *data;
}

template<class Base>
inline const Base& poly<Base>::get() const {
	return *data;
}

template<class Base>
template<class T>
T& poly<Base>::as() {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return *data.as<T>();
	}
	throw std::bad_cast();
}

template<class Base>
template<class T>
const T& poly<Base>::as() const {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return *data.as<T>();
	}
	throw std::bad_cast();
}

} // namespace zhukov