#pragma once
#include "poly_factory.hpp"
#include <algorithm>

namespace zhukov {

template <class Base, template<class> class CopyPolicy>
template<class Derived>
void factory<Base, CopyPolicy>::insert() {
	static_assert(std::is_base_of<Base, Derived>::value,
		"poly_factory: factory can only build types, derived from Base");
	static_assert(std::is_default_constructible<Derived>::value,
		"poly_factory: factory can only build default-constructible types");

	auto name = POLY_TYPE_NAME(Derived);

	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyPolicy>(*)()>& lhs,
		const decltype(name)& rhs) {
		return lhs.first < rhs;
	});

	if (it == make_funcs.cend() || it->first != name) {
		make_funcs.insert(it, 
			std::make_pair(name, &make_poly<Base, Derived, CopyPolicy>));
	}
}

template <class Base, template<class> class CopyPolicy>
inline std::vector<std::string> factory<Base, CopyPolicy>::list() const {
	std::vector<std::string> rslt;
	rslt.reserve(make_funcs.size());

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template <class Base, template<class> class CopyPolicy>
inline poly<Base, CopyPolicy> 
factory<Base, CopyPolicy>::make(const std::string& name) const {
	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyPolicy>(*)()>& lhs,
		const decltype(name)& rhs) {
		return lhs.first < rhs;
	});

	if (it != make_funcs.cend() && it->first == name) {
		return it->second();
	}

	throw std::invalid_argument(
		std::string("poly_factory: ") +
		name +
		" is not registered in this factory");
}

} // namespace zhukov