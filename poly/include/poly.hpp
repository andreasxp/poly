#pragma once

#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>      // string
#include <typeinfo>    // typeid, bad_cast
#include <type_traits> // is_polymorphic, enable_if, is_base_of, 
                       // is_copy_constructible
#include <utility>     // forward

#include "copy_policy.hpp"

namespace zhukov {

template<class Base, class CopyPolicy = no_copy>
class poly : private CopyPolicy {
	static_assert(std::is_polymorphic<Base>::value,
		"poly: poly can only be used with polymorphic types");
public:
	using base_type = Base;
	using copy_policy = CopyPolicy;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly() noexcept = default;
	constexpr poly(std::nullptr_t) noexcept;
	poly(const poly& other);
	poly(poly&&) noexcept = default;
	poly& operator=(const poly& other);
	poly& operator=(poly&&) noexcept = default;
	poly& operator=(std::nullptr_t) noexcept;

	// From an object ----------------------------------------------------------
	template <class Derived>
	explicit poly(Derived* obj);
	template <class Derived>
	poly& operator=(Derived* obj);

	// Destruction -------------------------------------------------------------
	~poly() = default;

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit constexpr operator bool() const noexcept;

	// Member access ===========================================================
	Base& operator*() const;
	Base* operator->() const noexcept;
	Base* get() const noexcept;

	template <class T>
	T* as() const noexcept;

private:
	std::unique_ptr<Base> data;
};

template <class Base, class Derived, class CopyPolicy = no_copy, class... Args>
inline poly<Base, CopyPolicy> make_poly(Args&&... args);

template <
	class NewBase, class Derived, 
	class OldBase, class CopyPolicy>
inline poly<NewBase, CopyPolicy> 
transform_poly(const poly<OldBase, CopyPolicy>& other);

template <
	class NewBase, class NewCopyPolicy, class Derived, 
	class OldBase, class OldCopyPolicy>
inline poly<NewBase, NewCopyPolicy> 
transform_poly(const poly<OldBase, OldCopyPolicy>& other);

template <
	class NewBase, class Derived, 
	class OldBase, class CopyPolicy>
inline poly<NewBase, CopyPolicy> 
transform_poly(poly<OldBase, CopyPolicy>&& other);

template <
	class NewBase, class NewCopyPolicy, class Derived, 
	class OldBase, class OldCopyPolicy>
inline poly<NewBase, NewCopyPolicy> 
transform_poly(poly<OldBase, OldCopyPolicy>&& other);

} // namespace zhukov

#include "poly.inl"