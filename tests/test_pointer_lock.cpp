#include "pointer_lock.hpp"
#include "unsafe.hpp"

#include <map>
#include <omp.h>

#define N_THREAD 8

int main() {
    srand(15618);

    omp_set_num_threads(N_THREAD);

    // our per-pointer skip list
    pointer_lock_skip_list<int, int> s;

    // reference
    // std::map<int, int> s_ref;

    int k = 1 << 20; // number of unique key values in the set

    // concurrent initialization
#pragma omp parallel for default(shared) schedule(dynamic)
    for (int i = 0; i < k; i++) {
        s.insert(i, i);
    }

    // check correctness
#pragma omp parallel for default(shared) schedule(dynamic)
    for (int i = 0; i < k; i++) {
        int val = s.get(i);
        if (val != i) {
            std::cout << "Fail: value mismatch for key=" << i << std::endl;
            exit(1);
        }
    }

    std::cout << "passed" << std::endl;

    return 0;
}