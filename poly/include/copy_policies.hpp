#pragma once
#include <type_traits>

namespace pl {

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
	typename std::remove_const<Base>::type* (*clone_ptr)(const Base*);

	template <class Base2>
	friend class deep_copy;
};

} // namespace pl

#include "copy_policies.inl"