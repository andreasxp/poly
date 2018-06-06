#pragma once

#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>      // string
#include <typeinfo>    // typeid, bad_cast
#include <type_traits> // has_virtual_destructor, enable_if, is_base_of
#include <utility>     // forward

#include "copy_policy.hpp"

namespace zhukov {

template<class Base, template<class> class CopyPolicy = no_copy>
class poly : private CopyPolicy<Base> {
	static_assert(std::has_virtual_destructor<Base>::value,
		"poly: Base must have a virtual destructor");
public:
	using base_type = Base;
	using copy_policy = CopyPolicy<Base>;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly() noexcept = default;
	constexpr poly(std::nullptr_t) noexcept;
	poly(const poly& other);
	poly(poly&&) noexcept = default;
	poly& operator=(const poly& other);
	poly& operator=(poly&&) noexcept = default;
	poly& operator=(std::nullptr_t) noexcept;

	// From a pointer ----------------------------------------------------------
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

	// Modifiers ===============================================================
	Base* release() noexcept;
	void reset(std::nullptr_t = nullptr) noexcept;

	template <class Derived>
	void reset(Derived* obj) noexcept;

	// Member access ===========================================================
	Base& operator*() const;
	Base* operator->() const noexcept;
	Base* get() const noexcept;

	template <class T>
	T* as() const noexcept;

private:
	std::unique_ptr<Base> data;
};

// Make ========================================================================

template <class Base, class Derived, 
	class... Args>
inline poly<Base> make_poly(Args&&... args);

template <class Base, template<class> class CopyPolicy, class Derived,
	class... Args>
inline poly<Base, CopyPolicy> make_poly(Args&&... args);

// Transform ===================================================================

template <
	class NewBase, class Derived, 
	class OldBase, template<class> class CopyPolicy>
inline poly<NewBase, CopyPolicy> 
transform_poly(const poly<OldBase, CopyPolicy>& other);

template <
	class NewBase, template<class> class NewCopyPolicy, class Derived,
	class OldBase, template<class> class OldCopyPolicy>
inline poly<NewBase, NewCopyPolicy> 
transform_poly(const poly<OldBase, OldCopyPolicy>& other);

template <
	class NewBase, class Derived, 
	class OldBase, template<class> class CopyPolicy>
inline poly<NewBase, CopyPolicy> 
transform_poly(poly<OldBase, CopyPolicy>&& other);

template <
	class NewBase, template<class> class NewCopyPolicy, class Derived,
	class OldBase, template<class> class OldCopyPolicy>
inline poly<NewBase, NewCopyPolicy> 
transform_poly(poly<OldBase, OldCopyPolicy>&& other);

// Comparison operators ========================================================

template<
	class B1, template <class> class C1, 
	class B2, template <class> class C2>
bool operator==(const poly<B1, C1>& x, const poly<B2, C2>& y);

template<
	class B1, template <class> class C1, 
	class B2, template <class> class C2>
bool operator!=(const poly<B1, C1>& x, const poly<B2, C2>& y);

template<
	class B1, template <class> class C1,
	class B2, template <class> class C2>
bool operator<(const poly<B1, C1>& x, const poly<B2, C2>& y);

template<
	class B1, template <class> class C1, 
	class B2, template <class> class C2>
bool operator<=(const poly<B1, C1>& x, const poly<B2, C2>& y);

template<
	class B1, template <class> class C1, 
	class B2, template <class> class C2>
bool operator>(const poly<B1, C1>& x, const poly<B2, C2>& y);

template<
	class B1, template <class> class C1, 
	class B2, template <class> class C2>
bool operator>=(const poly<B1, C1>& x, const poly<B2, C2>& y);

template <class B, template <class> class C>
bool operator==(const poly<B, C>& x, nullptr_t) noexcept;

template <class B, template <class> class C>
bool operator==(nullptr_t, const poly<B, C>& x) noexcept;

template <class B, template <class> class C>
bool operator!=(const poly<B, C>& x, nullptr_t) noexcept;

template <class B, template <class> class C>
bool operator!=(nullptr_t, const poly<B, C>& x) noexcept;

template <class B, template <class> class C>
bool operator<(const poly<B, C>& x, nullptr_t);

template <class B, template <class> class C>
bool operator<(nullptr_t, const poly<B, C>& y);

template <class B, template <class> class C>
bool operator<=(const poly<B, C>& x, nullptr_t);

template <class B, template <class> class C>
bool operator<=(nullptr_t, const poly<B, C>& y);

template <class B, template <class> class C>
bool operator>(const poly<B, C>& x, nullptr_t);

template <class B, template <class> class C>
bool operator>(nullptr_t, const poly<B, C>& y);

template <class B, template <class> class C>
bool operator>=(const poly<B, C>& x, nullptr_t);

template <class B, template <class> class C>
bool operator>=(nullptr_t, const poly<B, C>& y);

} // namespace zhukov

namespace std {
template<class Base, template<class> class CopyPolicy>
struct hash<zhukov::poly<Base, CopyPolicy>>;
} // namespace std

#include "poly.inl"