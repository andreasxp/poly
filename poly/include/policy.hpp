#pragma once
#include <type_traits> // conditional, has_virtual_destructor
#include "compound.hpp"
#include "copy_policies.hpp"
#include "delete_policies.hpp"

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