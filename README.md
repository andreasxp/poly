# poly
A library for easier (and faster!) polymorphism in C++.
Avoid memory leaks, enforce correct polymorphic behavior, clone without CRTP, and quickly switch between Base and Derived classes without using `dynamic_cast`.
## Installation
Poly is a single-header library. You can download the latest release [here](https://github.com/andreasxp/poly/releases).
To use `poly`, your compiler must be up to ISO C++11 standard.
## Usage
*This section contains only a basic explanation. For a more complete documentation, refer [here](#Documentation)*
#### Include header:
```c++
#include "poly.hpp"
using namespace zhukov; //For convenience
```

### `poly`
`poly` is the main class of this library. It's a smart pointer, that understands and supports polymorphic objects.

#### Create a `poly`:
```c++
poly<Animal> p1; //empty
poly<Animal> p2(new Dog); //holds a default-constructed derived object
poly<Animal> p3 = make_poly<Animal, Dog>(4, "ears", true); //Using a constructor function
```
Where `Dog` is derived from `Animal`.

#### Use `poly`:
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

auto         poly_carn1 = transform_poly<Carnivorous, Dog>(poly_mamm1); //Side-cast Mammal to Carnivorous (dogs only!)
auto         poly_carn2 = transform_poly<Carnivorous, Dog>(poly_mamm2); //Error (Cow is not a Dog)
auto         poly_carn3 = transform_poly<Carnivorous, Cow>(poly_mamm2); //Error (Cow is not a carnivorous)

poly<Dog>    poly_doggo = transform_poly<Dog, Dog>(poly_mammal); //Down-cast mammal to Dog
poly<Fish>   poly_fishy = transform_poly<Fish, Dog>poly_mammal; //Error (doggo is not a fish)
```

#### Using policies to customize the behavior of `poly`
`poly` has an optional second template parameter called CopyPolicy. It defines the behavior of `poly` when a copy-constructor is invoked. By default, it is set to `zhukov::no_copy`. With this option, `poly` has no memory overhead and behaves like a `unique_ptr` (cannot be copy-constructed). You can also set the parameter `zhukov::deep_copy`, so that on copy, the internal pointer will be *cloned* (i.e. correctly copied as a derived object). With `zhukov::deep_copy`, memory overhead is one pointer.

```c++
//           VVVVVVV -  default parameter  - VVVVVVV
poly<Animal, no_copy> p1 = make_poly<Animal, no_copy, Dog>();
poly<Animal, deep_copy> p2 = make_poly<Animal, deep_copy, Dog>();

auto p3 = p1; //Error: p1 is not copy-constructible
auto p4 = p2; //Works, internal object is also copied
```

*Note: if `poly` is copyable but the derived object is not, on copy a runtime exception will occur.*

### `poly_factory`
`poly.hpp` also contains class `factory`. `factory` lets you create poly by passing a string, representing the derived class. 
#### Create a `factory`:
```c++
factory<Animal> animal_farm;
```

#### Register a class in the factory:
```c++
animal_farm.insert<Dog>();
```

#### Make a dog:
By default, `poly`s that come out of the factory are deep-copyable.
```c++
auto doggo = animal_farm.make("Dog"); //Here, auto is poly<Animal, deep_copy>
```
*Note: Different compilers will require a diffrent string to create a class, depending on what typeid(Dog).name() returns. Consider using [some other rtti library](https://github.com/andreasxp/prindex) along with defining POLY_CUSTOM_TYPE_NAME(type) macro before including for a cross-compiler result.*

## Documentation
All described classes are in the namespace `zhukov`.
### `class poly`
```c++
template<class Base, template<class> class CopyPolicy = no_copy>
class poly;
```

`poly` is a smart pointer that owns and manages a polymorphic object through a pointer-to-base and disposes of that object when the `poly` goes out of scope.

The object is disposed of when either of the following happens:
* the managing `poly` is destroyed
* the managing `poly` is assigned another pointer via operator= or reset().
The object is disposed of using `operator delete`, which destroys the object using its virtual destructor and deallocates the memory.

A unique_ptr may alternatively own no object, in which case it is called empty.

The class satisfies the requirements of MoveConstructible, MoveAssignable, CopyConstructible and CopyAssignable, but the latter two do not compile if the copy policy does not provide a `clone` method. The default copy policy - `no_copy` - does not provide this method, but can alternatively be switched to `deep_copy`.

#### Member types
* `base_type`  
Base type, from which every polymorphic object is derived. Equivalent to `Base`.
* `copy_policy`  
Copy policy, used to manage internal pointer's behavior. Equivalent to `CopyPolicy<Base>`.

#### Member functions

Work in progress.

## License
This project is licenced under the MIT licence. It is free for personal and commercial use.
