#pragma once
#include "compound.hpp"
#include "copy_policies.hpp"
#include "delete_policies.hpp"

namespace pl {
// Default poliices, provided with the library.
// These policies use pl::default_delete, which requires the class to have
// a virtual destructor, thus preventing memory leaks.

template <class Base>
using unique = compound<no_copy, default_delete<Base>>;

template <class Base>
using deep = compound<deep_copy<Base>, default_delete<Base>>;

} // namespace pl