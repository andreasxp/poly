#pragma once
#include "poly.hpp"

namespace pl {

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base, class CopyDeletePolicy>
inline constexpr poly<Base, CopyDeletePolicy>::poly() noexcept :
	data (nullptr) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::poly(const poly& other) :
	policy(other),
	data(nullptr) {
	if (other) data = this->clone(other.get());
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::poly(poly&& other) noexcept :
	policy(std::move(static_cast<policy&>(other))),
	data(other.release()) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(const poly& other) {
	policy::operator=(other);

	reset();
	if (other) data = this->clone(other.get());
	
	return *this;
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>&
poly<Base, CopyDeletePolicy>::operator=(poly&& other) noexcept {
	policy::operator=(std::move(other));

	reset();
	data = other.release();

	return *this;
}

template<class Base, class CopyDeletePolicy>
inline constexpr poly<Base, CopyDeletePolicy>::poly(std::nullptr_t) noexcept :
	data(nullptr) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(std::nullptr_t) noexcept {
	data = nullptr;

	return *this;
}

// Converting constructors -------------------------------------------------

template<class Base, class CopyDeletePolicy>
template<class Base2, class CopyDeletePolicy2, class>
inline poly<Base, CopyDeletePolicy>::
poly(const poly<Base2, CopyDeletePolicy2>& other) :
	policy(static_cast<CopyDeletePolicy2>(other)),
	data(nullptr) {
	if (other) data = this->clone(other.get());
}

template<class Base, class CopyDeletePolicy>
template<class Base2, class CopyDeletePolicy2, class>
inline poly<Base, CopyDeletePolicy>::
poly(poly<Base2, CopyDeletePolicy2>&& other) noexcept :
	policy(std::move(static_cast<CopyDeletePolicy2>(other))),
	data(other.release()) {
}

// From a pointer ----------------------------------------------------------

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline poly<Base, CopyDeletePolicy>::poly(Derived* obj) :
	policy(obj),
	data(obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	reset(obj);
	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

// Destruction -------------------------------------------------------------

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::~poly() {
	if (data) this->destroy(get());
}

// Observers ===============================================================

template<class Base, class CopyDeletePolicy>
template<class T>
constexpr bool poly<Base, CopyDeletePolicy>::is() const noexcept {
	return get() != nullptr && typeid(*get()) == typeid(T);
}

template<class Base, class CopyDeletePolicy>
constexpr poly<Base, CopyDeletePolicy>::operator bool() const noexcept {
	return static_cast<bool>(data);
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::release() noexcept {
	Base* rslt = data;
	data = nullptr;

	return rslt;
}

template<class Base, class CopyDeletePolicy>
inline void poly<Base, CopyDeletePolicy>::reset(std::nullptr_t) noexcept {
	if (data) {
		destroy(data);
		data = nullptr;
	}
}

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline void poly<Base, CopyDeletePolicy>::reset(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	if (data) destroy(data);
	data = obj;
	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

template<class Base, class CopyDeletePolicy>
inline Base& poly<Base, CopyDeletePolicy>::operator*() const {
	return *data;
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::operator->() const noexcept {
	return data;
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::get() const noexcept {
	return data;
}

// Member access ===========================================================

template<class Base, class CopyDeletePolicy>
template<class T>
T* poly<Base, CopyDeletePolicy>::as() const noexcept {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return detail::inheritance_traits<Base, T>::downcast(get());
	}
	return nullptr;
}

// Non-member functions ========================================================

template<class PolyType, class Derived, class... Args>
PolyType make(Args&&... args) {
	return PolyType(new Derived(std::forward<Args>(args)...));
}

template<class PolyType, class Derived, class OldBase, class CopyDeletePolicy>
PolyType transform(const poly<OldBase, CopyDeletePolicy>& other) {
	if (other) {
		return PolyType(
			new Derived(*other.template as<Derived>()));
	}
	return PolyType();
}

template<class PolyType, class Derived, class OldBase, class CopyDeletePolicy>
PolyType transform(poly<OldBase, CopyDeletePolicy>&& other) {
	Derived* temp = other.template as<Derived>();
	other.release();
	
	return PolyType(temp);
}

// Comparison operators ========================================================

namespace detail {

template<class T, class R = void>
struct enable_if_defined {
	using type = R;
};

template<class T, class Enable = void>
struct defines_type : std::false_type {
};

template<class T>
struct defines_type<
	T, typename enable_if_defined<typename T::type>::type> : std::true_type {
};

} // namespace detail

template<class B1, class C1, class B2, class C2>
	bool operator==(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return x.get() == y.get();
}

template<class B1, class C1, class B2, class C2>
	bool operator!=(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return !(x == y);
}

template<class B1, class C1, class B2, class C2>
	bool operator<(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	using common = std::common_type<
		typename poly<B1, C1>::base_type*, 
		typename poly<B2, C2>::base_type*>;
	static_assert(detail::defines_type<common>::value, 
		"comparison: these poly types cannot be compared: internal objects must have the same class, or be convertible to one base class");

	return std::less<typename common::type>()(x.get(), y.get());
}

template<class B1, class P1, class B2, class P2>
	bool operator<=(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return !(y < x);
}

template<class B1, class P1, class B2, class P2>
	bool operator>(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return y < x;
}

template<class B1, class P1, class B2, class P2>
	bool operator>=(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return !(x < y);
}

template <class B, class P>
bool operator==(const poly<B, P>& x, std::nullptr_t) noexcept {
	return !(x != nullptr);
}

template <class B, class P>
bool operator==(std::nullptr_t, const poly<B, P>& x) noexcept {
	return !(nullptr != x);
}

template <class B, class P>
bool operator!=(const poly<B, P>& x, std::nullptr_t) noexcept {
	return static_cast<bool>(x);
}

template <class B, class P>
bool operator!=(std::nullptr_t, const poly<B, P>& x) noexcept {
	return static_cast<bool>(x);
}

template <class B, class P>
bool operator<(const poly<B, P>& x, std::nullptr_t) {
	return std::less<typename poly<B, P>::base_type*>()(x.get(), nullptr);
}

template <class B, class P>
bool operator<(std::nullptr_t, const poly<B, P>& y) {
	return std::less<typename poly<B, P>::base_type*>()(nullptr, y.get());
}

template <class B, class P>
bool operator<=(const poly<B, P>& x, std::nullptr_t) {
	return !(nullptr < x);
}

template <class B, class P>
bool operator<=(std::nullptr_t, const poly<B, P>& y) {
	return !(y < nullptr);
}

template <class B, class P>
bool operator>(const poly<B, P>& x, std::nullptr_t) {
	return nullptr < x;
}

template <class B, class P>
bool operator>(std::nullptr_t, const poly<B, P>& y) {
	return y < nullptr;
}

template <class B, class P>
bool operator>=(const poly<B, P>& x, std::nullptr_t) {
	return !(x < nullptr);
}

template <class B, class P>
bool operator>=(std::nullptr_t, const poly<B, P>& y) {
	return !(nullptr < y);
}

} // namespace pl

namespace std {

template<class Base, class CopyDeletePolicy>
struct hash<pl::poly<Base, CopyDeletePolicy>> {
	size_t operator()(const pl::poly<Base, CopyDeletePolicy>& x) const {
		return hash<typename pl::poly<Base, CopyDeletePolicy>::base_type*>()(x.get());
	}
};

} // namespace std