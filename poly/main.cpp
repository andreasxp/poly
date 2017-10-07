#include <iostream>
#include <string>
#include <map>
#include "poly_factory.hpp"

using namespace zhukov;

struct B {
	int x;
};

struct D : B {
	int y;
};

struct E : B {
	int y;
};

//POLYREF_CTOR(decltype(f), D)
//POLYREF_CTOR(decltype(f), E)

int main() {
	factory<B> f;
	f.add<D>();
	f.add<E>();

	auto t = f.make("struct D");
	auto t2 = t;
	t2 = f.make("struct E");
	t2.as<E>().y = 12;

	std::cout << t2.as<E>().y << std::endl;

	auto list = f.list();
	for (auto&& it : list) {
		std::cout << it << std::endl;
	}
}