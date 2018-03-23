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
		factory<A> fac;

		fac.add<B>();
		
		poly<A> var = fac.make("struct B");
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}
}