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

template<class base_t>
constexpr poly<base_t>::poly() noexcept :
	derived_ptr(nullptr),
	base_ptr(nullptr),
	copy_construct(nullptr) {
	static_assert(std::is_polymorphic<base_t>::value,
		"poly: poly can only be used with polymorphic types");
}

template<class base_t>
constexpr poly<base_t>::poly(std::nullptr_t) noexcept :
	poly() {
}

template<class base_t>
constexpr poly<base_t>::poly(const poly& other) :
	derived_ptr(nullptr),
	base_ptr(nullptr),
	copy_construct(other.copy_construct) {

	if (other) {
		auto copy_result = copy_construct(other.derived_ptr);

		base_ptr.reset(static_cast<base_t*>(copy_result.first));
		derived_ptr = copy_result.second;
	}
}

template<class base_t>
inline constexpr poly<base_t>::poly(poly && other) noexcept :
	poly() {
	swap(*this, other);
}

template<class base_t>
inline poly<base_t> & poly<base_t>::operator=(poly other) {
	swap(*this, other);
	return *this;
}

// From a pointer ----------------------------------------------------------
template<class base_t>
template<class derived_t, class>
constexpr poly<base_t>::poly(derived_t* obj) :
	derived_ptr(obj),
	base_ptr(static_cast<base_t*>(obj)),
	copy_construct(&detail::clone<base_t, derived_t>) {
	static_assert(std::is_polymorphic<base_t>::value,
		"poly: poly can only be used with polymorphic types");
}

// Observers ===============================================================
template<class base_t>
template<class T>
constexpr bool poly<base_t>::is() const noexcept {
	return base_ptr.get() != nullptr && typeid(*base_ptr.get()) == typeid(T);
}

template<class base_t>
inline poly<base_t>::operator bool() const noexcept {
	return static_cast<bool>(base_ptr);
}

// Modifiers ===============================================================
template<class base_t>
inline base_t* poly<base_t>::release() noexcept {
	base_t* rslt = base_ptr.release();
	reset();

	return rslt;
}

template<class base_t>
inline void poly<base_t>::reset() noexcept {
	base_ptr.reset();
	derived_ptr = nullptr;
	copy_construct = nullptr;
}

template<class base_t>
template<class derived_t, class>
inline void poly<base_t>::reset(derived_t* obj) {
	derived_ptr = obj;
	base_ptr.reset(obj);
	copy_construct = &detail::clone<base_t, derived_t>;
}

// Member access ===========================================================
template<class base_t>
inline base_t& poly<base_t>::operator*() {
	return *base_ptr;
}

template<class base_t>
constexpr base_t& poly<base_t>::operator*() const {
	return *base_ptr;
}

template<class base_t>
inline base_t* poly<base_t>::operator->() {
	return base_ptr.get();
}

template<class base_t>
constexpr base_t* poly<base_t>::operator->() const {
	return base_ptr.get();
}

template<class base_t>
inline base_t* poly<base_t>::get() {
	return base_ptr.get();
}

template<class base_t>
constexpr base_t* poly<base_t>::get() const {
	return base_ptr.get();
}

template<class base_t>
template<class T>
typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class base_t>
template<class T>
constexpr typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() const {

	if (is<T>()) {
		return *static_cast<T*>(derived_ptr);
	}
	throw std::bad_cast();
}

template<class base_t>
void swap(poly<base_t>& lhs, poly<base_t>& rhs) noexcept {
	using std::swap;

	swap(lhs.derived_ptr, rhs.derived_ptr);
	swap(lhs.base_ptr, rhs.base_ptr);
	swap(lhs.copy_construct, rhs.copy_construct);
}

} // namespace zhukov