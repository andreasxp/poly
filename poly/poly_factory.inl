#pragma once
#include "poly_factory.hpp"

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

///Namespace for internal-use-only functions
namespace detail {
template <class Base, class Derived>
constexpr typename
std::enable_if<std::is_base_of<Base, Derived>::value &&
	std::is_default_constructible<Derived>::value, poly<Base>>::type
	make_impl() {
	return poly<Base>(new Derived());
}

} // namespace detail

template<class Base>
template<class Derived>
constexpr typename std::enable_if<std::is_base_of<Base, Derived>::value && std::is_default_constructible<Derived>::value, void>::type
factory<Base>::add() {
	make_funcs[POLY_TYPE_NAME(Derived)] = &detail::make_impl<Base, Derived>;
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