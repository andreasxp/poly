/*
Copyright (c) 2017 Andrey Zhukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef POLY_FACTORY_HPP
#define POLY_FACTORY_HPP

#include <unordered_map> // std::unordered_map
#include <vector> // std::vector
#include "poly.hpp" // poly_t, poly

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

///Namespace for internal-use-only functions
namespace detail {

/*!
\brief   Makes a poly_t object, holding default-ctructed derived_t object
\details This function is only enabled if poly_t::element_type is base of
         derived_t and derived_t can be default-constructed.
\tparam  poly_t poly wrapper to insert object in
\tparam  derived_t  type of which a default-ctor will be invoked
*/
template <typename poly_t, typename derived_t>
constexpr typename std::enable_if<std::is_base_of<typename poly_t::element_type, derived_t>::value && std::is_default_constructible<derived_t>::value, poly_t>::type
make_impl() {
	return poly_t(derived_t());
}

} // namespace detail

/*!
\brief   Class that can make a poly_t using a string with type's name
\details This is poor man's reflection: register a class, and now you can
         make poly<> from just passing string("type").
\tparam  poly_t what to store built objects in
*/
template <typename poly_t>
class basic_factory {
private:
	std::unordered_map<std::string, poly_t(*)()> make_funcs;
public:
	/*!
	\brief   Add a class to factory, so it can be created by a string
	\tparam  derived_t Type to register. Must be derived from 
	                   poly_t::element_type.
	*/
	template <typename derived_t>
	constexpr typename std::enable_if<std::is_base_of<typename poly_t::element_type, derived_t>::value && std::is_default_constructible<derived_t>::value, void>::type
		add();

	///Get a vector of all names of classes that are registered
	std::vector<std::string> list() const;

	/*!
	\brief   Make an object from string containing its name
	\param   name string that contains object's type
	*/
	poly_t make(const std::string name) const;
};

template<typename poly_t>
template<typename derived_t>
constexpr typename std::enable_if<std::is_base_of<typename poly_t::element_type, derived_t>::value && std::is_default_constructible<derived_t>::value, void>::type
basic_factory<poly_t>::add() {
	make_funcs[typename poly_t::rtti_index(typeid(derived_t)).name()] = &detail::make_impl<poly_t, derived_t>;
}

template<typename poly_t>
inline std::vector<std::string> basic_factory<poly_t>::list() const {
	std::vector<std::string> rslt;

	for (auto&& it : make_funcs) {
		rslt.push_back(it.first);
	}

	return rslt;
}

template<typename poly_t>
inline poly_t basic_factory<poly_t>::make(const std::string name) const {
	return make_funcs.at(name)();
}

template <typename base_t>
using factory = basic_factory<poly<base_t>>;

} // namespace zhukov

#endif // POLY_FACTORY_HPP