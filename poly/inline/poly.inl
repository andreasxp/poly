#pragma once
#include "poly.hpp"

namespace zhukov {

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base, template<class> class CopyPolicy>
inline constexpr poly<Base, CopyPolicy>::poly(std::nullptr_t) noexcept :
	data(nullptr) {
}

template<class Base, template<class> class CopyPolicy>
inline poly<Base, CopyPolicy>::poly(const poly& other) :
	copy_policy(other),
	data(nullptr) {

	if (other) {
		data.reset(clone(other.data.get()));
	}
}

template<class Base, template<class> class CopyPolicy>
inline poly<Base, CopyPolicy>& 
poly<Base, CopyPolicy>::operator=(const poly& other) {
	copy_policy::operator=(other);

	if (other) {
		data.reset(clone(other.data.get()));
	}
	
	return *this;
}

template<class Base, template<class> class CopyPolicy>
inline poly<Base, CopyPolicy>& 
poly<Base, CopyPolicy>::operator=(std::nullptr_t) noexcept {
	data = nullptr;

	return *this;
}

// From an object ----------------------------------------------------------

template<class Base, template<class> class CopyPolicy>
template<class Derived>
inline poly<Base, CopyPolicy>::poly(Derived* obj) :
	copy_policy(obj),
	data(obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");

	detail::inheritance_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(data.get());
}

template<class Base, template<class> class CopyPolicy>
template<class Derived>
inline poly<Base, CopyPolicy>& 
poly<Base, CopyPolicy>::operator=(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");

	data.reset(obj);

	detail::inheritance_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(data.get());
}

// Observers ===============================================================
template<class Base, template<class> class CopyPolicy>
template<class T>
constexpr bool poly<Base, CopyPolicy>::is() const noexcept {
	return data.get() != nullptr && typeid(*data.get()) == typeid(T);
}

template<class Base, template<class> class CopyPolicy>
constexpr poly<Base, CopyPolicy>::operator bool() const noexcept {
	return static_cast<bool>(data);
}

template<class Base, template<class> class CopyPolicy>
inline Base* poly<Base, CopyPolicy>::release() noexcept {
	return data.release();
}

template<class Base, template<class> class CopyPolicy>
inline void poly<Base, CopyPolicy>::reset(std::nullptr_t) noexcept {
	data.reset();
}

template<class Base, template<class> class CopyPolicy>
template<class Derived>
inline void poly<Base, CopyPolicy>::reset(Derived* obj) noexcept {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");

	data.reset(obj);

	detail::inheritance_traits<Base, Derived>::offset =
		reinterpret_cast<unsigned char*>(obj) -
		reinterpret_cast<unsigned char*>(data.get());
}

template<class Base, template<class> class CopyPolicy>
inline Base& poly<Base, CopyPolicy>::operator*() const {
	return *data;
}

template<class Base, template<class> class CopyPolicy>
inline Base* poly<Base, CopyPolicy>::operator->() const noexcept {
	return  data.get();
}

// Member access ===========================================================
template<class Base, template<class> class CopyPolicy>
inline Base* poly<Base, CopyPolicy>::get() const noexcept {
	return data.get();
}

template<class Base, template<class> class CopyPolicy>
template<class T>
T* poly<Base, CopyPolicy>::as() const noexcept {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return detail::inheritance_traits<Base, T>::downcast(data.get());
	}
	return nullptr;
}

template <class Base, class Derived,
	class... Args>
	inline poly<Base> make_poly(Args&&... args) {
	return poly<Base>(
		new Derived(std::forward<Args>(args)...));
}

template <class Base, template<class> class CopyPolicy, class Derived,
	class... Args>
	inline poly<Base, CopyPolicy> make_poly(Args&&... args) {
	return poly<Base, CopyPolicy>(
		new Derived(std::forward<Args>(args)...));
}

template <
	class NewBase, class Derived,
	class OldBase, template<class> class CopyPolicy>
	inline poly<NewBase, CopyPolicy>
	transform_poly(const poly<OldBase, CopyPolicy>& other) {
	return transform_poly<NewBase, CopyPolicy, Derived>(other);
}

