#include <iostream>

using namespace std;

class X;
void F(X &x, int n);

class X {
public:
    X() {
        try {
            F(*this, -2);
            cout << 1 << endl;
        } catch (X) {
            cout << 2 << endl;
        } catch (int) {
            cout << 3 << endl;
        }
    }
    X(X &) { cout << 12 << endl; }
};

class Y : public X {
public:
    Y() { cout << 4 << endl; }
    Y(Y &a) { cout << 5 << endl; }
    ~Y() { cout << 6 << endl; }
};

void F(X &x, int n) {
    try {
        if (n < 0) throw x;
        if (n > 10) throw 1;
        cout << 7 << endl;
    } catch (int) {
        cout << 8 << endl;
    } catch (X &) {
        cout << 9 << endl;
        throw;
    }
}

int main() {
    try {
        Y a;
    } catch (...) {
        cout << 10 << endl;
    }
    cout << 11 << endl;
    return 0;
}
/*
    12
    9
    12
    2
    4
    6
    11
*/
