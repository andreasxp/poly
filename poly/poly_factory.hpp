#pragma once

#include <unordered_map> // unordered_map
#include <vector>        // vector
#include <string>        // string
#include <type_traits>   // enable_if, is_base_of, is_copy_constructible,
                         // is_default_constructible
#include "poly.hpp"

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

template <class base_t>
class factory {
public:
	template <class derived_t>
	constexpr typename std::enable_if<
		std::is_base_of<base_t, derived_t>::value && 
		std::is_default_constructible<derived_t>::value, void>::type
		add();
	
	std::vector<std::string> list() const;
	poly<base_t> make(const std::string& name) const;

private:
	std::unordered_map<std::string, poly<base_t>(*)()> make_funcs;
};

} // namespace zhukov

#include "poly_factory.inl"