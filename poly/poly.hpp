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
         using basic_poly::as.
\tparam  base_t Class that all the objects must derive from
\see     basic_poly::as
*/
template<typename base_t, typename type_index_t, typename smart_ptr_t>
class basic_poly {
public:
	using element_type = base_t;
	using rtti_index = type_index_t;
	using smart_pointer = smart_ptr_t;
private:
	///Placeholder for default-constructed basic_poly
	struct invalid_type : base_t {};

	///RTTI of type, stored inside
	type_index_t stored_type;

	smart_ptr_t data;
	base_t* (*copy_construct)(const void* const);
public:
	///Get RTTI of type, stored inside
	constexpr const type_index_t& get_stored_type() const;

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
	\brief   Allows to access basic_poly object as if it was not polymorphic
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
	\brief   Allows to access basic_poly<base_t, type_index_t, smart_ptr_t> as the original class
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
	         basic_poly::invalid_type. is<basic_poly::invalid_type> will evaluate to true, and
			 using as<> to cast to any (meaningful) type will throw.
	*/
	constexpr basic_poly();
	
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
		constexpr basic_poly(const derived_t& obj);

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
		constexpr basic_poly(derived_t&& obj); //Deep-Move constructor
	
	/*!
	\brief Copy constructor. Internal object will be copied with it's copy
	       constructor if it exists
	\throw std::runtime_error if internal object's copy constructor is
	       deleted
	*/
	constexpr basic_poly(const basic_poly& other);
	
	basic_poly& operator=(const basic_poly& rhs) = default;
	basic_poly& operator=(basic_poly&& rhs) = default;

	///Move constructor
	constexpr basic_poly(basic_poly&& other) = default;

	///Destructor
	~basic_poly() = default;
};



template<typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr const type_index_t & basic_poly<base_t, type_index_t, smart_ptr_t>::get_stored_type() const {
	return stored_type;
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
inline base_t * basic_poly<base_t, type_index_t, smart_ptr_t>::operator->() {
	return data.get();
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr base_t * basic_poly<base_t, type_index_t, smart_ptr_t>::operator->() const {
	return data.get();
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
inline base_t * basic_poly<base_t, type_index_t, smart_ptr_t>::get() {
	return data.get();
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr base_t * basic_poly<base_t, type_index_t, smart_ptr_t>::get() const {
	return data.get();
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
template<typename T>
constexpr bool basic_poly<base_t, type_index_t, smart_ptr_t>::is() const {
	return stored_type == type_index_t(typeid(T));
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
template <typename T>
typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	basic_poly<base_t, type_index_t, smart_ptr_t>::as() {

	if (is<T>()) {
		return static_cast<T&>(*get());
	}
	else throw std::bad_cast();
	//I'm really not sure why code below does not do the same thing
	/*return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();*/
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
template <typename T>
constexpr typename std::enable_if<
	std::is_base_of<base_t, T>::value, T&>::type
	basic_poly<base_t, type_index_t, smart_ptr_t>::as() const {
	return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();
}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
template<typename derived_t, typename Condition>
constexpr basic_poly<base_t, type_index_t, smart_ptr_t>::basic_poly(const derived_t& obj) :
	data(new derived_t(obj)),
	stored_type(typeid(derived_t)),
	copy_construct(&detail::poly_copy<base_t, derived_t>) 
{}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
template<typename derived_t, typename Condition>
constexpr basic_poly<base_t, type_index_t, smart_ptr_t>::basic_poly(derived_t && obj) :
	data(new derived_t(std::move(obj))),
	stored_type(typeid(derived_t)),
	copy_construct(&detail::poly_copy<base_t, derived_t>) 
{}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr basic_poly<base_t, type_index_t, smart_ptr_t>::basic_poly() :
	data (nullptr),
	stored_type (type_index_t(typeid(invalid_type))),
	copy_construct(&detail::poly_copy<base_t, invalid_type>)
{}

template<typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr basic_poly<base_t, type_index_t, smart_ptr_t>::basic_poly(const basic_poly & other) :
	data(other.copy_construct(other.data.get())),
	stored_type(other.stored_type),
	copy_construct(other.copy_construct) 
{}

template<typename base_t>
using poly = basic_poly<base_t, std::type_index, std::unique_ptr<base_t>>;

/*!
\brief   Casts polymorphic base_t-pointers to their class
\details This function is similar to dynamic_cast, but it only casts
         successfully if the type of object that is casted is exactly T. exact_cast is
         almost as fast as static_cast
\tparam  T Pointer to type to cast to
\tparam  base_t base_t class. Function is only enabled if base_t is base_t of T.
\param   obj Object to be casted
\return  pointer to casted object or nullptr if casting failed
\see     basic_poly
*/
/*template <typename T, typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr typename std::enable_if<
	std::is_pointer<T>::value &&
	std::is_base_of<base_t, 
	typename std::remove_pointer<T>::type>::value, T>::type
	exact_cast(const basic_poly<base_t, type_index_t, smart_ptr_t>& obj) {
	return obj.template is<std::remove_pointer<T>::type>()
		? static_cast<T>(obj.get())
		: nullptr;
}*/

/*!
\brief   Casts polymorphic base_t-pointers to their class
\details This function is similar to dynamic_cast, but it only casts
         successfully if the type of object that is casted is exactly T. exact_cast is
         almost as fast as static_cast
\tparam  T Pointer to type to cast to
\tparam  base_t base_t class. Function is only enabled if base_t is base_t of T.
\param   obj Object to be casted
\throw   std::bad_cast if object is not exactly of type T
\see     basic_poly
*/
/*template <typename T, typename base_t, typename type_index_t, typename smart_ptr_t>
constexpr typename std::enable_if<
	!std::is_pointer<T>::value && 
	std::is_base_of<base_t, T>::value, T>::type
	exact_cast(const basic_poly<base_t, type_index_t, smart_ptr_t>& obj) {
	return obj.template is<T>()
		? *static_cast<T*>(obj.get())
		: throw std::bad_cast();
}*/


} // namespace zhukov

#endif // POLY_HPP