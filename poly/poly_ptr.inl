#pragma once
#include "poly_ptr.hpp"

namespace zhukov {

// =============================================================================
// Definitions =================================================================

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base>
constexpr poly_ptr<Base>::poly_ptr() noexcept :
	derived_ptr(nullptr),
	base_ptr(nullptr) {
	static_assert(std::is_polymorphic<Base>::value,
		"poly_ptr: poly_ptr can only be used with polymorphic types");
}

template<class Base>
constexpr poly_ptr<Base>::poly_ptr(std::nullptr_t) noexcept :
	poly_ptr() {
}

template<class Base>
constexpr poly_ptr<Base>::poly_ptr(poly_ptr&& other) noexcept :
	poly_ptr() {
	swap(*this, other);
}

template<class Base>
constexpr poly_ptr<Base> & poly_ptr<Base>::operator=(poly_ptr&& other) {
	swap(*this, other);
	return *this;
}

template<class Base>
constexpr poly_ptr<Base> & poly_ptr<Base>::operator=(std::nullptr_t) {
	reset();
	return *this;
}

// From a pointer ----------------------------------------------------------
template<class Base>
template<class Derived, class>
constexpr poly_ptr<Base>::poly_ptr(Derived* obj) :
	derived_ptr(obj),
	base_ptr(static_cast<Base*>(obj)) {
	static_assert(std::is_polymorphic<Base>::value,
		"poly_ptr: poly_ptr can only be used with polymorphic types");
}

// Observers ===============================================================
template<class Base>
template<class T>
constexpr bool poly_ptr<Base>::is() const noexcept {
	return base_ptr.get() != nullptr && typeid(*base_ptr.get()) == typeid(T);
}

template<class Base>
constexpr poly_ptr<Base>::operator bool() const noexcept {
	return static_cast<bool>(base_ptr);
}

// Modifiers ===============================================================
template<class Base>
inline typename poly_ptr<Base>::pointer
poly_ptr<Base>::release() noexcept {
	Base* rslt = base_ptr.release();
	reset();

	return rslt;
}

template<class Base>
inline void poly_ptr<Base>::reset(std::nullptr_t) noexcept {
	reset();
}

template<class Base>
inline void poly_ptr<Base>::reset() noexcept {
	base_ptr.reset();
	derived_ptr = nullptr;
	copy_construct = nullptr;
}

template<class Base>
template<class Derived, class>
inline void poly_ptr<Base>::reset(Derived* obj) {
	derived_ptr = obj;
	base_ptr.reset(obj);
}

template<class Base>
inline void poly_ptr<Base>::swap(poly_ptr& other) noexcept {
	using std::swap;

	swap(derived_ptr, other.derived_ptr);
	swap(base_ptr, other.base_ptr);
}

// Member access ===========================================================
template<class Base>
inline Base& poly_ptr<Base>::operator*() const {
	return *base_ptr;
}

template<class Base>
inline typename poly_ptr<Base>::pointer
poly_ptr<Base>::operator->() const {
	return base_ptr.get();
}

template<class Base>
inline typename poly_ptr<Base>::pointer
poly_ptr<Base>::get() const {
	return base_ptr.get();
}

template<class Base>
template<class T>
typename std::enable_if<
	std::is_base_of<Base, T>::value, T*>::type
	poly_ptr<Base>::as() {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class Base>
template<class T>
constexpr typename std::enable_if<
	std::is_base_of<Base, T>::value, T&>::type
	poly_ptr<Base>::as() const {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class Base>
void swap(poly_ptr<Base>& lhs, poly_ptr<Base>& rhs) noexcept {
	lhs.swap(rhs);
}

} // namespace zhukov