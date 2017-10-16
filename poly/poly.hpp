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

#ifndef POLY_HPP
#define POLY_HPP

#include <memory> // std::unique_ptr
#include <typeinfo> // std::type_info
#include <typeindex> // std::type_index

///Namespace for all functions and classes, to not pollute global namespace
namespace zhukov {

///Namespace for internal-use-only functions
namespace detail {
//Black magic ahead

/*!
\brief   Wrapper for copy-constructor for derived_t class.
\details Creates a copy of a derived_t object, which is stored in a base_t class 
         ptr. This function is only enabled if base_t is base class of derived_t and 
         derived_t **is** Copy-Constructible.
\tparam  base_t base class
\tparam  derived_t derived class
\param   other Pointer to derived_t, casted to void* for use in generic
         function pointers
\warning This function performs **no checks** on whether the void* other
         parameter is actually of type derived_t. Passing an invalid argument results in
         undefined behaviour.
\return  A pointer to derived_t, casted to pointer to base_t.
\throw   std::bad_alloc If operator new fails to allocate memory
*/
template <typename base_t, typename derived_t>
constexpr typename std::enable_if<
	std::is_base_of<base_t, derived_t>::value &&
	std::is_copy_constructible<derived_t>::value, base_t*>::type
	poly_copy(const void* const other) {
	return new derived_t(*static_cast<const derived_t* const>(other));
}

/*!
\brief   Placeholder function for when copy-constructor of derived_t is deleted. 
\details This is a placeholder, enabled only when real poly_copy is disabled. 
         This function is only enabled if base_t is base class of derived_t and 
         derived_t **is not** Copy-Constructible.
\tparam  base_t base class
\tparam  derived_t derived_t class
\param   other Pointer to derived_t, casted to void* for use in generic
         function pointers
\warning This function does not actually construct anything, and will throw
         upon calling.
\throw   std::runtime_error Always.
*/
template <typename base_t, typename derived_t>
constexpr typename std::enable_if<
	std::is_base_of<base_t, derived_t>::value &&
	!std::is_copy_constructible<derived_t>::value, base_t*>::type //Note the !
	poly_copy(const void* const other) {
	throw std::runtime_error(
		"attempting to copy-construct a non-copyable object");
}

//There is no need for similar treatment of move-constructors:
//1. If an object is not move-constructible, it will be copied
//2. If an object is also not copy-constructible, it would
//   not end up in poly-wrapper in the first place.

} // namespace detail

/*!
\brief   Class that can hold any object derived from base_t class
\details This class holds information about its object. In particular, it
         remembers its type which can be used to safely cast the object
         using poly::as.
\tparam  base_t Class that all the objects must derive from
\see     poly::as
*/
template<typename base_t>
class poly {
public:
	using element_type = base_t;
private:
	///Placeholder for default-constructed poly
	struct invalid_type : base_t {};

	///RTTI of type, stored inside
	std::type_index stored_type;

	std::unique_ptr<base_t> data;
	base_t* (*copy_construct)(const void* const);
public:
	///Get RTTI of type, stored inside
	constexpr const std::type_index& get_stored_type() const;

	/*!
	\brief   Access operator
	\details Grants access to the object through base_t* pointer.
	\return  pointer to base_t (nullptr if not initialized)
	*/
	base_t* operator->();
	
	/*!
	\brief   Access operator
	\details Grants access to the object through base_t* pointer.
	\return  pointer to base_t (nullptr if not initialized)
	*/
	constexpr base_t* operator->() const;
	
	/*!
	\brief   Access function
 	\details Grants access to the object through base_t* pointer.
	\return  pointer to base_t (nullptr if not initialized)
	*/
	base_t* get();
	
	/*!
	\brief   Access function
 	\details Grants access to the object through base_t* pointer.
	\return  pointer to base_t (nullptr if not initialized)
	*/
	constexpr base_t* get() const;
	
	/*!
	\brief  Checks whether the object's actual type is T
	\tparam T Type to be checked against
	\return **true** if object is of type T, **false** otherwise
	*/
	template <typename T>
	constexpr bool is() const;
	
	/*!
	\brief   Allows to access poly object as if it was not polymorphic
	\details This function is similar to dynamic_cast, but it only casts
	         successfully if the type of object that is casted is exactly T.
	         as() is almost as fast as static_cast
	\tparam  T Type to cast to
	\throw   std::bad_cast if object is not exactly of type T
	\see     is()
	*/
	template <typename T>
	typename std::enable_if<
		std::is_base_of<base_t, T>::value, T&>::type
		as();

