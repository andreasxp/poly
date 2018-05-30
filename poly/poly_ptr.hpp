#pragma once

#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>      // string
#include <typeinfo>    // typeid, bad_cast
#include <type_traits> // is_polymorphic, enable_if, is_base_of, 
                       // is_copy_constructible
#include <utility>     // forward

/*!
\brief   Macro to override typeid for rtti in poly
\details Define POLY_CUSTOM_RTTI(type) as a function that returns
         your type's name. Do this **before** including poly.hpp
		 or poly_factory.hpp. 
\example #define POLY_CUSTOM_RTTI(...) my_typeid(__VA_ARGS__).name();
\example #define POLY_CUSTOM_RTTI(...) prid<__VA_ARGS__>().name();
\see     https://github.com/andreasxp/prindex
*/
#ifdef POLY_CUSTOM_TYPE_NAME
#define POLY_TYPE_NAME POLY_CUSTOM_TYPE_NAME
#else
/// Get name of type
#define POLY_TYPE_NAME(...) typeid(__VA_ARGS__).name()
#endif

/// Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

template<class Base>
class poly_ptr {
public:
	using pointer = Base*;
	using element_type = Base;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly_ptr() noexcept;
	constexpr poly_ptr(std::nullptr_t) noexcept;
	constexpr poly_ptr(poly_ptr&& other) noexcept;
	constexpr poly_ptr& operator=(poly_ptr&& other) noexcept;
	constexpr poly_ptr& operator=(std::nullptr_t) noexcept;

	// From a pointer ----------------------------------------------------------
	template <class Derived, class = typename std::enable_if<
		std::is_base_of<Base, Derived>::value>>
		explicit constexpr poly_ptr(Derived* obj);

	// Destruction -------------------------------------------------------------
	~poly_ptr() = default;

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit constexpr operator bool() const noexcept;

	// Modifiers ===============================================================
	pointer release() noexcept;
	void reset(std::nullptr_t = nullptr) noexcept;
	template <class Derived, class = typename std::enable_if<
		std::is_base_of<Base, Derived>::value>>
		void reset(Derived* obj) noexcept;

	void swap(poly_ptr& other) noexcept;

	// Member access ===========================================================
	Base& operator*() const;
	pointer operator->() const noexcept;
	pointer get() const noexcept;

	template <class T>
	typename 
		std::enable_if<std::is_base_of<Base, T>::value, T*>::type
		as();

	template <class T>
	constexpr typename 
		std::enable_if<std::is_base_of<Base, T>::value, T*>::type
		as() const;

private:
	void* derived_ptr; // points to derived
	std::unique_ptr<Base> base_ptr; // points to base
};

template <class Base, class Derived, class... Args>
poly_ptr<Base> make_poly_ptr(Args&&... args) {
	return poly_ptr<Base>(new Derived(std::forward<Args>(args)...));
}

template <class new_base_t, class Derived, class old_base_t>
poly_ptr<new_base_t> transform_poly_ptr(const poly_ptr<old_base_t>& other) {
	Derived* new_ptr = new Derived(*other.as<Derived>());
	return poly_ptr<new_base_t>(new_ptr);
}

template <class new_base_t, class Derived, class old_base_t>
poly_ptr<new_base_t> transform_poly_ptr(poly_ptr<old_base_t>&& other) {
	Derived* new_ptr = other.as<Derived>();
	other.reset();
	return poly_ptr<new_base_t>(new_ptr);
}

} // namespace zhukov

#include "poly_ptr.inl"