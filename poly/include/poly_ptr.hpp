#pragma once

#include <atomic>      // atomic
#include <cstddef>     // nullptr_t, ptrdiff_t
#include <memory>      // unique_ptr
#include <type_traits> // is_polymorphic, enable_if, is_base_of
#include <utility>     // forward

namespace zhukov {

template<class Base>
class poly_ptr {
	static_assert(std::is_polymorphic<Base>::value,
		"poly_ptr: poly_ptr can only be used with polymorphic types");
public:
	using pointer = Base*;
	using element_type = Base;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly_ptr() noexcept;
	constexpr poly_ptr(std::nullptr_t) noexcept;
	poly_ptr(poly_ptr&& other) noexcept;
	poly_ptr& operator=(poly_ptr&& other) noexcept;
	poly_ptr& operator=(std::nullptr_t) noexcept;

	poly_ptr(const poly_ptr&) = delete;
	poly_ptr& operator=(const poly_ptr&) = delete;

	// From a pointer ----------------------------------------------------------
	template <class Derived>
	explicit poly_ptr(Derived* obj);

	// Destruction -------------------------------------------------------------
	~poly_ptr() = default;

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit constexpr operator bool() const noexcept;

	// Modifiers ===============================================================
	pointer release() noexcept;
	void reset(std::nullptr_t = nullptr) noexcept;

	template <class Derived>
	void reset(Derived* obj) noexcept;

	void swap(poly_ptr& other) noexcept;

	// Member access ===========================================================
	Base& operator*() const;
	pointer operator->() const noexcept;
	pointer get() const noexcept;

	template <class T>
	T* as() const noexcept;

private:
	std::unique_ptr<Base> base_ptr;
};

template<class Base>
inline void swap(poly_ptr<Base>& lhs, poly_ptr<Base>& rhs) noexcept;

template <class Base, class Derived, class... Args>
inline poly_ptr<Base> make_poly_ptr(Args&&... args);

template <class NewBase, class Derived, class OldBase>
inline poly_ptr<NewBase> transform_poly_ptr(const poly_ptr<OldBase>& other);

template <class NewBase, class Derived, class OldBase>
inline poly_ptr<NewBase> transform_poly_ptr(poly_ptr<OldBase>&& other);

} // namespace zhukov

#include "poly_ptr.inl"