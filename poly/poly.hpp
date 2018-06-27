#include <algorithm>
#include <atomic>      // atomic
#include <cstddef>     // nullptr_t
#include <memory>      // unique_ptr
#include <stdexcept>   // runtime_error
#include <string>        // string
#include <type_traits>   // is_base_of, is_default_constructible
#include <typeinfo>    // typeid, bad_cast
#include <utility>     // forward
#include <vector>        // vector




// MSVC fails to optimize using EBO by default.
// This macro enables the optimization
#ifdef _MSC_VER
#define POLY_MULTIPLE_EMPTY_BASES __declspec(empty_bases)
#else
#define POLY_MULTIPLE_EMPTY_BASES
#endif

namespace pl {

template <class Cloner, class Deleter>
class POLY_MULTIPLE_EMPTY_BASES compound : private Cloner, private Deleter {
public:
	// Constructors ============================================================
	constexpr compound() noexcept = default;

	template <class Cloner2, class Deleter2>
	compound(const compound<Cloner2, Deleter2>& other);

	template <class Cloner2, class Deleter2>
	compound(compound<Cloner2, Deleter2>&& other);

	// From a pointer ----------------------------------------------------------
	// If a Cloner or Deleter is not constructible from a pointer, it will be 
	// default-constrcted.
	template <class T>
	compound(const T* ptr);

	// Operations ==============================================================
	template <class T>
	auto clone(const T* ptr) -> decltype(Cloner::operator()(ptr));

	template <class T>
	void destroy(T* ptr);

	// Friends =================================================================
	template <class Cloner2, class Deleter2>
	friend class compound;

private:
	// Private constructors ====================================================
	// 2nd argument: whether Cloner  is_constructible from const T*
	// 3nd argument: whether Deleter is_constructible from const T*
	template <class T> compound(const T* ptr, std::false_type, std::false_type);
	template <class T> compound(const T* ptr, std::false_type, std::true_type);
	template <class T> compound(const T* ptr, std::true_type,  std::false_type);
	template <class T> compound(const T* ptr, std::true_type,  std::true_type);
};

} // namespace pl

#undef POLY_MULTIPLE_EMPTY_BASES


namespace pl {

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::
compound(const compound<Cloner2, Deleter2>& other) :
	Cloner(static_cast<Cloner2&>(other)),
	Deleter(static_cast<Deleter2&>(other)) {
}

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::
compound(compound<Cloner2, Deleter2>&& other) :
	Cloner(std::move(static_cast<Cloner2&>(other))),
	Deleter(std::move(static_cast<Deleter2&>(other))) {
}

template<class Cloner, class Deleter>
template<class T>
	inline compound<Cloner, Deleter>::compound(const T* ptr) : 
	compound(ptr, 
		std::is_constructible<Cloner,  const T*>(),
		std::is_constructible<Deleter, const T*>()) {
}

template<class Cloner, class Deleter>
template<class T>
inline auto compound<Cloner, Deleter>::clone(const T* ptr)
->decltype(Cloner::operator()(ptr)) {
	return Cloner::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline void compound<Cloner, Deleter>::destroy(T* ptr) {
	Deleter::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T*, std::false_type, std::false_type) :
	Cloner(),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::false_type, std::true_type) :
	Cloner(),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::true_type, std::false_type) :
	Cloner(ptr),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::true_type, std::true_type) :
	Cloner(ptr),
	Deleter(ptr) {
}

} // namespace pl

namespace pl {

/// Empty class. Provides no operator(), so a policy is not copyable.
class no_copy {};

/// Stores a pointer to a function that copies the Base pointer
template <class Base>
class deep_copy {
public:
	constexpr deep_copy() noexcept;

	template <class Base2>
	deep_copy(const deep_copy<Base2>& other) noexcept;

	template <class Derived>
	deep_copy(const Derived*);

	Base* operator()(const Base* other);

private:
	typename std::remove_const<Base>::type* (*clone_ptr)(const Base*);

	template <class Base2>
	friend class deep_copy;
};

} // namespace pl


/*!
\brief Macro to override typeid for rtti in poly
Define POLY_CUSTOM_RTTI(type) as a function that returns
your type's name. Do this **before** including poly.hpp
or factory.hpp.
\example #define POLY_CUSTOM_RTTI(...) my_typeid(__VA_ARGS__).name();
\example #define POLY_CUSTOM_RTTI(...) prid<__VA_ARGS__>().name();
\see     https://github.com/andreasxp/prindex
*/
#ifdef POLY_CUSTOM_TYPE_NAME
/// Get name of type
#define POLY_TYPE_NAME POLY_CUSTOM_TYPE_NAME
#else
/// Get name of type
#define POLY_TYPE_NAME(...) typeid(__VA_ARGS__).name()
#endif