template <
	class NewBase, template<class> class NewCopyPolicy, class Derived,
	class OldBase, template<class> class OldCopyPolicy>
	inline poly<NewBase, NewCopyPolicy>
	transform_poly(const poly<OldBase, OldCopyPolicy>& other) {
	if (other) {
		return poly<NewBase, NewCopyPolicy>(
			new Derived(*other.template as<Derived>()));
	}
	return poly<NewBase, NewCopyPolicy>();
}

template <
	class NewBase, class Derived,
	class OldBase, template<class> class CopyPolicy>
	inline poly<NewBase, CopyPolicy>
	transform_poly(poly<OldBase, CopyPolicy>&& other) {
	return transform_poly<NewBase, CopyPolicy, Derived>(std::move(other));
}

template <
	class NewBase, template<class> class NewCopyPolicy, class Derived,
	class OldBase, template<class> class OldCopyPolicy>
	inline poly<NewBase, NewCopyPolicy>
	transform_poly(poly<OldBase, OldCopyPolicy>&& other) {
	Derived* temp = other.template as<Derived>();
	return poly<NewBase, NewCopyPolicy>(temp);
	other.release();
}

// Comparison operators ========================================================

//template<class T1, class D1, class T2, class D2>
//bool operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

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

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator==(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return x.get() == y.get();
}

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator!=(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return !(x == y);
}

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator<(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	using common = std::common_type<poly<B1, C1>::base_type*, poly<B2, C2>::base_type*>;
	static_assert(detail::defines_type<common>::value, 
		"comparison: these poly types cannot be compared: internal objects must have the same class, or be convertible to one base class");

	return std::less<common::type>()(x.get(), y.get());
}

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator<=(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return !(y < x);
}

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator>(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return y < x;
}

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
	bool operator>=(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return !(x < y);
}

template <class B, template <class> class C>
bool operator==(const poly<B, C>& x, nullptr_t) noexcept {
	return !(x != nullptr);
}

template <class B, template <class> class C>
bool operator==(nullptr_t, const poly<B, C>& x) noexcept {
	return !(nullptr != x);
}

template <class B, template <class> class C>
bool operator!=(const poly<B, C>& x, nullptr_t) noexcept {
	return static_cast<bool>(x);
}

template <class B, template <class> class C>
bool operator!=(nullptr_t, const poly<B, C>& x) noexcept {
	return static_cast<bool>(x);
}

template <class B, template <class> class C>
bool operator<(const poly<B, C>& x, nullptr_t) {
	return std::less<poly<B, C>::base_type*>()(x.get(), nullptr);
}

template <class B, template <class> class C>
bool operator<(nullptr_t, const poly<B, C>& y) {
	return std::less<poly<B, C>::base_type*>()(nullptr, y.get());
}

template <class B, template <class> class C>
bool operator<=(const poly<B, C>& x, nullptr_t) {
	return !(nullptr < x);
}

template <class B, template <class> class C>
bool operator<=(nullptr_t, const poly<B, C>& y) {
	return !(y < nullptr);
}

template <class B, template <class> class C>
bool operator>(const poly<B, C>& x, nullptr_t) {
	return nullptr < x;
}

template <class B, template <class> class C>
bool operator>(nullptr_t, const poly<B, C>& y) {
	return y < nullptr;
}

template <class B, template <class> class C>
bool operator>=(const poly<B, C>& x, nullptr_t) {
	return !(x < nullptr);
}

template <class B, template <class> class C>
bool operator>=(nullptr_t, const poly<B, C>& y) {
	return !(nullptr < y);
}

} // namespace zhukov

namespace std {

template<class Base, template<class> class CopyPolicy>
struct hash<zhukov::poly<Base, CopyPolicy>> {
	size_t operator()(const zhukov::poly<Base, CopyPolicy>& x) const {
		return hash<zhukov::poly<Base, CopyPolicy>::base_type*>()(x.get());
	}
};

} // namespace std