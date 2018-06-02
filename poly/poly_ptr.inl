#pragma once
#include "poly_ptr.hpp"
#include "polymorphic_traits.hpp"

namespace zhukov {

// =============================================================================
// Definitions =================================================================

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base>
constexpr poly_ptr<Base>::poly_ptr() noexcept :
	base_ptr(nullptr) {
}

template<class Base>
constexpr poly_ptr<Base>::poly_ptr(std::nullptr_t) noexcept :
	poly_ptr() {
}

template<class Base>
constexpr poly_ptr<Base>::poly_ptr(poly_ptr&& other) noexcept :
	poly_ptr() {
	swap(other);
}

template<class Base>
constexpr poly_ptr<Base>& poly_ptr<Base>::operator=(poly_ptr&& other) noexcept {
	swap(other);
	return *this;
}

template<class Base>
constexpr poly_ptr<Base>& poly_ptr<Base>::operator=(std::nullptr_t) noexcept {
	reset();
	return *this;
}

// From a pointer ----------------------------------------------------------
template<class Base>
template<class Derived>
constexpr poly_ptr<Base>::poly_ptr(Derived* obj) :
	base_ptr(static_cast<Base*>(obj)) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly_ptr: poly_ptr can only be built using types, derived from Base");

	detail::polymorphic_traits<Base, Derived>::offset = 
		reinterpret_cast<unsigned char*>(obj) - 
		reinterpret_cast<unsigned char*>(base_ptr.get());
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
	base_ptr.reset();
}

template<class Base>
template<class Derived>
inline void poly_ptr<Base>::reset(Derived* obj) noexcept {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly_ptr: poly_ptr can only be built using types, derived from Base");

	base_ptr.reset(obj);

	detail::polymorphic_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(base_ptr.get());
}

template<class Base>
inline void poly_ptr<Base>::swap(poly_ptr& other) noexcept {
	using std::swap;

	swap(base_ptr, other.base_ptr);
}

// Member access ===========================================================
template<class Base>
inline Base& poly_ptr<Base>::operator*() const {
	return *base_ptr;
}

template<class Base>
inline typename poly_ptr<Base>::pointer
poly_ptr<Base>::operator->() const noexcept {
	return base_ptr.get();
}

template<class Base>
inline typename poly_ptr<Base>::pointer
poly_ptr<Base>::get() const noexcept {
	return base_ptr.get();
}

template<class Base>
template<class T>
T* poly_ptr<Base>::as() const noexcept {
	static_assert(std::is_base_of<Base, T>::value,
		"poly_ptr: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return detail::polymorphic_traits<Base, T>::downcast(base_ptr.get());
	}
	return nullptr;
}

template<class Base>
void swap(poly_ptr<Base>& lhs, poly_ptr<Base>& rhs) noexcept {
	lhs.swap(rhs);
}

} // namespace zhukov