namespace pl {
namespace detail {

/// A class that can downcast Base to Derived without dynamic_cast
template <class Base, class Derived>
class inheritance_traits_impl {
	static_assert(std::is_base_of<Base, Derived>::value,
		"inheritance_traits: Base is not base of Derived");
public:
	static void set_offset(const Base* base_ptr, const Derived* derived_ptr);

	static Derived* downcast(Base* ptr);
	static const Derived* downcast(const Base* ptr);

private:
	/// Holds a memory offset that converts Base* to Derived*
	static std::atomic<std::ptrdiff_t> offset;
};

// Inherits inheritance_traits_impl, so every base-derived
// pair has the same offset and members no matter how const
template <class Base, class Derived>
using inheritance_traits = inheritance_traits_impl<
	typename std::remove_cv<Base>::type,
	typename std::remove_cv<Derived>::type>;

} // namespace detail
} // namespace pl


namespace pl {
namespace detail {

template <class Base, class Derived>
std::atomic<std::ptrdiff_t> inheritance_traits_impl<Base, Derived>::offset;

template<class Base, class Derived>
inline void inheritance_traits_impl<Base, Derived>::
set_offset(const Base* base_ptr, const Derived* derived_ptr) {
	offset = 
		reinterpret_cast<const unsigned char*>(derived_ptr) -
		reinterpret_cast<const unsigned char*>(base_ptr);
}

template<class Base, class Derived>
inline Derived* inheritance_traits_impl<Base, Derived>::downcast(Base* ptr) {
	return reinterpret_cast<Derived*>(
		reinterpret_cast<unsigned char*>(ptr) + offset);
}

template<class Base, class Derived>
inline const Derived* inheritance_traits_impl<Base, Derived>::downcast(const Base* ptr) {
	return reinterpret_cast<const Derived*>(
		reinterpret_cast<const unsigned char*>(ptr) + offset);
}

} // namespace detail
} // namespace pl

namespace pl {
namespace detail {

/// Casts Base* to Derived* using inheritance_traits and copies it
template<class Base, class Derived>
inline typename std::enable_if<
	std::is_copy_constructible<Derived>::value, Base*>::type
	clone(const Base* other) {
	const Derived* temp = inheritance_traits<Base, Derived>::downcast(other);

	Derived* rslt = new Derived(*temp);
	return static_cast<Base*>(rslt);
}

/// Placeholder for when Derived is not CopyConstructible. Throws on call.
template<class Base, class Derived>
inline typename std::enable_if<
	!std::is_copy_constructible<Derived>::value, Base*>::type // Note the !
	clone(const Base*) {
	return throw std::runtime_error(
		std::string("poly: poly<") +
		POLY_TYPE_NAME(Base) +
		"> is attempting to copy '" +
		POLY_TYPE_NAME(Derived) +
		"', which is not copy-constructible");
}

} // namespace detail

// class deep_copy =============================================================

template<class Base>
constexpr deep_copy<Base>::deep_copy() noexcept :
	clone_ptr(nullptr) {
}

template<class Base>
template<class Base2>
inline deep_copy<Base>::deep_copy(const deep_copy<Base2>& other) noexcept :
	clone_ptr(other.clone_ptr) {
}

template<class Base>
template<class Derived>
inline deep_copy<Base>::deep_copy(const Derived*) :
	clone_ptr(&detail::clone<Base, Derived>) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"deep_copy: Base is not base of Derived");
}

template<class Base>
inline Base* deep_copy<Base>::operator()(const Base* other) {
	return clone_ptr(other);
}

} // namespace pl

namespace pl {

/// Holds a function that casts object to derived before deleting
template <class Base>
class pmr_delete {
public:
	constexpr pmr_delete() noexcept;

	template <class Base2>
	pmr_delete(const pmr_delete<Base2>& other) noexcept;

	template <class Derived>
	pmr_delete(const Derived*);

	void operator()(const Base* other);

private:
	void (*destroy_ptr)(const Base*);

	template <class Base2>
	friend class pmr_delete;
};

} // namespace pl


