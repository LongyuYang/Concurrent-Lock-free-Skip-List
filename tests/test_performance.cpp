#include "glock.hpp"
#include "lock_free.hpp"
#include "pointer_lock.hpp"
#include "CycleTimer.h"

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <array>


#define MAX_THREAD  128
#define OP_SIZE_1M  1000000
#define OP_SIZE_1M5 1500000

struct thread_arg {
    size_t tid;
    size_t total;
    int* keys;
    size_t N_THREAD;
    skip_list<int, int>* s;
};


void print_3lists_time(double glock_time, double plock_time, double free_time) {
    printf("=============================================\n");
    printf("[global lock skip list]: \t\t[%.3f] ms\n", glock_time);
    printf("[pointer lock skip list]:\t\t[%.3f] ms\n", plock_time);
    printf("[lock free skip list]:   \t\t[%.3f] ms\n", free_time);
    printf("=============================================\n");
}


void* thread_insert_data(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;

    for (size_t i = arg->tid; i < arg->total; i += arg->N_THREAD) {
        int num = arg->keys[i];
        s->insert(num, num);
    }

}


void* thread_delete_data(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;

    for (size_t i = arg->tid; i < arg->total; i += arg->N_THREAD) {
        int num = arg->keys[i];
        s->erase(num);
    }
}


void* thread_get_data(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;

    for (size_t i = arg->tid; i < arg->total; i += arg->N_THREAD) {
        int num = arg->keys[i];
        s->get(num);
    }
}


double insert_test(skip_list<int, int>* s, int* keys, size_t total, size_t N_THREAD) {
    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    // init args
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, total, keys, N_THREAD, s};
    }

    double startTime = CycleTimer::currentSeconds();
    // start threads
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, thread_insert_data, &arg[i]);
    }
    thread_insert_data(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    double endTime = CycleTimer::currentSeconds();

    return (endTime - startTime) * 1000;
}


double erase_test(skip_list<int, int>* s, int* keys, size_t total, size_t N_THREAD) {
    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    // init args
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, total, keys, N_THREAD, s};
    }

    double startTime = CycleTimer::currentSeconds();
    // start threads
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, thread_delete_data, &arg[i]);
    }
    thread_delete_data(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    double endTime = CycleTimer::currentSeconds();

    return (endTime - startTime) * 1000;
}


double get_test(skip_list<int, int>* s, int* keys, size_t total, size_t N_THREAD) {
    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    // init args
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, total, keys, N_THREAD, s};
    }

    double startTime = CycleTimer::currentSeconds();
    // start threads
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, thread_get_data, &arg[i]);
    }
    thread_get_data(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    double endTime = CycleTimer::currentSeconds();

    return (endTime - startTime) * 1000;
}


int main() {

    // srand(15618);

    // total operations = 1,000,000

    /*
        50% insert, 50% erase
    */
    
    int *keys = new int[OP_SIZE_1M];
    for (int i = 0; i < OP_SIZE_1M; i++) {
        keys[i] = i;
    }

    std::shuffle(keys, keys + OP_SIZE_1M, std::default_random_engine(15618));

    int *get_keys = new int[OP_SIZE_1M5];
    for (int i = 0; i < OP_SIZE_1M5; i++) {
        get_keys[i] = i;
    }

    std::shuffle(get_keys, get_keys + OP_SIZE_1M5, std::default_random_engine(15418));

    // std::cout << keys[0] << " " << keys[OP_SIZE_1M - 1] << std::endl;

    for (size_t n_thread = 2; n_thread <= MAX_THREAD; n_thread <<= 1) {
        glock_skip_list<int, int> glock_list;
        pointer_lock_skip_list<int, int> plock_list;
        lock_free_skip_list<int, int> free_list;

        double glock_time = insert_test(&glock_list, keys, OP_SIZE_1M, n_thread);
        glock_time += erase_test(&glock_list, keys, OP_SIZE_1M, n_thread);
        assert(glock_list.is_empty());

        double plock_time = insert_test(&plock_list, keys, OP_SIZE_1M, n_thread);
        plock_time += erase_test(&plock_list, keys, OP_SIZE_1M, n_thread);
        assert(plock_list.is_empty());

        double free_time = insert_test(&free_list, keys, OP_SIZE_1M, n_thread);
        free_time += erase_test(&free_list, keys, OP_SIZE_1M, n_thread);
        assert(free_list.is_empty());

        std::cout << " 1 Million Insertion And Deletion With " << n_thread << " Threads.\n";
        print_3lists_time(glock_time, plock_time, free_time);
    }

    
    for (size_t n_thread = 2; n_thread <= MAX_THREAD; n_thread <<= 1) {
        glock_skip_list<int, int> glock_list;
        pointer_lock_skip_list<int, int> plock_list;
        lock_free_skip_list<int, int> free_list;

        double glock_time = insert_test(&glock_list, keys, OP_SIZE_1M, n_thread);
        glock_time += get_test(&glock_list, get_keys, OP_SIZE_1M5, n_thread);
        glock_time += erase_test(&glock_list, keys, OP_SIZE_1M, n_thread);
        assert(glock_list.is_empty());

        double plock_time = insert_test(&plock_list, keys, OP_SIZE_1M, n_thread);
        plock_time += get_test(&plock_list, get_keys, OP_SIZE_1M5, n_thread);
        plock_time += erase_test(&plock_list, keys, OP_SIZE_1M, n_thread);
        assert(plock_list.is_empty());

        double free_time = insert_test(&free_list, keys, OP_SIZE_1M, n_thread);
        free_time += get_test(&free_list, get_keys, OP_SIZE_1M5, n_thread);
        free_time += erase_test(&free_list, keys, OP_SIZE_1M, n_thread);
        assert(free_list.is_empty());

        std::cout << " 1 Million Insertion, Deletion and 1.5 Million Get With " << n_thread << " Threads.\n";
        print_3lists_time(glock_time, plock_time, free_time);
    }

    delete[] keys;
    delete[] get_keys;
    

    return 0;
}

