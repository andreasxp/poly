#pragma once
#include "delete_policies.hpp"
#include "inheritance_traits.hpp"

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