namespace pl {
namespace detail {

/// Casts Base* to Derived* using inheritance_traits and deletes it
template<class Base, class Derived>
inline void	destroy(const Base* other) {
	const Derived* temp = inheritance_traits<Base, Derived>::downcast(other);
	delete temp;
}

} // namespace detail

// class pmr_delete ============================================================

template<class Base>
constexpr pmr_delete<Base>::pmr_delete() noexcept :
	destroy_ptr(nullptr) {
}

template<class Base>
template<class Base2>
inline pmr_delete<Base>::pmr_delete(const pmr_delete<Base2>& other) noexcept :
	destroy_ptr(other.destroy_ptr) {
}

template<class Base>
template<class Derived>
inline pmr_delete<Base>::pmr_delete(const Derived*) :
	destroy_ptr(&detail::destroy<Base, Derived>) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"pmr_delete: Base is not base of Derived");
}

template<class Base>
inline void pmr_delete<Base>::operator()(const Base* other) {
	destroy_ptr(other);
}

} // namespace pl

namespace pl {

template <class Base>
using unique = compound<no_copy,
	typename std::conditional<std::has_virtual_destructor<Base>::value,
	std::default_delete<Base>,
	pmr_delete<Base>>::type>;

template <class Base>
using deep = compound<deep_copy<Base>,
	typename std::conditional<std::has_virtual_destructor<Base>::value,
	std::default_delete<Base>,
	pmr_delete<Base>>::type>;

} // namespace pl

namespace pl {
namespace detail {

// is_less_cv and is_more_cv - compare constraints of two types
// const int is more cv than int
// const int is NOT more cv than volatile int
// "more" in this context means that, assuming 
// remove_cv_t<T> is same as remove_cv_t<U>,
// U can be upgraded to T without losing qualifiers.

template <class T, class U>
struct is_less_cv
	: std::integral_constant<bool,
	(std::is_const<T>::value <= std::is_const<U>::value &&
		std::is_volatile<T>::value <= std::is_volatile<U>::value)> {
};

template <class T, class U>
struct is_more_cv
	: std::integral_constant<bool,
	(std::is_const<T>::value >= std::is_const<U>::value &&
	std::is_volatile<T>::value >= std::is_volatile<U>::value)> {
};

// is_stronger_qualified - checks that a type T is a more strongly qualified
// version of U

template <class T, class U>
struct is_stronger_qualified
	: std::integral_constant<bool,
	(std::is_same<
		typename std::remove_cv<T>::type, 
		typename std::remove_cv<U>::type>::value &&
	is_more_cv<T, U>::value)> {
};

template <class T, class U>
struct is_weaker_qualified
	: std::integral_constant<bool,
	(std::is_same<
		typename std::remove_cv<T>::type,
		typename std::remove_cv<U>::type>::value &&
		is_less_cv<T, U>::value)> {
};

} // namespace detail
} // namespace pl

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
bool operator==(const poly<B, P>& x, std::nullptr_t) noexcept;

template <class B, class P>
bool operator==(std::nullptr_t, const poly<B, P>& x) noexcept;

template <class B, class P>
bool operator!=(const poly<B, P>& x, std::nullptr_t) noexcept;

template <class B, class P>
bool operator!=(std::nullptr_t, const poly<B, P>& x) noexcept;

template <class B, class P>
bool operator<(const poly<B, P>& x, std::nullptr_t);

