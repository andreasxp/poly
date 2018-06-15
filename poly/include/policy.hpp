#pragma once
#include <type_traits>
#include <memory>      // default_delete

#ifdef _MSC_VER
#define POLY_MULTIPLE_EMPTY_BASES __declspec(empty_bases)
#else
#define POLY_MULTIPLE_EMPTY_BASES
#endif

namespace pl {
namespace detail {

template <size_t>
class tag {};

} // namespace detail

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
	template <class T,
		typename std::enable_if<
		!std::is_constructible<Cloner, const T*>::value &&
		!std::is_constructible<Deleter, const T*>::value,
		detail::tag<0>*>::type = nullptr>
	compound(const T*);

	template <class T,
		typename std::enable_if<
		!std::is_constructible<Cloner, const T*>::value &&
		 std::is_constructible<Deleter, const T*>::value,
		detail::tag<1>*>::type = nullptr>
	compound(const T* ptr);

	template <class T,
		typename std::enable_if<
		 std::is_constructible<Cloner, const T*>::value &&
		!std::is_constructible<Deleter, const T*>::value,
		detail::tag<2>*>::type = nullptr>
	compound(const T* ptr);

	template <class T,
		typename std::enable_if<
		std::is_constructible<Cloner, const T*>::value &&
		std::is_constructible<Deleter, const T*>::value,
		detail::tag<3>*>::type = nullptr>
	compound(const T* ptr);

	// Operations ==============================================================
	template <class T>
	auto clone(const T* ptr) -> decltype(Cloner::operator()(ptr));

	template <class T>
	void destroy(T* ptr);

	// Friends =================================================================
	template <class Cloner2, class Deleter2>
	friend class compound;
};

class no_copy {};

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
	typename std::remove_const<Base>::type* (*copy_construct)(const Base*);

	template <class Base2>
	friend class deep_copy;
};

template <class Base>
using unique = compound<no_copy, std::default_delete<Base>>;

template <class Base>
using deep = compound<deep_copy<Base>, std::default_delete<Base>>;

} // namespace pl

#include "policy.inl"