#pragma once
#include <type_traits> // is_constructible

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
	auto clone(const T* ptr) -> decltype(std::declval<Cloner>()(ptr));

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

#include "compound.inl"