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

POLYREF_CTOR(B, D)
POLYREF_CTOR(B, E)

int main() {
	auto t = make_s<B>("struct D");
	t.as<D>().y = 5;
	t.as<D>().x = 4;
	std::cout << t.as<D>().y << std::endl;

	std::cout << typeid(t).name() << std::endl;
	std::cout << t.as<D>().x << std::endl;

	auto list = zhukov::factory<B>::list();
	for (auto&& it : list) {
		std::cout << it << std::endl;
	}
}