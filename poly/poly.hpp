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

///Typedef for a type that can store RTTI and is copy-constructible
using type_index_t = std::type_index;

///Namespace for internal-use-only functions
namespace details {
//Black magic ahead

/*!
\brief   Wrapper for copy-constructor for Derived class.
\details Creates a copy of a Derived object, which is stored in a Base class 
         ptr. This function is only enabled if Base is base class of Derived and 
         Derived **is** Copy-Constructible.
\tparam  Base Base class
\tparam  Derived Derived class
\param   other Pointer to Derived, casted to void* for use in generic
         function pointers
\warning This function performs **no checks** on whether the void* other
         parameter is actually of type Derived. Passing an invalid argument results in
         undefined behaviour.
\return  A pointer to Derived, casted to pointer to Base.
\throw   std::bad_alloc If operator new fails to allocate memory
*/
template <typename Base, typename Derived>
constexpr typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	std::is_copy_constructible<Derived>::value, Base*>::type
	copy_ctor(const void* const other) {
	return new Derived(*static_cast<const Derived* const>(other));
}

/*!
\brief   Placeholder function for when copy-constructor of Derived is deleted. 
\details This is a placeholder, enabled only when real copy_ctor is disabled. 
         This function is only enabled if Base is base class of Derived and 
         Derived **is not** Copy-Constructible.
\tparam  Base Base class
\tparam  Derived Derived class
\param   other Pointer to Derived, casted to void* for use in generic
         function pointers
\warning This function does not actually construct anything, and will throw
         upon calling.
\throw   std::runtime_error Always.
*/
template <typename Base, typename Derived>
constexpr typename std::enable_if<
	std::is_base_of<Base, Derived>::value &&
	!std::is_copy_constructible<Derived>::value, Base*>::type //Note the !
	copy_ctor(const void* const other) {
	throw std::runtime_error(
		"attempting to copy-construct a non-copyable object");
}

//There is no need for similar treatment of move-constructors:
//1. If an object is not move-constructible, it will be copied
//2. If an object is also not copy-constructible, it would
//   not end up in poly-wrapper in the first place.

} // namespace details

/*!
\brief   Class that can hold any object derived from Base class
\details This class holds information about its object. In particular, it
         remembers it's type which can be used to safely cast the object
         using exact_cast
\tparam  Base Class that all the objects must derive from
\see     exact_cast
*/
template<typename Base>
class poly {
private:
	///Placeholder for default-constructed poly
	struct invalid_type : Base {};

	///RTTI of type, stored inside
	type_index_t stored_type;

	std::unique_ptr<Base> data;
	Base* (*copy_construct)(const void* const);
public:
	///Get RTTI of type, stored inside
	constexpr type_index_t& get_stored_type() const;

	/*!
	\brief   Access operator
	\details Grants access to the object through Base* pointer.
	\return  pointer to Base (nullptr if not initialized)
	*/
	Base* operator->();
	
	/*!
	\brief   Access operator
	\details Grants access to the object through Base* pointer.
	\return  pointer to Base (nullptr if not initialized)
	*/
	constexpr Base* operator->() const;
	
	/*!
	\brief   Access function
 	\details Grants access to the object through Base* pointer.
	\return  pointer to Base (nullptr if not initialized)
	*/
	Base* get();
	
	/*!
	\brief   Access function
 	\details Grants access to the object through Base* pointer.
	\return  pointer to Base (nullptr if not initialized)
	*/
	constexpr Base* get() const;
	
	/*!
	\brief  Checks whether the object is of type T
	\tparam T Type to be checked against
	\return **true** if object is of type T, **false** otherwise
	*/
	template <typename T>
	constexpr bool is() const;
	
	/*!
	\brief   Allows to access poly<Base> as the original class
	\details This function is similar to dynamic_cast, but it only casts
	successfully if the type of object that is casted is exactly T.
	as() is almost as fast as static_cast
	\tparam  T Type to cast to
	\throw   std::bad_cast if object is not exactly of type T
	\see     is()
	*/
	template <typename T>
	typename std::enable_if<
		std::is_base_of<Base, T>::value, T&>::type
		as();

	/*!
	\brief   Allows to access poly<Base> as the original class
	\details This function is similar to dynamic_cast, but it only casts
	         successfully if the type of object that is casted is exactly T. 
			 as() is almost as fast as static_cast
	\tparam  T Type to cast to
	\throw   std::bad_cast if object is not exactly of type T
	\see     is()
	*/
	template <typename T>
	constexpr typename std::enable_if<
		std::is_base_of<Base, T>::value, T&>::type
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
	         of obj. It is enabled only if obj is of a class derived from Base, and
	         can be copy-constructed.
	\tparam  Derived Type of object to be copied. Must be derived from Base
	\param   obj Object to be copied
	\throw   std::bad_alloc if operator new fails to allocate memory
	*/
	template <typename Derived, typename Condition = typename std::enable_if<
		std::is_base_of<Base, Derived>::value &&
		std::is_copy_constructible<Derived>::value>::type>
		constexpr poly(const Derived& obj);

