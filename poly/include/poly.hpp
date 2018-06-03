#pragma once

#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>      // string
#include <typeinfo>    // typeid, bad_cast
#include <type_traits> // is_polymorphic, enable_if, is_base_of, 
                       // is_copy_constructible
#include <utility>     // forward

#include "polymorphic_traits.hpp"

/// Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

template<class Base>
class poly {
	static_assert(std::is_polymorphic<Base>::value,
		"poly: poly can only be used with polymorphic types");
public:
	using type = Base;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly() noexcept;
	poly(const poly& other);
	poly(poly&&) noexcept = default;
	poly& operator=(const poly& other);
	poly& operator=(poly&&) noexcept = default;

	// From an object ----------------------------------------------------------
	template <class Derived, class = typename std::enable_if<
		std::is_base_of<Base, Derived>::value>::type>
	explicit poly(const Derived& obj);

	template <class Derived, class = typename std::enable_if<
		std::is_base_of<Base, Derived>::value>::type>
	explicit poly(Derived&& obj);

	// Destruction -------------------------------------------------------------
	~poly() = default;

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit constexpr operator bool() const noexcept;

	// Member access ===========================================================
	operator Base&();
	operator const Base&() const;

	Base& get();
	const Base& get() const;

	template <class T>
	T& as();

	template <class T>
	const T& as() const;

private:
	poly_ptr<Base> data;

	Base* (*copy_construct)(const Base*);
};

template <class Base, class Derived, class... Args>
inline poly<Base> make_poly(Args&&... args) {
	return poly<Base>(std::move(Derived(std::forward<Args>(args)...)));
}

template <class NewBase, class Derived, class OldBase>
inline poly<NewBase> transform_poly(const poly<OldBase>& other) {
	return poly<NewBase>(Derived(other.template as<Derived>()));
}

template <class NewBase, class Derived, class OldBase>
inline poly<NewBase> transform_poly(poly<OldBase>&& other) {
	return poly<NewBase>(Derived(std::move(other.template as<Derived>())));
}

} // namespace zhukov

#include "poly.inl"