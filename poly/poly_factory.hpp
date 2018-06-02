#pragma once

#include <unordered_map> // unordered_map
#include <vector>        // vector
#include <string>        // string
#include <type_traits>   // enable_if, is_base_of, is_copy_constructible,
                         // is_default_constructible
#include "poly.hpp"

namespace zhukov {

template <class Base>
class factory {
	static_assert(std::is_polymorphic<Base>::value,
		"poly_factory: factory can only be used with polymorphic types");
public:
	template <class Derived>
	void add();
	
	std::vector<std::string> list() const;
	poly<Base> make(const std::string& name) const;

private:
	std::unordered_map<std::string, poly<Base>(*)()> make_funcs;
};

} // namespace zhukov

#include "poly_factory.inl"