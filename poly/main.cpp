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

struct B {
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
		auto* ptr = new C;
		ptr->a = 10;
		ptr->b = 20;
		ptr->c = 30;

		poly<A> pa(ptr);
		poly<B> pb(pa);

		cout << pb->b << endl;
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}
}