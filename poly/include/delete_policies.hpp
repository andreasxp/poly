#pragma once
#include <memory> // default_delete

namespace pl {

/// Adds a virtual destructor check to std::default_delete
template <class Base>
class default_delete : public std::default_delete<Base> {
public:
	using std::default_delete<Base>::default_delete; // Inherit constructors

	void operator()(const Base* ptr) const;
};

/// Holds a function that casts object to derived before deleting
template <class Base>
class pmr_delete {
public:
	constexpr pmr_delete() noexcept;

	template <class Base2>
	pmr_delete(const pmr_delete<Base2>& other) noexcept;

	template <class Derived>
	pmr_delete(const Derived*);

	void operator()(const Base* ptr);

private:
	void (*destroy_ptr)(const Base*);

	template <class Base2>
	friend class pmr_delete;
};

} // namespace pl

#include "delete_policies.inl"