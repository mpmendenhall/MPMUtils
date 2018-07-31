/// \file TestOperators.hh templates to test out operators

#include <iostream>
using std::cout;

/// display effects of in- and out-of-place addition
template<class T>
void testAdd(T& a, T& b) {
    cout << a << " + " << b << " = ";
    a += b;
    cout << a << ";\n";
    cout << a << " + " << b << " = " << a + b << ".\n\n";
}

/// display effects of in- and out-of-place multiplication
template<class T>
void testMul(T& a, T& b) {
    cout << a << " * " << b << " = ";
    a *= b;
    cout << a << ";\n";
    cout << a << " * " << b << " = " << a * b << ".\n\n";
}
