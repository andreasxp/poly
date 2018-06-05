#pragma once
#include <type_traits>

namespace zhukov {

class no_copy {
public:
	// Dummy constructor. No action is performed
	template <class Derived>
	no_copy(const Derived*);
};

template <class Base>
class deep_copy {
	static_assert(std::is_polymorphic<Base>::value,
		"deep_copy: Base is not polymorphic");
public:
	deep_copy();

	template <class Derived>
	deep_copy(const Derived*);

	Base* copy(const Base* other);

private:
	Base* (*copy_construct)(const Base*);
};

} // namespace zhukov

#include "copy_policy.inl"