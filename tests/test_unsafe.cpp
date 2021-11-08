#include "unsafe.hpp"

int main() {
    unsafe_skip_list<int, int> s;

    for (int i = 0; i < 10; i++) {
        s.insert(i, 99);
    }

    s.debug_print();

    return 0;
}