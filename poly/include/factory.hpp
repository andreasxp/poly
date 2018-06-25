#pragma once

#include <vector>        // vector
#include <string>        // string
#include <type_traits>   // is_base_of, is_default_constructible
#include "poly.hpp"

namespace pl {

template <class Base, class CopyDeletePolicy = deep<Base>>
class factory {
public:
	template <class Derived>
	void insert();
	
	std::vector<std::string> list() const;
	poly<Base, CopyDeletePolicy> make(const std::string& name) const;

private:
	std::vector<std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>> make_funcs;
};

} // namespace pl

#include "factory.inl"