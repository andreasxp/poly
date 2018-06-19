#pragma once
#include <memory> // default_delete

namespace pl {

template <class Base>
class pmr_delete {
public:
	constexpr pmr_delete() noexcept;

	template <class Base2>
	pmr_delete(const pmr_delete<Base2>& other) noexcept;

	template <class Derived>
	pmr_delete(const Derived*);

	void operator()(const Base* other);

private:
	void (*destroy_ptr)(const Base*);

	template <class Base2>
	friend class pmr_delete;
};

} // namespace pl

#include "delete_policies.inl"