#pragma once
#include <type_traits>

namespace pl {

// Empty class. Provides no operator(), so a policy is not copyable.
class no_copy {
	/// Dummy operator. Static asserts if called.
	template <class T>
	T* operator()(const T*);
};

// Stores a pointer to a function that copies the Base pointer
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

#include "copy_policies.inl"