#include <iostream>
#include <iomanip>
#include "tests.hpp"
#include "poly.hpp"
#include "poly_factory.hpp"

using namespace zhukov;
using namespace std;

#define TEST(...) print_test_result(#__VA_ARGS__, __VA_ARGS__)

void Tester::print_test_result(const std::string& name, bool result) {
	string spacer(80 - 5 - name.size(), ' ');
	cerr << std::boolalpha <<
		name << spacer << result << endl;
}

void Tester::run() {
	// poly ====================================================================
	{
		poly<Base> p0(new Der);
		poly<Base> p1(std::move(p0));
		auto p2 = make_poly<Base, Der>();
		auto p3 = transform_poly<Mid1, Der>(p2);
		auto p4 = transform_poly<Mid2, deep_copy, Der>(p3);
		auto p5 = p4;
		
		TEST(!static_cast<bool>(p0));

		TEST(p1->name() == "der");
		TEST(p2->name() == "der");
		TEST(p3->name() == "der");
		TEST(p4->name() == "der");

		TEST(p1.is<Der>());
		TEST(p2.is<Der>());
		TEST(p3.is<Der>());
		TEST(p4.is<Der>());

		TEST(!p1.is<Base>());
		TEST(!p2.is<Base>());
		TEST(!p3.is<Base>());
		TEST(!p4.is<Base>());

		TEST(p1.as<Der>() != nullptr);
		TEST(p2.as<Der>() != nullptr);
		TEST(p3.as<Der>() != nullptr);
		TEST(p4.as<Der>() != nullptr);

		TEST(p1.as<Der>()->name() == "der");
		TEST(p2.as<Der>()->name() == "der");
		TEST(p3.as<Der>()->name() == "der");
		TEST(p4.as<Der>()->name() == "der");

		TEST(p1.as<Base>() == nullptr);
		TEST(p2.as<Base>() == nullptr);
		TEST(p3.as<Mid1>() == nullptr);
		TEST(p4.as<Mid2>() == nullptr);

		TEST(p1 == p1);
		TEST(p1 != p2);
		TEST((p1 < p2) ^ (p1 > p2));
		TEST((p1 <= p2) ^ (p1 >= p2));

		TEST(p0 == nullptr);
		TEST(nullptr == p0);
		TEST(nullptr != p1);
		TEST(p1 != nullptr);

		TEST(!(p0 < nullptr));
		TEST(!(nullptr < p0));
		TEST(!(nullptr > p0));
		TEST(!(p0 > nullptr));

		TEST(p0 <= nullptr);
		TEST(nullptr <= p0);
		TEST(nullptr >= p0);
		TEST(p0 >= nullptr);

		TEST(std::hash<poly<Base>>()(p1) == std::hash<Base*>()(p1.get()));
	}

	// poly_factory ============================================================
	{
		factory<Base> f;
		f.insert<Der>();
		f.insert<Mid1>();
		f.insert<Mid2>();
		f.insert<Base>();

		auto p1 = f.make(typeid(Der).name());
		auto p2 = f.make(typeid(Mid1).name());
		auto p3 = f.make(typeid(Mid2).name());
		auto p4 = f.make(typeid(Base).name());

		TEST(p1.get()->name() == "der");
		TEST(p2.get()->name() == "mid1");
		TEST(p3.get()->name() == "mid2");
		TEST(p4.get()->name() == "base");

		TEST(p1.is<Der>());
		TEST(p2.is<Mid1>());
		TEST(p3.is<Mid2>());
		TEST(p4.is<Base>());

		TEST(!p1.is<Base>());
		TEST(!p2.is<Base>());
		TEST(!p3.is<Base>());
	}
}

#undef TEST