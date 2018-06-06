#pragma once
#include <type_traits>

namespace zhukov {

template <class Base>
class no_copy {
public:
	// Dummy constructor. No action is performed
	constexpr no_copy() noexcept = default;

	template <class Derived>
	no_copy(const Derived*);
};

template <class Base>
class deep_copy {
public:
	constexpr deep_copy() noexcept;

	template <class Derived>
	deep_copy(const Derived*);

	Base* clone(const Base* other);

private:
	Base* (*copy_construct)(const Base*);
};

} // namespace zhukov

#include "copy_policy.inl"