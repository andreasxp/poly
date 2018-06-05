#pragma once
#include "poly.hpp"

namespace zhukov {

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base, class CopyPolicy>
inline constexpr poly<Base, CopyPolicy>::poly(std::nullptr_t) noexcept :
	data(nullptr) {
}

template<class Base, class CopyPolicy>
inline poly<Base, CopyPolicy>::poly(const poly& other) :
	CopyPolicy(other),
	data(nullptr) {

	if (other) {
		data.reset(copy_construct(other.data.get()));
	}
}

template<class Base, class CopyPolicy>
inline poly<Base, CopyPolicy>& poly<Base, CopyPolicy>::operator=(const poly& other) {
	CopyPolicy::operator=(other);
	copy_construct = other.copy_construct;

	if (other) {
		data.reset(copy_construct(other.data.get()));
	}
	
	return *this;
}

template<class Base, class CopyPolicy>
inline poly<Base, CopyPolicy>& poly<Base, CopyPolicy>::operator=(std::nullptr_t) noexcept {
	data = nullptr;

	return *this;
}

// From an object ----------------------------------------------------------

template<class Base, class CopyPolicy>
template<class Derived>
inline poly<Base, CopyPolicy>::poly(Derived* obj) :
	CopyPolicy(obj),
	data(obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");

	detail::polymorphic_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(data.get());
}

template<class Base, class CopyPolicy>
template<class Derived>
inline poly<Base, CopyPolicy>& poly<Base, CopyPolicy>::operator=(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");

	data.reset(obj);

	detail::polymorphic_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(data.get());
}

// Observers ===============================================================
template<class Base, class CopyPolicy>
template<class T>
constexpr bool poly<Base, CopyPolicy>::is() const noexcept {
	return data.get() != nullptr && typeid(*data.get()) == typeid(T);
}

template<class Base, class CopyPolicy>
constexpr poly<Base, CopyPolicy>::operator bool() const noexcept {
	return static_cast<bool>(data);
}

template<class Base, class CopyPolicy>
inline Base& poly<Base, CopyPolicy>::operator*() const {
	return *data;
}

template<class Base, class CopyPolicy>
inline Base* poly<Base, CopyPolicy>::operator->() const noexcept {
	return  data.get();
}

// Member access ===========================================================
template<class Base, class CopyPolicy>
inline Base* poly<Base, CopyPolicy>::get() const noexcept {
	return data.get();
}

template<class Base, class CopyPolicy>
template<class T>
T* poly<Base, CopyPolicy>::as() const noexcept {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return detail::polymorphic_traits<Base, T>::downcast(data.get());
	}
	return nullptr;
}

template <class Base, class Derived, class CopyPolicy, class... Args>
inline poly<Base, CopyPolicy> make_poly(Args&&... args) {
	return poly<Base, CopyPolicy>(new Derived(std::forward<Args>(args)...));
}

template <class NewBase, class Derived, class CopyPolicy, class OldBase>
inline poly<NewBase, CopyPolicy> transform_poly(const poly<OldBase>& other) {
	return poly<NewBase, CopyPolicy>(new Derived(*other.template as<Derived>()));
}

template <class NewBase, class Derived, class CopyPolicy, class OldBase>
inline poly<NewBase, CopyPolicy> transform_poly(poly<OldBase>&& other) {
	return poly<NewBase, CopyPolicy>(new Derived(std::move(*other.template as<Derived>())));
}

} // namespace zhukov