#pragma once
#include "poly.hpp"

namespace zhukov {
namespace detail {
// Black magic ahead

template <class Base, class Derived>
typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	std::is_copy_constructible<Derived>::value, 
	std::pair<void*, void*>>::type 
	clone(const void* derived_ptr) {
	Derived* ptr = new Derived(*static_cast<const Derived*>(derived_ptr));

	return std::make_pair(
		static_cast<void*>(static_cast<Base*>(ptr)),
		static_cast<void*>(ptr));
}

template <class Base, class Derived>
typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	!std::is_copy_constructible<Derived>::value, // Note the !
	std::pair<void*, void*>>::type
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
	derived_ptr(nullptr),
	base_ptr(nullptr),
	copy_construct(nullptr) {
	static_assert(std::is_polymorphic<Base>::value,
		"poly: poly can only be used with polymorphic types");
}

template<class Base>
constexpr poly<Base>::poly(std::nullptr_t) noexcept :
	poly() {
}

template<class Base>
constexpr poly<Base>::poly(const poly& other) :
	derived_ptr(nullptr),
	base_ptr(nullptr),
	copy_construct(other.copy_construct) {

	if (other) {
		auto copy_result = copy_construct(other.derived_ptr);

		base_ptr.reset(static_cast<Base*>(copy_result.first));
		derived_ptr = copy_result.second;
	}
}

template<class Base>
inline constexpr poly<Base>::poly(poly && other) noexcept :
	poly() {
	swap(*this, other);
}

template<class Base>
inline poly<Base> & poly<Base>::operator=(poly other) {
	swap(*this, other);
	return *this;
}

// From a pointer ----------------------------------------------------------
template<class Base>
template<class Derived, class>
constexpr poly<Base>::poly(Derived* obj) :
	derived_ptr(obj),
	base_ptr(static_cast<Base*>(obj)),
	copy_construct(&detail::clone<Base, Derived>) {
	static_assert(std::is_polymorphic<Base>::value,
		"poly: poly can only be used with polymorphic types");
}

// Observers ===============================================================
template<class Base>
template<class T>
constexpr bool poly<Base>::is() const noexcept {
	return base_ptr.get() != nullptr && typeid(*base_ptr.get()) == typeid(T);
}

template<class Base>
inline poly<Base>::operator bool() const noexcept {
	return static_cast<bool>(base_ptr);
}

// Modifiers ===============================================================
template<class Base>
inline Base* poly<Base>::release() noexcept {
	Base* rslt = base_ptr.release();
	reset();

	return rslt;
}

template<class Base>
inline void poly<Base>::reset() noexcept {
	base_ptr.reset();
	derived_ptr = nullptr;
	copy_construct = nullptr;
}

template<class Base>
template<class Derived, class>
inline void poly<Base>::reset(Derived* obj) {
	derived_ptr = obj;
	base_ptr.reset(obj);
	copy_construct = &detail::clone<Base, Derived>;
}

// Member access ===========================================================
template<class Base>
inline Base& poly<Base>::operator*() {
	return *base_ptr;
}

template<class Base>
constexpr Base& poly<Base>::operator*() const {
	return *base_ptr;
}

template<class Base>
inline Base* poly<Base>::operator->() {
	return base_ptr.get();
}

template<class Base>
constexpr Base* poly<Base>::operator->() const {
	return base_ptr.get();
}

template<class Base>
inline Base* poly<Base>::get() {
	return base_ptr.get();
}

template<class Base>
constexpr Base* poly<Base>::get() const {
	return base_ptr.get();
}

template<class Base>
template<class T>
typename std::enable_if<
	std::is_base_of<Base, T>::value, T&>::type
	poly<Base>::as() {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class Base>
template<class T>
constexpr typename std::enable_if<
	std::is_base_of<Base, T>::value, T&>::type
	poly<Base>::as() const {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class Base>
void swap(poly<Base>& lhs, poly<Base>& rhs) noexcept {
	using std::swap;

	swap(lhs.derived_ptr, rhs.derived_ptr);
	swap(lhs.base_ptr, rhs.base_ptr);
	swap(lhs.copy_construct, rhs.copy_construct);
}

} // namespace zhukov