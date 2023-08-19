#include <iostream>
#include "log.h"
#include <iostream>
#include <memory>
#include "util.h"

class C {
public:
    int value;

    C(int val) : value(val) {}
};

class A {
public:
    std::shared_ptr<C> a;

    A(std::shared_ptr<C>& ptr) : a(ptr) {}
};

class B {
public:
    std::shared_ptr<C> b;

    B(std::shared_ptr<C>& ptr) : b(ptr) {}
};

int main(int argc, char *argv[])
{
    
   std::shared_ptr<C> ptr = std::make_shared<C>(42);

    A objectA(ptr);
    B objectB(ptr);

    std::cout << "Value in A: " << objectA.a->value << std::endl;
    std::cout << "Value in B: " << objectB.b->value << std::endl;

    std::shared_ptr<C> newPtr = std::make_shared<C>(100);
    objectA.a = newPtr; // 修改 A 中的指针

    std::cout << "Value in A after change: " << objectA.a->value << std::endl;
    std::cout << "Value in B after change: " << objectB.b->value << std::endl;

    return 0;
}
