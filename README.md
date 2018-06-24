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
You can also group copy- and delete policies with `pl::compound`. See the #documentation for more details.


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
2. Copy-constructs `poly` from another poly. Internal policy is copied, and the internal pointer is *cloned* using the policy's `clone` method.
3. Move-constructs `poly` from another poly. Internal policy is moved, and the internal pointer simply move as pointer (shallow move).
4. Copy-constructs `poly` from a different poly. Compiles if 

Work in progress.
## License
This project is licenced under the MIT licence. It is free for personal and commercial use.
