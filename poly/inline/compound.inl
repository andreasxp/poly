#pragma once
#include "compound.hpp"

namespace pl {

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::
compound(const compound<Cloner2, Deleter2>& other) :
	Cloner(static_cast<Cloner2&>(other)),
	Deleter(static_cast<Deleter2&>(other)) {
}

template<class Cloner, class Deleter>
template<class Cloner2, class Deleter2>
inline compound<Cloner, Deleter>::
compound(compound<Cloner2, Deleter2>&& other) :
	Cloner(std::move(static_cast<Cloner2&>(other))),
	Deleter(std::move(static_cast<Deleter2&>(other))) {
}

template<class Cloner, class Deleter>
template<class T>
	inline compound<Cloner, Deleter>::compound(const T* ptr) : 
	compound(ptr, 
		std::is_constructible<Cloner,  const T*>(),
		std::is_constructible<Deleter, const T*>()) {
}

template<class Cloner, class Deleter>
template<class T>
inline auto compound<Cloner, Deleter>::clone(const T* ptr)
->decltype(std::declval<Cloner>()(ptr)) {
	return Cloner::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline void compound<Cloner, Deleter>::destroy(T* ptr) {
	Deleter::operator()(ptr);
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T*, std::false_type, std::false_type) :
	Cloner(),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::false_type, std::true_type) :
	Cloner(),
	Deleter(ptr) {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::true_type, std::false_type) :
	Cloner(ptr),
	Deleter() {
}

template<class Cloner, class Deleter>
template<class T>
inline compound<Cloner, Deleter>::
compound(const T* ptr, std::true_type, std::true_type) :
	Cloner(ptr),
	Deleter(ptr) {
}

} // namespace pl