template <class B, class P>
bool operator<(std::nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator<=(const poly<B, P>& x, std::nullptr_t);

template <class B, class P>
bool operator<=(std::nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator>(const poly<B, P>& x, std::nullptr_t);

template <class B, class P>
bool operator>(std::nullptr_t, const poly<B, P>& y);

template <class B, class P>
bool operator>=(const poly<B, P>& x, std::nullptr_t);

template <class B, class P>
bool operator>=(std::nullptr_t, const poly<B, P>& y);

} // namespace pl

namespace std {
template<class Base, class CopyDeletePolicy>
struct hash<pl::poly<Base, CopyDeletePolicy>>;
} // namespace std


namespace pl {

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

template<class Base, class CopyDeletePolicy>
inline constexpr poly<Base, CopyDeletePolicy>::poly() noexcept :
	data (nullptr) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::poly(const poly& other) :
	policy(other),
	data(nullptr) {
	if (other) data = this->clone(other.get());
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::poly(poly&& other) noexcept :
	policy(std::move(static_cast<policy&>(other))),
	data(other.release()) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(const poly& other) {
	policy::operator=(other);

	reset();
	if (other) data = this->clone(other.get());
	
	return *this;
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>&
poly<Base, CopyDeletePolicy>::operator=(poly&& other) noexcept {
	policy::operator=(std::move(other));

	reset();
	data = other.release();

	return *this;
}

template<class Base, class CopyDeletePolicy>
inline constexpr poly<Base, CopyDeletePolicy>::poly(std::nullptr_t) noexcept :
	data(nullptr) {
}

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(std::nullptr_t) noexcept {
	data = nullptr;

	return *this;
}

// Converting constructors -------------------------------------------------

template<class Base, class CopyDeletePolicy>
template<class Base2, class CopyDeletePolicy2, class>
inline poly<Base, CopyDeletePolicy>::
poly(const poly<Base2, CopyDeletePolicy2>& other) :
	policy(static_cast<CopyDeletePolicy2>(other)),
	data(nullptr) {
	if (other) data = this->clone(other.get());
}

template<class Base, class CopyDeletePolicy>
template<class Base2, class CopyDeletePolicy2, class>
inline poly<Base, CopyDeletePolicy>::
poly(poly<Base2, CopyDeletePolicy2>&& other) noexcept :
	policy(std::move(static_cast<CopyDeletePolicy2>(other))),
	data(other.release()) {
}

// From a pointer ----------------------------------------------------------

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline poly<Base, CopyDeletePolicy>::poly(Derived* obj) :
	policy(obj),
	data(obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline poly<Base, CopyDeletePolicy>& 
poly<Base, CopyDeletePolicy>::operator=(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	reset(obj);
	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

// Destruction -------------------------------------------------------------

template<class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy>::~poly() {
	if (data) this->destroy(get());
}

// Observers ===============================================================

template<class Base, class CopyDeletePolicy>
template<class T>
constexpr bool poly<Base, CopyDeletePolicy>::is() const noexcept {
	return get() != nullptr && typeid(*get()) == typeid(T);
}

template<class Base, class CopyDeletePolicy>
constexpr poly<Base, CopyDeletePolicy>::operator bool() const noexcept {
	return static_cast<bool>(data);
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::release() noexcept {
	Base* rslt = data;
	data = nullptr;

	return rslt;
}

template<class Base, class CopyDeletePolicy>
inline void poly<Base, CopyDeletePolicy>::reset(std::nullptr_t) noexcept {
	if (data) {
		destroy(data);
		data = nullptr;
	}
}

template<class Base, class CopyDeletePolicy>
template<class Derived>
inline void poly<Base, CopyDeletePolicy>::reset(Derived* obj) {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly: poly can only be built using types, derived from Base");
	if (typeid(*obj) != typeid(Derived))
		throw std::runtime_error(
			std::string("poly: passed pointer of type '") +
			POLY_TYPE_NAME(Derived) +
			"' must hold an object of exacly that type (not '" +
			POLY_TYPE_NAME(*obj));

	if (data) destroy(data);
	data = obj;
	detail::inheritance_traits<Base, Derived>::set_offset(get(), obj);
}

template<class Base, class CopyDeletePolicy>
inline Base& poly<Base, CopyDeletePolicy>::operator*() const {
	return *data;
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::operator->() const noexcept {
	return data;
}

template<class Base, class CopyDeletePolicy>
inline Base* poly<Base, CopyDeletePolicy>::get() const noexcept {
	return data;
}

// Member access ===========================================================

template<class Base, class CopyDeletePolicy>
template<class T>
T* poly<Base, CopyDeletePolicy>::as() const noexcept {
	static_assert(std::is_base_of<Base, T>::value,
		"poly: cannot interpret as class not derived from Base");

	if (is<T>()) {
		return detail::inheritance_traits<Base, T>::downcast(get());
	}
	return nullptr;
}

// Non-member functions ========================================================

template<class PolyType, class Derived, class... Args>
PolyType make(Args&&... args) {
	return PolyType(new Derived(std::forward<Args>(args)...));
}

template<class PolyType, class Derived, class OldBase, class CopyDeletePolicy>
PolyType transform(const poly<OldBase, CopyDeletePolicy>& other) {
	if (other) {
		return PolyType(
			new Derived(*other.template as<Derived>()));
	}
	return PolyType();
}

template<class PolyType, class Derived, class OldBase, class CopyDeletePolicy>
PolyType transform(poly<OldBase, CopyDeletePolicy>&& other) {
	Derived* temp = other.template as<Derived>();
	other.release();
	
	return PolyType(temp);
}

// Comparison operators ========================================================

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

template<class B1, class C1, class B2, class C2>
	bool operator==(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return x.get() == y.get();
}

template<class B1, class C1, class B2, class C2>
	bool operator!=(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	return !(x == y);
}

template<class B1, class C1, class B2, class C2>
	bool operator<(const poly<B1, C1>& x, const poly<B2, C2>& y) {
	using common = std::common_type<
		typename poly<B1, C1>::base_type*, 
		typename poly<B2, C2>::base_type*>;
	static_assert(detail::defines_type<common>::value, 
		"comparison: these poly types cannot be compared: internal objects must have the same class, or be convertible to one base class");

	return std::less<typename common::type>()(x.get(), y.get());
}

template<class B1, class P1, class B2, class P2>
	bool operator<=(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return !(y < x);
}

template<class B1, class P1, class B2, class P2>
	bool operator>(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return y < x;
}

template<class B1, class P1, class B2, class P2>
	bool operator>=(const poly<B1, P1>& x, const poly<B2, P2>& y) {
	return !(x < y);
}

template <class B, class P>
bool operator==(const poly<B, P>& x, std::nullptr_t) noexcept {
	return !(x != nullptr);
}

template <class B, class P>
bool operator==(std::nullptr_t, const poly<B, P>& x) noexcept {
	return !(nullptr != x);
}

template <class B, class P>
bool operator!=(const poly<B, P>& x, std::nullptr_t) noexcept {
	return static_cast<bool>(x);
}

template <class B, class P>
bool operator!=(std::nullptr_t, const poly<B, P>& x) noexcept {
	return static_cast<bool>(x);
}

template <class B, class P>
bool operator<(const poly<B, P>& x, std::nullptr_t) {
	return std::less<typename poly<B, P>::base_type*>()(x.get(), nullptr);
}

template <class B, class P>
bool operator<(std::nullptr_t, const poly<B, P>& y) {
	return std::less<typename poly<B, P>::base_type*>()(nullptr, y.get());
}

template <class B, class P>
bool operator<=(const poly<B, P>& x, std::nullptr_t) {
	return !(nullptr < x);
}

template <class B, class P>
bool operator<=(std::nullptr_t, const poly<B, P>& y) {
	return !(y < nullptr);
}

template <class B, class P>
bool operator>(const poly<B, P>& x, std::nullptr_t) {
	return nullptr < x;
}

template <class B, class P>
bool operator>(std::nullptr_t, const poly<B, P>& y) {
	return y < nullptr;
}

template <class B, class P>
bool operator>=(const poly<B, P>& x, std::nullptr_t) {
	return !(x < nullptr);
}

template <class B, class P>
bool operator>=(std::nullptr_t, const poly<B, P>& y) {
	return !(nullptr < y);
}

} // namespace pl

namespace std {

template<class Base, class CopyDeletePolicy>
struct hash<pl::poly<Base, CopyDeletePolicy>> {
	size_t operator()(const pl::poly<Base, CopyDeletePolicy>& x) const {
		return hash<typename pl::poly<Base, CopyDeletePolicy>::base_type*>()(x.get());
	}
};

} // namespace std

namespace pl {

template <class Base, class CopyDeletePolicy = deep<Base>>
class factory {
public:
	template <class Derived>
	void insert();
	
	std::vector<std::string> list() const;
	poly<Base, CopyDeletePolicy> make(const std::string& name) const;

private:
	std::vector<std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>> make_funcs;
};

} // namespace pl


namespace pl {

template <class Base, class CopyDeletePolicy>
template<class Derived>
void factory<Base, CopyDeletePolicy>::insert() {
	static_assert(std::is_base_of<Base, Derived>::value,
		"factory: factory can only build types, derived from Base");
	static_assert(std::is_default_constructible<Derived>::value,
		"factory: factory can only build default-constructible types");

	auto name = POLY_TYPE_NAME(Derived);

	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>& lhs,
		const decltype(name)& rhs) {
		return lhs.first < rhs;
	});

	if (it == make_funcs.cend() || it->first != name) {
		make_funcs.insert(it, 
			std::make_pair(name, &pl::make<poly<Base, CopyDeletePolicy>, Derived>));
	}
}

template <class Base, class CopyDeletePolicy>
inline std::vector<std::string> factory<Base, CopyDeletePolicy>::list() const {
	std::vector<std::string> rslt;
	rslt.reserve(make_funcs.size());

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template <class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy> 
factory<Base, CopyDeletePolicy>::make(const std::string& name) const {
	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>& lhs,
		const decltype(name)& rhs) {
		return lhs.first < rhs;
	});

	if (it != make_funcs.cend() && it->first == name) {
		return it->second();
	}

	throw std::invalid_argument(
		std::string("factory: ") +
		name +
		" is not registered in this factory");
}

} // namespace pl
