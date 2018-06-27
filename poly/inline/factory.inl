#pragma once
#include "factory.hpp"
#include <algorithm>

namespace pl {

template <class Base, class CopyDeletePolicy>
template<class Derived>
void factory<Base, CopyDeletePolicy>::insert() {
	static_assert(std::is_base_of<Base, Derived>::value,
		"factory: factory can only build types, derived from Base");
	static_assert(std::is_default_constructible<Derived>::value,
		"factory: factory can only build default-constructible types");

	std::string name(POLY_TYPE_NAME(Derived));

	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>& lhs,
		const std::string& rhs) {
		return lhs.first < rhs;
	});

	if (it == make_funcs.cend() || it->first != name) {
		make_funcs.insert(it, 
			std::make_pair(name, &pl::make<poly<Base, CopyDeletePolicy>, Derived>));
	}
}

template <class Base, class CopyDeletePolicy>
inline std::vector<std::string> factory<Base, CopyDeletePolicy>::list() const {
	std::vector<std::string> rslt;
	rslt.reserve(make_funcs.size());

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template <class Base, class CopyDeletePolicy>
inline poly<Base, CopyDeletePolicy> 
factory<Base, CopyDeletePolicy>::make(const std::string& name) const {
	auto it = std::lower_bound(make_funcs.cbegin(), make_funcs.cend(), name,
		[](
		const std::pair<std::string, poly<Base, CopyDeletePolicy>(*)()>& lhs,
		const std::string& rhs) {
		return lhs.first < rhs;
	});

	if (it != make_funcs.cend() && it->first == name) {
		return it->second();
	}

	throw std::invalid_argument(
		std::string("factory: ") +
		name +
		" is not registered in this factory");
}

} // namespace pl