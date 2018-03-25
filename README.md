# poly
A library for easier polymorphism in C++
## Download
To use `poly`, only a single file is required - `poly.hpp`. You can also use it in conjunction with `poly_factory` by downloading `poly_factory.hpp`. All downloads are available [here](https://github.com/andreasxp/poly/releases).
## Compiler requirements
To use `poly`, your compiler must be up to ISO C++14 standard (It could work on C++11, testing is in progress, sorry!).
## How to use
### `poly.hpp`
#### Include header:
```c++
#include "poly.hpp"
using zhukov::poly; //For convenience
```
#### Create a `poly`:
```c++
poly<Animal> p1(); //empty
poly<Animal> p2(new Dog); //holds a default-constructed derived object
poly<Animal> p3 = make_poly<Animal, Dog>(4, "ears", true); //Using a constructor function
```
Where `Dog` is derived from `Animal`

#### Use `poly`:
```c++
bool is_dog = p2.is<Dog>(); //true only if p2 holds exactly a Dog (not derived from Dog)
if (is_dog)
    p2.as<Dog>().pet(); //Cast to dog (only works for Dog and typed derived from Dog)
```

#### Create a `poly`, vol. 2:
Assume you have the following data structure:
```c++
struct Animal;

struct Mammal : Animal;
struct Fish : Animal;
struct Carnivorous : Animal;

struct Dog : Mammal, Carnivorous;
struct Cow : Mammal;
```
This is a fairly complex data structure, but `poly` can down- and sidecast anywhere it is allowed to.
```c++
poly<Mammal>      poly_mamm1(new Dog); //Construct a mammal
poly<Mammal>      poly_mamm2(new Cow); //Construct a different mammal

poly<Carnivorous> poly_carn1 = poly_mamm1; //Side-cast Mammal to Carnivorous (dogs only!)
poly<Carnivorous> poly_carn2 = poly_mamm2; //Runtime exception (Cow is not carnivorous)

poly<Dog>         poly_doggo = poly_mammal; //Down-cast mammal to Dog
poly<Fish>        poly_fishy = poly_mammal; //Runtime exception (doggo is not a fish)
```
### `poly_factory.hpp`
#### Include header:
```c++
#include "poly_factory.hpp"
//For convenience
using zhukov::poly;
using zhukov::factory;
```
#### Create a `poly_factory`:
```c++
factory<Animal> animal_farm;
```

#### Register a class in the factory:
```c++
animal_farm.add<Dog>;
```

#### Make a dog:
```c++
poly<Animal> doggo = animal_farm.make("Dog");
```
*Note: Different compilers will require a diffrent string to create a class, depending on what typeid(Dog).name() returns. Consider using [some other rtti library](https://github.com/andreasxp/pretty_index) along with a POLY_CUSTOM_RTTI(type) macro for a cross-compiler result.*

Source code contains extensive documentation on every class, method and macro in doxygen format.

## License
This project is licenced under the MIT licence. It is free for personal and commercial use.
