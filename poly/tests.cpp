#include <iostream>
#include <iomanip>
#include "tests.hpp"
#include "poly.hpp"
#include "poly_factory.hpp"

using namespace zhukov;
using namespace std;

#define TEST(...) print_test_result(#__VA_ARGS__, __VA_ARGS__)

void Tester::print_test_result(const std::string & name, bool result) {
	string spacer(80 - 13 - name.size(), ' ');
	cerr << std::boolalpha <<
		"Testing " << name << spacer << result << endl;
}

void Tester::run() {
	// poly ====================================================================
	poly<Base> p0(new Der);
	poly<Base> p1(p0);
	poly<Base> p2(std::move(p0));
	auto p3 = make_poly<Base, Der>();
	auto p4 = transform_poly<Mid1, Der>(p2);

	TEST(p1->get() == "der");
	TEST(p1->get() == p2->get());
	TEST(p1->get() == p3->get());
	TEST(p1->get() == p4->get());

	TEST(p1.is<Der>());
	TEST(p2.is<Der>());
	TEST(p3.is<Der>());
	TEST(p4.is<Der>());

	TEST(!p1.is<Base>());
	TEST(!p2.is<Base>());
	TEST(!p3.is<Base>());
	TEST(!p4.is<Base>());
}

#undef TEST