	/*!
	\brief   Allows to access poly<base_t> as the original class
	\details This function is similar to dynamic_cast, but it only casts
	         successfully if the type of object that is casted is exactly T. 
			 as() is almost as fast as static_cast
	\tparam  T Type to cast to
	\throw   std::bad_cast if object is not exactly of type T
	\see     is()
	*/
	template <typename T>
	constexpr typename std::enable_if<
		std::is_base_of<base_t, T>::value, T&>::type
		as() const;

	/*!
	\brief   Default constructor.
	\details Since no object is being held inside, stored_type will be equal to
	         poly::invalid_type. is<poly::invalid_type> will evaluate to true, and
			 using as<> to cast to any (meaningful) type will throw.
	*/
	constexpr poly();
	
	/*!
	\brief   Deep-Copy constructor.
	\details This constructor will initialize internal pointer with a copy
	         of obj. It is enabled only if obj is of a class derived_t from base_t, and
	         can be copy-constructed.
	\tparam  derived_t Type of object to be copied. Must be derived_t from base_t
	\param   obj Object to be copied
	\throw   std::bad_alloc if operator new fails to allocate memory
	*/
	template <typename derived_t, typename Condition = typename std::enable_if<
		std::is_base_of<base_t, derived_t>::value &&
		std::is_copy_constructible<derived_t>::value>::type>
		constexpr poly(const derived_t& obj);

	/*!
	\brief   Deep-Move constructor.
	\details This constructor will initialize internal pointer with a copy
	         of obj. It is enabled only if obj is of a class derived_t from base_t, and
	         can be copy-constructed.
	\tparam  derived_t Type of object to be copied. Must be derived_t from base_t
	\param   obj Object to be copied
	\throw   std::bad_alloc if operator new fails to allocate memory
	*/
	template <typename derived_t, typename Condition = typename std::enable_if<
		std::is_base_of<base_t, derived_t>::value &&
		std::is_move_constructible<derived_t>::value>::type>
		constexpr poly(derived_t&& obj); //Deep-Move constructor
	
	/*!
	\brief Copy constructor. Internal object will be copied with it's copy
	       constructor if it exists
	\throw std::runtime_error if internal object's copy constructor is
	       deleted
	*/
	constexpr poly(const poly& other);
	
	poly& operator=(const poly& rhs) = default;
	poly& operator=(poly&& rhs) = default;

	///Move constructor
	constexpr poly(poly&& other) = default;

	///Destructor
	~poly() = default;
};

template<typename base_t>
constexpr const std::type_index & poly<base_t>::get_stored_type() const {
	return stored_type;
}

template<typename base_t>
inline base_t * poly<base_t>::operator->() {
	return data.get();
}

template<typename base_t>
constexpr base_t * poly<base_t>::operator->() const {
	return data.get();
}

template<typename base_t>
inline base_t * poly<base_t>::get() {
	return data.get();
}

template<typename base_t>
constexpr base_t * poly<base_t>::get() const {
	return data.get();
}

template<typename base_t>
template<typename T>
constexpr bool poly<base_t>::is() const {
	return stored_type == std::type_index(typeid(T));
}

template<typename base_t>
template <typename T>
typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() {

	if (is<T>()) {
		return static_cast<T&>(*get());
	}
	else throw std::bad_cast();
	//I'm really not sure why code below does not do the same thing
	/*return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();*/
}

template<typename base_t>
template <typename T>
constexpr typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	poly<base_t>::as() const {
	return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();
}

template<typename base_t>
template<typename derived_t, typename Condition>
constexpr poly<base_t>::poly(const derived_t& obj) :
	data(new derived_t(obj)),
	stored_type(typeid(derived_t)),
	copy_construct(&detail::poly_copy<base_t, derived_t>) 
{}

template<typename base_t>
template<typename derived_t, typename Condition>
constexpr poly<base_t>::poly(derived_t && obj) :
	data(new derived_t(std::move(obj))),
	stored_type(typeid(derived_t)),
	copy_construct(&detail::poly_copy<base_t, derived_t>) 
{}

template<typename base_t>
constexpr poly<base_t>::poly() :
	data (nullptr),
	stored_type (std::type_index(typeid(invalid_type))),
	copy_construct(&detail::poly_copy<base_t, invalid_type>)
{}

template<typename base_t>
constexpr poly<base_t>::poly(const poly & other) :
	data(other.copy_construct(other.data.get())),
	stored_type(other.stored_type),
	copy_construct(other.copy_construct) 
{}

} // namespace zhukov

#endif // POLY_HPP