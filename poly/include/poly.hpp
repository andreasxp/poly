#pragma once

#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>      // string
#include <typeinfo>    // typeid, bad_cast
#include <type_traits> // has_virtual_destructor, enable_if, is_base_of
#include <utility>     // forward

#include "policy.hpp"
#include "cv_checks.hpp"

namespace pl {

template<class Base, class CopyDeletePolicy = deep<Base>>
class poly : private CopyDeletePolicy {
	static_assert(std::is_polymorphic<Base>::value,
		"poly: Base is not polymorphic, poly is useless. Consider using std::unique_ptr instead.");
public:
	using base_type = Base;
	using policy = CopyDeletePolicy;

	// Construction ============================================================
	// Default, copy, move -----------------------------------------------------
	constexpr poly() noexcept;
	poly(const poly& other);
	poly(poly&&) noexcept;
	poly& operator=(const poly& other);
	poly& operator=(poly&&) noexcept;
	
	constexpr poly(std::nullptr_t) noexcept;
	poly& operator=(std::nullptr_t) noexcept;

	// Converting constructors -------------------------------------------------
	template <class Base2, class CopyDeletePolicy2, 
		class = typename std::enable_if<
		detail::is_stronger_qualified<Base, Base2>::value>::type>
	poly(const poly<Base2, CopyDeletePolicy2>& other);

	template <class Base2, class CopyDeletePolicy2,
		class = typename std::enable_if<
		detail::is_stronger_qualified<Base, Base2>::value>::type>
	poly(poly<Base2, CopyDeletePolicy2>&& other) noexcept;

	// From a pointer ----------------------------------------------------------
	template <class Derived>
	explicit poly(Derived* obj);
	template <class Derived>
	poly& operator=(Derived* obj);

	// Destruction -------------------------------------------------------------
	~poly();

	// Observers ===============================================================
	template <class T>
	constexpr bool is() const noexcept;

	explicit constexpr operator bool() const noexcept;

	// Modifiers ===============================================================
	template <class Derived>
	void reset(Derived* obj);
	void reset(std::nullptr_t = nullptr) noexcept;

	Base* release() noexcept;

	// Member access ===========================================================
	Base& operator*() const;
	Base* operator->() const noexcept;
	Base* get() const noexcept;

	template <class T>
	T* as() const noexcept;

	// Friends =================================================================
	template<class Base2, class CopyDeletePolicy2>
	friend class poly;

private:
	Base* data;
};

// Make ========================================================================

template <class PolyType, class Derived, class... Args>
inline PolyType make(Args&&... args);

// Transform ===================================================================

template <class PolyType, class Derived, 
	class OldBase, class CopyDeletePolicy>
inline PolyType transform(const poly<OldBase, CopyDeletePolicy>& other);

template <class PolyType, class Derived,
	class OldBase, class CopyDeletePolicy>
inline PolyType transform(poly<OldBase, CopyDeletePolicy>&& other);

// Comparison operators ========================================================

template<class B1, class P1, class B2, class P2>
bool operator==(const poly<B1, P1>& x, const poly<B2, P2>& y);

template<class B1, class P1, class B2, class P2>
bool operator!=(const poly<B1, P1>& x, const poly<B2, P2>& y);

template<class B1, class P1, class B2, class P2>
bool operator<(const poly<B1, P1>& x, const poly<B2, P2>& y);

template<class B1, class P1, class B2, class P2>
bool operator<=(const poly<B1, P1>& x, const poly<B2, P2>& y);

template<class B1, class P1, class B2, class P2>
bool operator>(const poly<B1, P1>& x, const poly<B2, P2>& y);

template<class B1, class P1, class B2, class P2>
bool operator>=(const poly<B1, P1>& x, const poly<B2, P2>& y);

template <class B, class P>
bool operator==(const poly<B, P>& x, nullptr_t) noexcept;

template <class B, class P>
bool operator==(nullptr_t, const poly<B, P>& x) noexcept;

template <class B, class P>
bool operator!=(const poly<B, P>& x, nullptr_t) noexcept;

template <class B, class P>
bool operator!=(nullptr_t, const poly<B, P>& x) noexcept;

template <class B, class P>
bool operator<(const poly<B, P>& x, nullptr_t);

template <class B, class P>
bool operator<(nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator<=(const poly<B, P>& x, nullptr_t);

template <class B, class P>
bool operator<=(nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator>(const poly<B, P>& x, nullptr_t);

template <class B, class P>
bool operator>(nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator>=(const poly<B, P>& x, nullptr_t);

template <class B, class P>
bool operator>=(nullptr_t, const poly<B, P>& y);

} // namespace pl

namespace std {
template<class Base, class CopyDeletePolicy>
struct hash<pl::poly<Base, CopyDeletePolicy>>;
} // namespace std

#include "poly.inl"