// #include "glock.hpp"
// #include "lock_free.hpp"
// #include "pointer_lock.hpp"
// #include "CycleTimer.h"

// #include <assert.h>
// #include <iostream>
// #include <iomanip>

// struct thread_arg {
//     size_t tid;
//     size_t k;
//     size_t N_THREAD;
//     skip_list<int, int>* s;

//     // specific for high_contention_update
//     size_t update_start;
// };

// void* data_initialize(void* _arg) {
//     thread_arg* arg = (thread_arg*)(_arg);
//     size_t N_THREAD = arg->N_THREAD;
//     skip_list<int, int>* s = arg->s;
//     for (int i = arg->tid; i < arg->k; i += N_THREAD) {
//         s->insert(i, i);
//     }
// }

// void* high_contention_update(void* _arg) {
//     size_t HIGH_CONTENTION_INCREMENT = 100;
//     thread_arg* arg = (thread_arg*)(_arg);
//     size_t index = arg->update_start + arg->tid;
//     for (size_t cnt = 1; cnt <= HIGH_CONTENTION_INCREMENT; cnt++) {
//         int v = arg->s->get(index);
//         arg->s->erase(index);
//         v++;
//         arg->s->insert(index, v);
//     }
// }

// template<class _Key, class _Val>
// double test_init_time(skip_list<_Key, _Val>* s, size_t N_THREAD) {
        
//     pthread_t ts[N_THREAD];
//     thread_arg arg[N_THREAD];

//     size_t k = 1 << 15; // number of unique key values in the set
    
//     // concurrent initialization
//     double startTime = CycleTimer::currentSeconds();
//     for (size_t i = 0; i < N_THREAD; i++) {
//         // arg[i] = {i, k, N_THREAD, &s};
//         arg[i].tid = i;
//         arg[i].k = k;
//         arg[i].N_THREAD = N_THREAD;
//         arg[i].s = s;
//     }
//     for (size_t i = 1; i < N_THREAD; i++) {
//         pthread_create(&ts[i], NULL, data_initialize, &arg[i]);
//     }
//     data_initialize(&arg[0]);
//     for (size_t i = 1; i < N_THREAD; i++) {
//         pthread_join(ts[i], NULL);
//     }
//     double endTime = CycleTimer::currentSeconds();

//     return (endTime - startTime) * 1000;
// }

// template<class _Key, class _Val>
// double test_update_time(skip_list<_Key, _Val>* s, size_t N_THREAD) {
//     pthread_t ts[N_THREAD];
//     thread_arg arg[N_THREAD];

//     size_t k = 1 << 15;

//     double startTime = CycleTimer::currentSeconds();

//     for (size_t start = 0; start < k; start += N_THREAD) {
//         for (size_t i = 0; i < N_THREAD; i++) {
//             arg[i].tid = i;
//             arg[i].k = k;
//             arg[i].N_THREAD = N_THREAD;
//             arg[i].s = s;
//             arg[i].update_start = start;
//         }
//         for (size_t i = 1; i < N_THREAD; i++) {
//             pthread_create(&ts[i], NULL, high_contention_update, &arg[i]);
//         }
//         high_contention_update(&arg[0]);
//         for (size_t i = 1; i < N_THREAD; i++) {
//             pthread_join(ts[i], NULL);
//         }
//     }

//     double endTime = CycleTimer::currentSeconds();

//     return (endTime - startTime) * 1000;
// }

// void print_3lists_time(double glock_time, double plock_time, double free_time) {
//     printf("=============================================\n");
//     printf("[global lock skip list]: \t\t[%.3f] ms\n", glock_time);
//     printf("[pointer lock skip list]:\t\t[%.3f] ms\n", plock_time);
//     printf("[lock free skip list]:   \t\t[%.3f] ms\n", free_time);
//     printf("=============================================\n");
// }

// int main() {

//     srand(15618);

//     // computation time
//     // init time
//     size_t N_THREAD = 8;
//     glock_skip_list<int, int> glock_list;
//     pointer_lock_skip_list<int, int> plock_list;
//     lock_free_skip_list<int, int> free_list;

//     double glock_time = test_init_time(&glock_list, N_THREAD);
//     double plock_time = test_init_time(&plock_list, N_THREAD);
//     double free_time = test_init_time(&free_list, N_THREAD);

//     std::cout << "===== test init time =====\n";
//     print_3lists_time(glock_time, plock_time, free_time);

//     // update time
//     glock_time = test_update_time(&glock_list, N_THREAD);
//     plock_time = test_update_time(&plock_list, N_THREAD);
//     free_time = test_update_time(&free_list, N_THREAD);

//     std::cout << "===== test update time =====\n";
//     print_3lists_time(glock_time, plock_time, free_time);

//     return 0;
// }