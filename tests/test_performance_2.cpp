#include "glock.hpp"
#include "lock_free.hpp"
#include "pointer_lock.hpp"
#include "CycleTimer.h"

#include <getopt.h>

#include <assert.h>
#include <iostream>
#include <random>

// number of seconds execution per thread
const double OPT_SECONDS = 10.f;

// proportions for insert, erase, and get
#define INSERT_PROPORTION 125
#define ERASE_PROPORTION 125
#define GET_PROPORTION 750

enum VERSION_TYPE {
    GLOCK = 0,
    PLOCK,
    LOCK_FREE
};

struct thread_arg {
    size_t tid;
    double start_time;
    size_t N_THREAD;
    int data_range;
    skip_list<int, int>* s;

    // return value
    double num_opts;
};

const char* msg[3] = {
    "global  lock",
    "pointer lock",
    "lock    free"
};

void print_time_per_operation(VERSION_TYPE version, int n_threads, int power, double total_opts) {
    printf("[%s skip list with %d threads and 2^%d range]: \t\t[%.3f] us/opt\n", 
    msg[int(version)], n_threads, power, 1000000.f * OPT_SECONDS / total_opts);
    // printf("%.3f\t", 1000000.f * OPT_SECONDS / total_opts);
}

void* thread_execution(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;
    double num_opts = 0.f;

    // each thread executes 10s
    double endTime = CycleTimer::currentSeconds();
    while (endTime - arg->start_time <= OPT_SECONDS) {
        // assign a random number from data range
        int num = rand() % arg->data_range;

        // assign a random operation based on proportions
        int rand_num = rand() % 1000;
        if (rand_num < INSERT_PROPORTION) {
            s->insert(num, num);
        } else if (rand_num >= INSERT_PROPORTION && rand_num < INSERT_PROPORTION + ERASE_PROPORTION) {
            s->erase(num);
        } else {
            s->get(num);
        }

        endTime = CycleTimer::currentSeconds();
        num_opts += 1;
    }

    arg->num_opts = num_opts;

    return nullptr;
}


double start_test(skip_list<int, int>* s, size_t N_THREAD, int data_range) {
    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    // init data
    for (int i = 0; i < data_range; i++) {
        s->insert(i, i);
    }

    double startTime = CycleTimer::currentSeconds();

    // init args
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, startTime, N_THREAD, data_range, s};
    }

    // start threads
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, thread_execution, &arg[i]);
    }
    thread_execution(&arg[0]);

    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    // clear data
    for (size_t i = 0; i < data_range; i++) {
        s->erase(i);
    }

    // sum total operations
    double total_opts = 0.f;
    for (int i = 0; i < N_THREAD; i++) {
        total_opts += arg[i].num_opts;
    }

    return total_opts;
}


int main(int argc, char** argv) {
    assert(INSERT_PROPORTION + ERASE_PROPORTION + GET_PROPORTION == 1000);

    int opt;
    static struct option long_options[] = {
        {"help",     0, 0,  'h'},
        {"thread",   1, 0,  't'},
        {"version",  1, 0,  'v'},
        {"range",    1, 0,  'r'},
        {0 ,0, 0, 0}
    };

    int n_threads;
    VERSION_TYPE version;
    int data_range;
    int power;

    // start parsing arguments
    while ((opt = getopt_long(argc, argv, "t:v:r:h", long_options, NULL)) != EOF) {
        switch (opt) {
        case 't':
            n_threads = atoi(optarg);
            break;
        case 'v':
            version = VERSION_TYPE(atoi(optarg));
            break;
        case 'r':
            power = (atoi(optarg));
            data_range = 1 << power;
            break;
        case 'h':
        default:
            printf("  -t  --thread  <INT> number of threads\n");
            printf("  -v  --version <INT> 0: global lock, 1: pointer lock, 2: lock free\n");
            printf("  -r  --range   <INT> data range: 2^r\n");
            printf("  -h  --help    This message\n");
            return 1;
        }
    }

    assert(version >= 0 && version <= 2);
    assert(n_threads >= 1);

    double total_opts;
    
    skip_list<int, int>* s;
    switch (version) {
    case VERSION_TYPE::GLOCK:
        s = new glock_skip_list<int, int>();
        break;
    case VERSION_TYPE::PLOCK:
        s = new pointer_lock_skip_list<int, int>();
        break;
    case VERSION_TYPE::LOCK_FREE:
        s = new lock_free_skip_list<int, int>();
        break;
    default:
        printf("unknown skip list version\n");
        return 1;
    }

    total_opts = start_test(s, n_threads, data_range);

    delete s;

    print_time_per_operation(version, n_threads, power, total_opts);
    
    return 0;
}
