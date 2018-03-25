#include <iostream>
#include <string>
#include <map>
#include "poly_factory.hpp"

using namespace zhukov;
using namespace std;

struct Base {
	int a;

	Base() : a(1) {}
	virtual ~Base() = default;
};

struct Mid1 : Base {
	int x;
	Mid1() : x(2) {}
	virtual ~Mid1() = default;
};

struct Mid2 : Base {
	int y;

	Mid2() : y(3) {}
	virtual ~Mid2() = default;
};

struct Mid3 : Base {
	int h;

	Mid3() : h(3) {}
	virtual ~Mid3() = default;
};

struct Der : Mid1, Mid2 {
	Der() : z(4) {}
	int z;
};

//POLYREF_CTOR(decltype(f), D)
//POLYREF_CTOR(decltype(f), E)

int main() {
	try {
		factory<Base> pf;
		pf.add<Der>();

		auto v1 = make_poly<Mid1, Der>();
		poly<Mid3> v2 = v1;

		cout << v2->h;
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}
}