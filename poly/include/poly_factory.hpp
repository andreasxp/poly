#pragma once

#include <vector>        // vector
#include <string>        // string
#include <type_traits>   // is_base_of, is_default_constructible
#include "poly.hpp"

namespace zhukov {

template <class Base, template<class> class CopyPolicy = deep_copy>
class factory {
public:
	template <class Derived>
	void insert();
	
	std::vector<std::string> list() const;
	poly<Base, CopyPolicy> make(const std::string& name) const;

private:
	std::vector<std::pair<std::string, poly<Base, CopyPolicy>(*)()>> make_funcs;
};

} // namespace zhukov

#include "poly_factory.inl"