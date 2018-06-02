#pragma once

#include <unordered_map> // unordered_map
#include <vector>        // vector
#include <string>        // string
#include <type_traits>   // enable_if, is_base_of, is_copy_constructible,
                         // is_default_constructible
#include "poly.hpp"

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

template <class Base>
class factory {
	static_assert(std::is_polymorphic<Base>::value,
		"poly_factory: factory can only be used with polymorphic types");
public:
	template <class Derived>
	constexpr typename std::enable_if<
		std::is_base_of<Base, Derived>::value && 
		std::is_default_constructible<Derived>::value, void>::type
		add();
	
	std::vector<std::string> list() const;
	poly<Base> make(const std::string& name) const;

private:
	std::unordered_map<std::string, poly<Base>(*)()> make_funcs;
};

} // namespace zhukov

#include "poly_factory.inl"