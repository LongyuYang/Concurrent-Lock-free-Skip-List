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
#include <vector>

#define MAX_THREAD 128
#define OPT_SIZE 1000000
#define DATA_RANGE 2000000

#define INSERT_PROPORTION 20
#define ERASE_PROPORTION 10
#define GET_PROPORTION 70

enum OPT_TYPE {
    INSERT = 0,
    ERASE,
    GET
};

struct opt {
    int key;
    OPT_TYPE opt_type;
};

struct thread_arg {
    size_t tid;
    size_t total;
    std::vector<opt>* opts;
    size_t N_THREAD;
    skip_list<int, int>* s;
};


void print_3lists_throughput(long long n_threads, double glock_time, double plock_time, double free_time) {
    printf("=============================================\n");
    printf("[global lock skip list]: \t\t[%.3f] \n", double(OPT_SIZE*n_threads)/glock_time);
    printf("[pointer lock skip list]:\t\t[%.3f] \n", double(OPT_SIZE*n_threads)/plock_time);
    printf("[lock free skip list]:   \t\t[%.3f] \n", double(OPT_SIZE*n_threads)/free_time);
    printf("=============================================\n");
}


void* thread_execution(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;
    std::vector<opt> opts = *arg->opts;

    for (size_t i = 0; i < OPT_SIZE; i++) {
        switch (opts[i].opt_type)
        {
        case OPT_TYPE::INSERT:
            s->insert(opts[i].key, opts[i].key);
            break;
        case OPT_TYPE::ERASE:
            s->erase(opts[i].key);
            break;
        case OPT_TYPE::GET:
            s->get(opts[i].key);
            break;
        
        default:
            break;
        }
    }
}


double start_test(skip_list<int, int>* s, size_t total, size_t N_THREAD) {
    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    // assign operatons
    std::vector<std::vector<opt>> opts(N_THREAD, std::vector<opt>(OPT_SIZE));
    for (size_t n = 0; n < N_THREAD; n++) {
        for (size_t i = 0; i < OPT_SIZE; i++) {
            // assign a random key selected from range
            int num = rand() % DATA_RANGE;

            // assign operation type based on proportions
            int rand_num = rand() % 100;
            if (rand_num < INSERT_PROPORTION) {
                opts[n][i] = opt{num, OPT_TYPE::INSERT};
            } else if (rand_num >= INSERT_PROPORTION && rand_num < INSERT_PROPORTION + ERASE_PROPORTION) {
                opts[n][i] = opt{num, OPT_TYPE::ERASE};
            } else {
                opts[n][i] = opt{num, OPT_TYPE::GET};
            }
        }
    }

    // init args
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, total, &opts[i], N_THREAD, s};
    }

    double startTime = CycleTimer::currentSeconds();
    // start threads
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, thread_execution, &arg[i]);
    }
    thread_execution(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    double endTime = CycleTimer::currentSeconds();

    // clear data
    for (size_t i = 0; i < DATA_RANGE; i++) {
        s->erase(i);
    }

    return (endTime - startTime) * 1000;
}


int main() {
    assert(INSERT_PROPORTION + ERASE_PROPORTION + GET_PROPORTION == 100);

    for (size_t n_threads = 1; n_threads <= MAX_THREAD; n_threads <<= 1) {
        glock_skip_list<int, int> glock_list;
        pointer_lock_skip_list<int, int> plock_list;
        lock_free_skip_list<int, int> free_list;

        double glock_time, plock_time, free_time;

        printf("start testing global lock...\n");
        glock_time = start_test(&glock_list, OPT_SIZE, n_threads);

        printf("start testing per pointer lock...\n");
        plock_time = start_test(&plock_list, OPT_SIZE, n_threads);

        printf("start testing lock free...\n");
        free_time = start_test(&free_list, OPT_SIZE, n_threads);

        printf("%d operations with %d%% insert, %d%% erase, %d%% get.\n", 
                OPT_SIZE, INSERT_PROPORTION, ERASE_PROPORTION, GET_PROPORTION);
        printf("num threads=%d, data range=%d.\n", n_threads, DATA_RANGE);

        print_3lists_throughput(n_threads, glock_time, plock_time, free_time);
    }
    
    return 0;
}