	/*!
	\brief   Deep-Move constructor.
	\details This constructor will initialize internal pointer with a copy
	         of obj. It is enabled only if obj is of a class derived from Base, and
	         can be copy-constructed.
	\tparam  Derived Type of object to be copied. Must be derived from Base
	\param   obj Object to be copied
	\throw   std::bad_alloc if operator new fails to allocate memory
	*/
	template <typename Derived, typename Condition = typename std::enable_if<
		std::is_base_of<Base, Derived>::value &&
		std::is_move_constructible<Derived>::value>::type>
		constexpr poly(Derived&& obj); //Deep-Move constructor
	
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
	constexpr poly(poly&& other) = default; //Move constructor

	~poly() = default; //Destructor
};



template<typename Base>
constexpr type_index_t & poly<Base>::get_stored_type() const {
	return stored_type;
}

template<typename Base>
inline Base * poly<Base>::operator->() {
	return data.get();
}

template<typename Base>
constexpr Base * poly<Base>::operator->() const {
	return data.get();
}

template<typename Base>
inline Base * poly<Base>::get() {
	return data.get();
}

template<typename Base>
constexpr Base * poly<Base>::get() const {
	return data.get();
}

template<typename Base>
template<typename T>
constexpr bool poly<Base>::is() const {
	return stored_type == type_index_t(typeid(T));
}

template<typename Base>
template <typename T>
typename std::enable_if<
	std::is_base_of<Base, T>::value, T&>::type
	poly<Base>::as() {

	if (is<T>()) {
		return static_cast<T&>(*get());
	}
	else throw std::bad_cast();
	//I'm really not sure why code below does not do the same thing
	/*return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();*/
}

template<typename Base>
template <typename T>
constexpr typename std::enable_if<
	std::is_base_of<Base, T>::value, T&>::type
	poly<Base>::as() const {
	return is<T>()
		? static_cast<T&>(*get())
		: throw std::bad_cast();
}

template<typename Base>
template<typename Derived, typename Condition>
constexpr poly<Base>::poly(const Derived& obj) :
	data(new Derived(obj)),
	stored_type(typeid(Derived)),
	copy_construct(&details::copy_ctor<Base, Derived>) 
{}

template<typename Base>
template<typename Derived, typename Condition>
constexpr poly<Base>::poly(Derived && obj) :
	data(new Derived(std::move(obj))),
	stored_type(typeid(Derived)),
	copy_construct(&details::copy_ctor<Base, Derived>) 
{}

template<typename Base>
constexpr poly<Base>::poly() :
	data (nullptr),
	stored_type (type_index_t(typeid(invalid_type))),
	copy_construct(&details::copy_ctor<Base, invalid_type>)
{}

template<typename Base>
constexpr poly<Base>::poly(const poly & other) :
	data(other.copy_construct(other.data.get())),
	stored_type(other.stored_type),
	copy_construct(other.copy_construct) 
{}

/*!
\brief   Casts polymorphic base-pointers to their class
\details This function is similar to dynamic_cast, but it only casts
         successfully if the type of object that is casted is exactly T. exact_cast is
         almost as fast as static_cast
\tparam  T Pointer to type to cast to
\tparam  Base Base class. Function is only enabled if Base is base of T.
\param   obj Object to be casted
\return  pointer to casted object or nullptr if casting failed
\see     poly
*/
/*template <typename T, typename Base>
constexpr typename std::enable_if<
	std::is_pointer<T>::value &&
	std::is_base_of<Base, 
	typename std::remove_pointer<T>::type>::value, T>::type
	exact_cast(const poly<Base>& obj) {
	return obj.template is<std::remove_pointer<T>::type>()
		? static_cast<T>(obj.get())
		: nullptr;
}*/

/*!
\brief   Casts polymorphic base-pointers to their class
\details This function is similar to dynamic_cast, but it only casts
         successfully if the type of object that is casted is exactly T. exact_cast is
         almost as fast as static_cast
\tparam  T Pointer to type to cast to
\tparam  Base Base class. Function is only enabled if Base is base of T.
\param   obj Object to be casted
\throw   std::bad_cast if object is not exactly of type T
\see     poly
*/
/*template <typename T, typename Base>
constexpr typename std::enable_if<
	!std::is_pointer<T>::value && 
	std::is_base_of<Base, T>::value, T>::type
	exact_cast(const poly<Base>& obj) {
	return obj.template is<T>()
		? *static_cast<T*>(obj.get())
		: throw std::bad_cast();
}*/


} // namespace zhukov

#endif // POLY_HPP