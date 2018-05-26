#pragma once
#include "poly.hpp"

namespace zhukov {
namespace detail {
// Black magic ahead

template <class T>
typename std::enable_if<
	std::is_copy_constructible<T>::value, void*>::type
	clone(const void* other) {
	return new T(*static_cast<const T*>(other));
}

template <class T>
typename std::enable_if<
	!std::is_copy_constructible<T>::value, void*>::type //Note the !
	clone(const void* /*other (unused)*/) {
	return throw std::runtime_error(
		"attempting to copy-construct a non-copyable object");
}

template <class T, class U>
T try_downcast(U ptr) {
	// Runtime check if the pointer can be down- or side-casted to a new base
	if (ptr) {
		T rslt = dynamic_cast<T>(ptr);
		if (!rslt) throw std::runtime_error(
			std::string("unable to cast from '")
			+ POLY_TYPE_NAME(U)
			+ "' to '"
			+ POLY_TYPE_NAME(T) + "'");
		return rslt;
	}
	return nullptr;
}

} // namespace detail

// =============================================================================
// Definitions =================================================================

// Construction ============================================================
// Default, copy, move -----------------------------------------------------

/// Check if a type is polymorphic (and so can be used in poly)
#define POLY_ASSERT_POLYMORPHIC(type) \
static_assert(std::is_polymorphic<type>::value, \
	"poly can only be used with polymorphic types");

template<class base_t>
constexpr poly<base_t>::poly() :
	base_ptr(nullptr),
	copy_construct(nullptr) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

template<class base_t>
constexpr poly<base_t>::poly(const poly& other) :
	base_ptr(static_cast<base_t*>(other.copy_construct(other.base_ptr.get()))),
	copy_construct(other.copy_construct) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

template<class base_t>
inline poly<base_t> & poly<base_t>::operator=(const poly& rhs) {
	base_ptr.reset(static_cast<base_t*>(rhs.copy_construct(rhs.base_ptr.get())));
	copy_construct = rhs.copy_construct;
	return *this;
}

// From a pointer ----------------------------------------------------------
template<class base_t>
template<class derived_t, class>
constexpr poly<base_t>::poly(derived_t* obj) :
	base_ptr(obj),
	copy_construct(&detail::clone<derived_t>) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

// Casting from poly to poly -----------------------------------------------
template<class base_t>
template<class base2_t, typename std::enable_if<
	std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	constexpr poly<base_t>::poly(const poly<base2_t>& other) :
	base_ptr(static_cast<base_t*>(other.copy_construct(other.base_ptr.get()))),
	copy_construct(other.copy_construct) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	!std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t>::poly(const poly<base2_t>& other) :
	poly() {
	POLY_ASSERT_POLYMORPHIC(base_t);

	auto ptr = detail::try_downcast<base_t*>(other.base_ptr.get());
	if (ptr) base_ptr.reset(static_cast<base_t*>(other.copy_construct(ptr)));
	else base_ptr.reset();

	copy_construct = other.copy_construct;
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	constexpr poly<base_t>::poly(poly<base2_t>&& other) :
	base_ptr(std::move(other.base_ptr)),
	copy_construct(std::move(other.copy_construct)) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	!std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t>::poly(poly<base2_t>&& other) :
	base_ptr(detail::try_downcast<base_t*>(other.base_ptr.release())),
	copy_construct(std::move(other.copy_construct)) {
	POLY_ASSERT_POLYMORPHIC(base_t);
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t> & poly<base_t>::operator=(const poly<base2_t>& rhs) {
	base_ptr = static_cast<base_t*>(rhs.copy_construct(rhs.base_ptr.get()));
	copy_construct = rhs.copy_construct;
	return *this;
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	!std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t> & poly<base_t>::operator=(const poly<base2_t>& rhs) {
	auto ptr = detail::try_downcast<base_t*>(rhs.base_ptr.get());
	if (ptr) base_ptr.reset(static_cast<base_t*>(rhs.copy_construct(ptr)));
	else base_ptr.reset();

	copy_construct = rhs.copy_construct;
	return *this;
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t> & poly<base_t>::operator=(poly<base2_t>&& rhs) {
	base_ptr = std::move(rhs.base_ptr);
	copy_construct = std::move(rhs.copy_construct);
	return *this;
}

template<class base_t>
template<class base2_t, typename std::enable_if<
	!std::is_base_of<base_t, base2_t>::value, base2_t>::type*>
	inline poly<base_t> & poly<base_t>::operator=(poly<base2_t>&& rhs) {
	base_ptr.reset(detail::try_downcast<base_t*>(rhs.base_ptr.release()));
	copy_construct = std::move(rhs.copy_construct);
	return *this;
}

#undef POLY_ASSERT_POLYMORPHIC

// Observers ===============================================================
template<class base_t>
template<class T>
constexpr bool poly<base_t>::is() const {
	return base_ptr.get() != nullptr && typeid(*base_ptr) == typeid(T);
}

// Member access ===========================================================
template<class base_t>
inline base_t & poly<base_t>::operator*() {
	return *base_ptr;
}

template<class base_t>
constexpr base_t & poly<base_t>::operator*() const {
	return *base_ptr;
}

template<class base_t>
inline base_t * poly<base_t>::operator->() {
	return base_ptr.get();
}

template<class base_t>
constexpr base_t * poly<base_t>::operator->() const {
	return base_ptr.get();
}

template<class base_t>
inline base_t * poly<base_t>::get() {
	return base_ptr.get();
}

template<class base_t>
constexpr base_t * poly<base_t>::get() const {
	return base_ptr.get();
}

template<class base_t>
template<class T>
typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() {

	if (is<T>()) {
		return static_cast<T&>(*base_ptr);
	}
	throw std::bad_cast();
	//I'm really not sure why code below does not do the same thing
	/*return is<T>()
	? static_cast<T&>(*base_ptr)
	: throw std::bad_cast();*/
}

template<class base_t>
template<class T>
constexpr typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() const {
	return is<T>()
		? static_cast<T&>(*base_ptr)
		: throw std::bad_cast();
}

} // namespace zhukov