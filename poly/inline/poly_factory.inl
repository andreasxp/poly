#pragma once
#include "poly_factory.hpp"

namespace zhukov {

template<class Base>
template<class Derived>
void factory<Base>::add() {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly_factory: factory can only build types, derived from Base");
	static_assert(std::is_default_constructible<Derived>::value,
		"poly_factory: factory can only build default-constructible types");

	make_funcs[POLY_TYPE_NAME(Derived)] = &make_poly<Base, Derived>;
}

template<class Base>
inline std::vector<std::string> factory<Base>::list() const {
	std::vector<std::string> rslt;

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template<class Base>
inline poly<Base> factory<Base>::make(const std::string& name) const {
	return make_funcs.at(name)();
}

} // namespace zhukov