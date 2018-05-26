#pragma once
#include "poly_factory.hpp"

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

///Namespace for internal-use-only functions
namespace detail {
template <class base_t, class derived_t>
constexpr typename
std::enable_if<std::is_base_of<base_t, derived_t>::value &&
	std::is_default_constructible<derived_t>::value, poly<base_t>>::type
	make_impl() {
	return poly<base_t>(new derived_t());
}

} // namespace detail

template<class base_t>
template<class derived_t>
constexpr typename std::enable_if<std::is_base_of<base_t, derived_t>::value && std::is_default_constructible<derived_t>::value, void>::type
factory<base_t>::add() {
	make_funcs[POLY_TYPE_NAME(derived_t)] = &detail::make_impl<base_t, derived_t>;
}

template<class base_t>
inline std::vector<std::string> factory<base_t>::list() const {
	std::vector<std::string> rslt;

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template<class base_t>
inline poly<base_t> factory<base_t>::make(const std::string& name) const {
	return make_funcs.at(name)();
}

} // namespace zhukov