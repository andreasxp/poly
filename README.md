# poly
A library for easier (and faster!) polymorphism in C++.
Avoid memory leaks, enforce correct polymorphic behavior, clone without CRTP, and quickly switch between Base and Derived classes without using `dynamic_cast`.
## Installation
Poly is a single-header library. You can download the latest release [here](https://github.com/andreasxp/poly/releases).
To use `poly`, your compiler must be up to ISO C++11 standard.
## Usage
This section contains only a basic explanation. For a more complete documentation, refer [here](#documentation).
#### Include header:
```c++
#include "poly.hpp"
using pl::poly;    //For convenience
using pl::factory; //For convenience
```
**Warning:** Do not use `using namespace pl` and `using namespace std` together in one program. You will get name collisions.

### `poly`
`poly` is the main class of this library. It's a smart pointer, that understands and supports polymorphic objects.

#### Create a `poly`:
```c++
poly<Animal> p1; //empty
poly<Animal> p2(new Dog); //holds a default-constructed derived object
poly<Animal> p3 = pl::make<poly<Animal>, Dog>(4, "ears", true); //Using a constructor function
poly<Mammal> p4 = pl::transform<poly<Mammal>, Dog>(p3); //Transforming from another poly
```
Where `Dog` is derived from `Animal`.

#### Use `poly`:
```c++
Animal* animal_ptr = p1.release(); //Release the stored pointer
p1 = new Dog; //Assign a new pointer

if (p1) { //Check if poly holds an object
    p1.reset(); //Clear stored value
}
```

```c++
bool is_dog = p2.is<Dog>(); //true only if p2 holds exactly a Dog
if (is_dog)
    p2.as<Dog>()->pet(); //Cast to dog (only works for Dog)
```

#### Create a `poly`, vol. 2:
Assume you have the following data structure:
```c++
struct Animal;

struct Mammal : virtual Animal;
struct Fish : virtual Animal;
struct Carnivorous : virtual Animal;

struct Dog : Mammal, Carnivorous;
struct Cow : Mammal;
```
This is a fairly complex data structure, but `poly.hpp` provides tools to down- and sidecast anywhere you need.
```c++
poly<Mammal> poly_mamm1(new Dog); //Construct a mammal
poly<Mammal> poly_mamm2(new Cow); //Construct a different mammal

auto         poly_carn1 = pl::transform<poly<Carnivorous>, Dog>(poly_mamm1); //Side-cast Mammal to Carnivorous (dogs only!)
auto         poly_carn2 = pl::transform<poly<Carnivorous>, Dog>(poly_mamm2); //Compile error (Cow is not a Dog)
auto         poly_carn3 = pl::transform<poly<Carnivorous>, Cow>(poly_mamm2); //Compile error (Cow is not Carnivorous)

poly<Dog>    poly_doggo = pl::transform<poly<Dog>, Dog>(poly_mammal); //Down-cast mammal to Dog
poly<Fish>   poly_fishy = pl::transform<poly<Dog>, Dog>(poly_mammal); //Compile error (doggo is not a fish)
```

#### Using policies to customize the behavior of `poly`
`poly` has an optional second template parameter called CopyDeletePolicy. It defines the behavior of `poly` when a copy-constructor or a destructor is invoked. `poly.hpp` contains 2 pre-built policies:  
* `pl::deep` (default): When `poly` is copied, internal object is copied as well. `pl::deep` invokes the proper copy-constructor of the derived object. That means you don't need to add the `clone` method to your class. `pl::deep` is also destructor-permissive: if you forgot to define your base class destructor `virtual`, the pointer in `poly` will still be deleted without any memory leaks.  
`pl::deep` adds a memory overhead of one pointer if the base class has a virtual destructor, and 2 pointers if it doesn't.
* `pl::unique`: With this policy, `poly` behaves like a `unique_ptr` (a.k.a. is not copy-constructible). `pl::unique` is also destructor-permissive.  
`pl::unique` adds no memory overhead if the base class has a virtual destructor, and an overhead of 1 pointer if it doesn't.

```c++
poly<Animal, pl::deep<Animal>> p1 = pl::make<poly<Animal, pl::deep<Animal>>, Dog>(); //Same as poly<Animal> p1 = pl::make<poly<Animal>, Dog>()
poly<Animal, pl::unique<Animal>> p2 = make<poly<Animal, pl::unique<Animal>>, Dog>();

auto p3 = p1; //Works, internal object is also copied
auto p4 = p2; //Error: p2 is not copy-constructible
```
You can make your own policies, too. A basic policy looks like the following:
```c++
class my_policy {
    my_policy(); //Default constructor
    my_policy(const T* ptr); //Construct a policy for operating on the type T
    
    T* clone(const T* ptr); //Clones the object held by ptr.
    void destroy(T* ptr); //Destroys the object held by ptr.
};
```
You can also group copy- and delete policies with `pl::compound`. See the [documentation](#class-compound) for more details.

*Note: if `poly` is copyable but the derived object is not, on copy a runtime exception will occur.*

### `factory`
`poly.hpp` also contains class `factory`. `factory` lets you create poly by passing a string, representing the derived class. 
#### Create a `factory`:
```c++
factory<Animal> animal_farm;
//Or factory<Animal, pl::unique<Animal>> for a non-copyable alternative.
```

#### Register a class in the factory:
```c++
animal_farm.insert<Dog>();
```

#### Make a dog:
```c++
auto doggo = animal_farm.make("Dog"); //Here, auto is poly<Animal, pl::deep<Animal>>
```
*Note: Different compilers will require a diffrent string to create a class, depending on what typeid(Dog).name() returns. Consider using [some other rtti library](https://github.com/andreasxp/prindex) along with defining POLY_CUSTOM_TYPE_NAME(type) macro before including for a cross-compiler result.*

## Documentation
All described classes are in the namespace `pl`.
### `class poly`
```c++
template<class Base, class CopyDeletePolicy = deep<Base>>
class poly;
```

`poly` is a smart pointer that owns and manages a polymorphic object through a pointer-to-base and disposes of that object when the `poly` goes out of scope.

The object is disposed of when either of the following happens:
* the managing `poly` is destroyed
* the managing `poly` is assigned another pointer via operator= or reset().
The object is disposed of using the selected policy's `destroy` function.

`poly` may alternatively own no object, in which case it is called empty.

The class satisfies the requirements of MoveConstructible, MoveAssignable, and CopyConstructible, CopyAssignable if the selected policy  provides a `clone` method. The default policy - `deep` - does provide this method, but can alternatively be switched to `unique` which does not.

#### Member types
`base_type` - Base type, from which every polymorphic object is derived. Equivalent to `Base`.
`policy` - Policy, used to manage internal pointer's behavior. Equivalent to `CopyDeletePolicy`.

#### Member functions
##### Constructors and assignment operators
```c++
constexpr poly() noexcept;                                          | (1)
constexpr poly(std::nullptr_t) noexcept;                            | 
poly& operator=(std::nullptr_t) noexcept;                           |
-------------------------------------------------------------------------
poly(const poly& other);                                            | (2)
poly& operator=(const poly& other);                                 |        
-------------------------------------------------------------------------
poly(poly&&) noexcept;                                              | (3)
poly& operator=(poly&&) noexcept;                                   |	
-------------------------------------------------------------------------
template <class Base2, class CopyDeletePolicy2,                     | (4)
    class = typename std::enable_if<                                |
    detail::is_stronger_qualified<Base, Base2>::value>::type>>      |
poly(const poly<Base2, CopyDeletePolicy2>& other);                  |
-------------------------------------------------------------------------
template <class Base2, class CopyDeletePolicy2,                     | (5)
    class = typename std::enable_if<                                |
    detail::is_stronger_qualified<Base, Base2>::value>::type>>      |
poly(poly<Base2, CopyDeletePolicy2>&& other) noexcept;              |
-------------------------------------------------------------------------
template <class Derived>                                            | (6)
explicit poly(Derived* obj);                                        |    
template <class Derived>                                            |
poly& operator=(Derived* obj);                                      |      
```
1. Constructs an empty `poly` that owns nothing. Default-constructs the internal policy object.
2. Copy-constructs `poly` from another `poly`. Internal policy is copied, and the internal pointer is *cloned* using the policy's `clone` method.
3. Move-constructs `poly` from another `poly`. Internal policy is moved, and the internal pointer simply move as pointer (shallow move).
4. Copy-constructs `poly` from a different `poly`. This converting constructor is enabled if new base is more strongly qualified than old base. For example, using this contructor, `poly<const Base>` is implicitly constructible from `poly<Base>`, but not `poly<volatile Base>`.
5. Move-constructs `poly` from a different `poly`. Same rules as for (4) apply.
6. Constructs a poly by adopting a raw pointer to a derived class. Besides pointing to a class, derived from Base, the pointer must also *not* be a polymorhphic pointer to a different object (i.e. `Derived*` that points to `struct Derived2 : Derived`). Attemping to adopt such a pointer will result in a runtime exception.
##### Destructor
```c++
~poly();
```
Destructs the managed pointer using selected policy's `destroy()` function.
##### Observers
```c++
template <class T> constexpr bool is() const noexcept;
```
Checks if the stored pointer holds an object is of type `T`. Returns `true` if it does, `false` otherwise.
```c++
explicit constexpr operator bool() const noexcept;
```
Checks whether `poly` owns an object, i.e. whether `get() != nullptr`. Returns `true` if it does, `false` otherwise.
##### Modifiers
```c++
template <class Derived>                         | (1)
void reset(Derived* ptr);                        |
-------------------------------------------------------
void reset(std::nullptr_t = nullptr) noexcept;   | (2)
```
1. Destructs the managed pointer using selected policy's `destroy()` function, and replaces it with `ptr`. For `ptr`, same constraints as in constructor (6) apply.
2. Destructs the managed pointer using selected policy's `destroy()` function, and replaces it with `nullptr`.
```c++
Base* release() noexcept;
```
Releases the ownership of the managed object if any. `get()` returns `nullptr` after the call. Returns pointer to the managed object as `Base*` or `nullptr` if there was no managed object, i.e. the value which would be returned by `get()` before the call.
##### Member access
```c++
Base& operator*() const;             | (1)
-------------------------------------------
Base* operator->() const noexcept;   | (2)
Base* get() const noexcept;          |
```
1. Returns the object owned by `poly`, equivalent to `*get()`. The object must not be empty, otherwise this operation results in undefined behavior.
2. Returns the managed pointer, or `nullptr` if no object is owned.
```c++
template <class T>
T* as() const noexcept;
```
Returns a pointer to the object owned by `poly` in the exact type of that object. Returns `nullptr` if the stored object is not of type `T` or if `poly` is empty.

*Note: This function is not the same as dynamic_cast. First, types must match exactly, i.e. no up- or side-casting is allowed. Second, perfomance of this function is much better than of dynamic_cast, as no type tree traversal is performed.*
### `class factory`
```c++
template <class Base, class CopyDeletePolicy = deep<Base>>
class factory;
```
`factory` is a class that registers and creates `poly` objects using strings as identifiers.  
By default, when a type is registered, a string identifier is generated using `std::type_info::name`, but this behavior can be overridden by defining a macro `POLY_CUSTOM_TYPE_NAME(type)` that accepts a type token and retuns a value of type `const cher*` or `std::string`. This macro must be defined before including `poly.hpp`.
Example of a custom name generator using [prindex](https://github.com/andreasxp/prindex) library:
```c++
#include "prindex.hpp"
// Using __VA_ARGS__ is recommended to elide the commas that can appear in templated types.
#define POLY_CUSTOM_TYPE_NAME(...) prid<__VA_ARGS__>().name()
#include "poly.hpp"
```
Usage example:
```c++
pl::factory<Base> f;
f.insert<Mid1>();
f.insert<Mid2>();
f.insert<Der>();

auto p = f.make("struct Der"); //MSVC
//auto p = f.make("Der"); //GCC or Clang (or prindex)

auto ls = f.list();
for (auto& i : ls) std::cout << i << std::endl;
```
Possible output:
```
struct Der
struct Mid1
struct Mid2
```
#### Member functions
```c++
template <class Derived> void insert();
```
Inserts the type `Derived` into the factory. This function must be called at least once before `poly<Derived>` can be made with this instance of the factory.
```c++
std::vector<std::string> list() const;
```
Returns a list of all types, registered in the factory, as a `std::vector` of `std::string`s that are used to `make` objects.
```c++
poly<Base, CopyDeletePolicy> make(const std::string& name) const;
```
Makes a `poly<Base, CopyDeletePolicy>`, holding a type, represented by the string `name`. The required type must be registered using `insert` before a `poly` of that type can be made. If no such type is registered, a runtime exception is thrown.
### `class compound`
```c++
template <class Cloner, class Deleter>
class compound;
```
`compound` is a helper class that can be used to build policies for `poly`. When instantiated with a cloner and a deleter, `compound` becomes a valid CopyDeletePolicy.
Both `Cloner` and `Deleter` are classes that
1. Provide either or both:
    * a constructor from `const T*`, where `T` is the type on which the class operates;
    * a default constructor.  
    
    If both constructors are provided, the class will be instantiated with `const T*`.
2. An `operator()`:  
    * For `Cloner`, `operator()` accepts `const T*` and returns a pointer to a copied object;  
    * For `Deleter`, `operator()` accepts `T*`, destroys the object, and returns nothing.

All pre-defined policies were made using `compound`.
### `unique`
```c++
template <class Base>
using unique = compound<no_copy,
	typename std::conditional<std::has_virtual_destructor<Base>::value,
	std::default_delete<Base>,
	pmr_delete<Base>>::type>;
```
`unique` is a policy for `poly` that disallows copying. When instantiated with this policy, `poly` is not CopyConstructible nor CopyAssignable.
`unique` is destructor-permissive: if no virtual destructor was provided, this policy guarantees that the object will be destructed properly and without memory leaks.
### `deep`
```c++
template <class Base>
using deep = compound<deep_copy<Base>,
	typename std::conditional<std::has_virtual_destructor<Base>::value,
	std::default_delete<Base>,
	pmr_delete<Base>>::type>;
```
`deep` is a policy for `poly` that allows copying. When instantiated with this policy, `poly` is CopyConstructible and CopyAssignable.
`deep` is destructor-permissive: if no virtual destructor was provided, this policy guarantees that the object will be destructed properly and without memory leaks.
## License
This project is licenced under the MIT licence. It is free for personal and commercial use.
