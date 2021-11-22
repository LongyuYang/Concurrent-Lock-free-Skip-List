
#include "glock.hpp"
#include "unsafe.hpp"
#include <omp.h>
#include <map>

#include <assert.h>

#define N_THREAD 8
#define HIGH_CONTENTION_INCREMENT 100

struct thread_arg {
    size_t tid;
    size_t k;
    glock_skip_list<int, int>* s;

    // specific for high_contention_update
    size_t update_start;
};

void* data_initialize(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    glock_skip_list<int, int>* s = arg->s;
    for (int i = arg->tid; i < arg->k; i += N_THREAD) {
        s->insert(i, i);
    }
}

void* high_contention_update(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    size_t index = arg->update_start + arg->tid;
    for (size_t cnt = 1; cnt <= HIGH_CONTENTION_INCREMENT; cnt++) {
        int v = arg->s->get(index);
        arg->s->erase(index);
        v++;
        arg->s->insert(index, v);
    }
}

void* check_init(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    for (int i = arg->tid; i < arg->k; i += N_THREAD) {
        int val = arg->s->get(i);
        
        if (val != i) {
            printf("Error: value mismacthed at key=%d, val=%d, expected=%d\n", i, val, i);
            exit(1);
        }
    }
}

void* check_correctness(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    for (int i = arg->tid; i < arg->k; i += N_THREAD) {
        int val = arg->s->get(i);
        if (val != i + HIGH_CONTENTION_INCREMENT) {
            printf("Error: value mismacthed at key=%d, val=%d, expected=%d\n", i, val, i+HIGH_CONTENTION_INCREMENT);
            exit(1);
        }
    }
}

void* data_clear(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    glock_skip_list<int, int>* s = arg->s;
    for (int i = arg->tid; i < arg->k; i += N_THREAD) {
        s->erase(i);
    }
}

int main() {

    srand(15618);

    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];
    glock_skip_list<int, int> s;

    size_t k = 1 << 15; // number of unique key values in the set
    
    // concurrent initialization
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, k, &s};
    }
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, data_initialize, &arg[i]);
    }
    data_initialize(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    std::cout << "Initialize successfully!" << std::endl;
    std::cout << "Checking initialization..." << std::endl;

    // check initialization correctness
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, k, &s};
    }
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, check_init, &arg[i]);
    }
    check_init(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    std::cout << "Performing high contention update..." << std::endl;

    // high contention update
    for (size_t start = 0; start < k; start += N_THREAD) {
        for (size_t i = 0; i < N_THREAD; i++) {
            arg[i] = {i, k, &s, start};
        }
        for (size_t i = 1; i < N_THREAD; i++) {
            pthread_create(&ts[i], NULL, high_contention_update, &arg[i]);
        }
        high_contention_update(&arg[0]);
        for (size_t i = 1; i < N_THREAD; i++) {
            pthread_join(ts[i], NULL);
        }
    }

    std::cout << "High contention update finished!" << std::endl;
    std::cout << "Checking correctness..." << std::endl;

    // check correctness
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, k, &s};
    }
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, check_correctness, &arg[i]);
    }
    check_correctness(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    // concurrently remove all elements
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, k, &s};
    }
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, data_clear, &arg[i]);
    }
    data_clear(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    // check if it is empty
    assert(s.is_empty());

    std::cout << "Passed!" << std::endl;

    return 0;
}

// int main() {
//     srand(15618);

//     omp_set_num_threads(N_THREAD);

//     // our per-pointer skip list
//     glock_skip_list<int, int> s;

//     // reference
//     // std::map<int, int> s_ref;

//     int k = 1 << 12; // number of unique key values in the set

//     // concurrent initialization
// #pragma omp parallel for default(shared) schedule(static)
//     for (int i = 0; i < k; i++) {
//         s.insert(i, i);
//     }

//     // high contention update
//     for (int i = 0; i < k; i += N_THREAD) {
// #pragma omp parallel for default(shared) schedule(dynamic)
//         for (int j = i; j < std::min(k, i + N_THREAD); j++) {
//             // increment to val + 10
//             for (int cnt = 1; cnt <= 10; cnt++) {
//                 int v = s.get(j);
//                 s.erase(j);
//                 v++;
//                 s.insert(j, v);
//             }
//         }
//     }

//     // check correctness
// #pragma omp parallel for default(shared) schedule(dynamic)
//     for (int i = 0; i < k; i++) {
//         int val = s.get(i);
//         if (val != i + 10) {
//             std::cout << "Fail: value mismatch for key=" << i << std::endl;
//             exit(1);
//         }
//     }

//     std::cout << "passed" << std::endl;

//     return 0;
// }
