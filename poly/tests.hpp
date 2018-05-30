#pragma once
#include <string>

class Tester {
private:
	static void print_test_result(const std::string& name, bool result);

public:
	void run();
};

struct Base {
	std::string base_name;

	virtual std::string get() { return base_name; }
	Base() : base_name("base") {}
	virtual ~Base() = default;
};

struct Mid1 : virtual Base {
	std::string mid1_name;

	virtual std::string get() { return mid1_name; }
	Mid1() : mid1_name("mid1") {}
	virtual ~Mid1() = default;
};

struct Mid2 : virtual Base {
	std::string mid2_name;

	virtual std::string get() { return mid2_name; }
	Mid2() : mid2_name("mid2") {}
	virtual ~Mid2() = default;
};

struct Mid3 : virtual Base {
	std::string mid3_name;

	virtual std::string get() { return mid3_name; }
	Mid3() : mid3_name("mid3") {}
	virtual ~Mid3() = default;
};

struct Der : Mid1, Mid2 {
	std::string der_name;

	virtual std::string get() { return der_name; }
	Der() : der_name("der") {}
};
