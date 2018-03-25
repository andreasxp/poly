#include <iostream>
#include <string>
#include <map>
#include "poly_factory.hpp"

using namespace zhukov;
using namespace std;

struct A {
	int a;

	virtual ~A() = default;
};

struct B : A {
	int b;

	virtual ~B() = default;
};

struct C : B, A {
	int c;
};

//POLYREF_CTOR(decltype(f), D)
//POLYREF_CTOR(decltype(f), E)

int main() {
	try {
		auto var = make_poly<A, B>();
		var.as<B>().b = 10;

		auto var2 = var;

		cout << var2.as<B>().b;
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}
}