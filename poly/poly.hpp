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

template<class base_t>
class poly {
public:
	using element_type = base_t;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly() noexcept;
	constexpr poly(std::nullptr_t) noexcept;
	constexpr poly(const poly& other);
	constexpr poly(poly&& other) noexcept;
	poly& operator=(poly other);

	// From a pointer ----------------------------------------------------------
	template <class derived_t, class = typename std::enable_if<
		std::is_base_of<base_t, derived_t>::value>>
		constexpr poly(derived_t* obj);

	// Destruction -------------------------------------------------------------
	~poly() = default;

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit operator bool() const noexcept;

	// Modifiers ===============================================================
	base_t* release() noexcept;
	void reset() noexcept;
	template <class derived_t, class = typename std::enable_if<
		std::is_base_of<base_t, derived_t>::value>>
		void reset(derived_t* obj);

	// Member access ===========================================================
	base_t& operator*();
	constexpr base_t& operator*() const;

	base_t* operator->();
	constexpr base_t* operator->() const;

	base_t* get();
	constexpr base_t* get() const;

	template <class T>
	typename 
		std::enable_if<std::is_base_of<base_t, T>::value, T&>::type
		as();

	template <class T>
	constexpr typename 
		std::enable_if<std::is_base_of<base_t, T>::value, T&>::type
		as() const;

	// Friends =================================================================
	template <class T>
	friend void swap(poly<T>& lhs, poly<T>& rhs) noexcept;

private:
	void* derived_ptr; // points to derived
	std::unique_ptr<base_t> base_ptr; // points to base

	std::pair<void*, void*> (*copy_construct)(const void*);
};

template <class base_t, class derived_t, class... Args>
poly<base_t> make_poly(Args&&... args) {
	return poly<base_t>(new derived_t(std::forward<Args>(args)...));
}

template <class new_base_t, class derived_t, class old_base_t>
poly<new_base_t> transform_poly(const poly<old_base_t>& other) {
	derived_t* new_ptr = new derived_t(other.as<derived_t>());
	return poly<new_base_t>(new_ptr);
}

template <class new_base_t, class derived_t, class old_base_t>
poly<new_base_t> transform_poly(poly<old_base_t>&& other) {
	derived_t* new_ptr = other.as<derived_t>();
	other.reset();
	return poly<new_base_t>(new_ptr);
}

} // namespace zhukov

#include "poly.inl"