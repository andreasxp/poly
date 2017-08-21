#pragma once
#include <unordered_map>
#include <vector>
#include "poly.hpp"

namespace zhukov {

template <typename Base>
poly<Base> make(const std::string name);

template <typename Base>
poly<Base> make_s(const std::string name);

template <typename Base>
class factory {
private:
	static std::unordered_map<std::string, poly<Base>(*)()>& get_make_funcs() {
		static std::unordered_map<std::string, poly<Base>(*)()> make_funcs;
		return make_funcs;
	}
public:
	template <typename Derived>
	static constexpr typename std::enable_if<std::is_base_of<Base, Derived>::value && std::is_default_constructible<Derived>::value, void>::type
		add();

	static const std::vector<std::string> list();

	friend typename poly<Base> make<Base>(const std::string name);
	friend typename poly<Base> make_s<Base>(const std::string name);
};

template <typename Base, typename Derived>
constexpr typename std::enable_if<std::is_base_of<Base, Derived>::value && std::is_default_constructible<Derived>::value, poly<Base>>::type
make() {
	return poly<Base>(Derived());
}

template <typename Base>
poly<Base> make(const std::string name) {
	return factory<Base>::get_make_funcs()[name]();
}

template <typename Base>
poly<Base> make_s(const std::string name) {
	return factory<Base>::get_make_funcs().at(name)();
}

template<typename Base>
template<typename Derived>
inline constexpr typename std::enable_if<std::is_base_of<Base, Derived>::value && std::is_default_constructible<Derived>::value, void>::type
factory<Base>::add() {
	get_make_funcs()[type_index_t(typeid(Derived)).name()] = &make<Base, Derived>;
}

template<typename Base>
inline const std::vector<std::string> factory<Base>::list() {
	std::vector<std::string> rslt;

	for (auto&& it : get_make_funcs()) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template <typename Base, typename Derived>
struct add_polyref_ctor {
	add_polyref_ctor() {
		factory<Base>::template add<Derived>();
	}
};

#define POLYREF_CTOR(Base, Derived) \
namespace { \
static const zhukov::add_polyref_ctor<Base, Derived> poly_ ## Base ## _from_ ## Derived ## _; \
} // namespace

} // namespace zhukov