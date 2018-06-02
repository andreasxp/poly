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
	constexpr poly_ptr(poly_ptr&& other) noexcept;
	constexpr poly_ptr& operator=(poly_ptr&& other) noexcept;
	constexpr poly_ptr& operator=(std::nullptr_t) noexcept;

	poly_ptr(const poly_ptr&) = delete;
	poly_ptr& operator=(const poly_ptr&) = delete;

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
	typename std::enable_if<std::is_base_of<Base, T>::value, T*>::type
		as() const noexcept;

private:
	std::unique_ptr<Base> base_ptr;
};

template <class Base, class Derived, class... Args>
poly_ptr<Base> make_poly_ptr(Args&&... args) {
	return poly_ptr<Base>(new Derived(std::forward<Args>(args)...));
}

template <class NewBase, class Derived, class OldBase>
poly_ptr<NewBase> transform_poly_ptr(const poly_ptr<OldBase>& other) {
	Derived* new_ptr = nullptr;
	if (other) new_ptr = new Derived(*other.template as<Derived>());

	return poly_ptr<NewBase>(new_ptr);
}

template <class NewBase, class Derived, class OldBase>
poly_ptr<NewBase> transform_poly_ptr(poly_ptr<OldBase>&& other) {
	Derived* new_ptr = other.template as<Derived>();
	other.reset();
	return poly_ptr<NewBase>(new_ptr);
}

} // namespace zhukov

#include "poly_ptr.inl"