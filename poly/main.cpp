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
	auto t2 = t;
	t2 = make<B>("struct E");

	auto list = zhukov::factory<B>::list();
	for (auto&& it : list) {
		std::cout << it << std::endl;
	}
}