#pragma once
#include "compound.hpp"

namespace pl {

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::compound(const compound<Cloner2, Deleter2>& other) :
	Cloner(static_cast<Cloner2&>(other)),
	Deleter(static_cast<Deleter2&>(other)) {
}

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::compound(compound<Cloner2, Deleter2>&& other) :
	Cloner(std::move(static_cast<Cloner2&>(other))),
	Deleter(std::move(static_cast<Deleter2&>(other))) {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	!std::is_constructible<Cloner, const T*>::value &&
	!std::is_constructible<Deleter, const T*>::value,
	detail::tag<0>*>::type>
	inline compound<Cloner, Deleter>::compound(const T*) :
	Cloner(),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	!std::is_constructible<Cloner, const T*>::value &&
	std::is_constructible<Deleter, const T*>::value,
	detail::tag<1>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	std::is_constructible<Cloner, const T*>::value &&
	!std::is_constructible<Deleter, const T*>::value,
	detail::tag<2>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(ptr),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T,
	typename std::enable_if<
	std::is_constructible<Cloner, const T*>::value &&
	std::is_constructible<Deleter, const T*>::value,
	detail::tag<3>*>::type>
	inline compound<Cloner, Deleter>::compound(const T* ptr) :
	Cloner(ptr),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T>
inline auto compound<Cloner, Deleter>::clone(const T* ptr)
->decltype(Cloner::operator()(ptr)) {
	return Cloner::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline void compound<Cloner, Deleter>::destroy(T* ptr) {
	Deleter::operator()(ptr);
}

} // namespace pl