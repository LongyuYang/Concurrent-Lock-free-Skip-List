#include "CycleTimer.h"
#include "glock.hpp"
#include "lock_free.hpp"
#include "pointer_lock.hpp"

#include <getopt.h>

#include <assert.h>
#include <iostream>
#include <random>

// number of operations per thread
#define OPT_SIZE 1000000

// data range of the skip list
#define DATA_RANGE 2000000

// proportions for insert, erase, and get
#define INSERT_PROPORTION 50
#define ERASE_PROPORTION 50
#define GET_PROPORTION 0

#define NUM_ROUNDS 3

enum VERSION_TYPE { GLOCK = 0, PLOCK, LOCK_FREE };

enum OPT_TYPE { INSERT = 0, ERASE, GET };

struct opt {
    int key;
    OPT_TYPE opt_type;
};

struct thread_arg {
    size_t tid;
    size_t total;
    std::vector<opt> *opts;
    size_t N_THREAD;
    skip_list<int, int> *s;
};

const char *msg[3] = {"global  lock", "pointer lock", "lock    free"};

void print_throughput(VERSION_TYPE version, size_t n_threads,
                      double execution_time) {
    printf("[%s skip list with %lu threads]: \t\t[%.3f] opt/ms\n",
           msg[int(version)], n_threads,
           double(OPT_SIZE * n_threads) / execution_time);
}

void *thread_execution(void *_arg) {
    thread_arg *arg = (thread_arg *)(_arg);
    skip_list<int, int> *s = arg->s;
    std::vector<opt> opts = *arg->opts;

    for (size_t i = 0; i < OPT_SIZE; i++) {
        switch (opts[i].opt_type) {
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

double start_test(skip_list<int, int> *s, size_t total, size_t N_THREAD) {
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
            } else if (rand_num >= INSERT_PROPORTION &&
                       rand_num < INSERT_PROPORTION + ERASE_PROPORTION) {
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

int main(int argc, char **argv) {
    assert(INSERT_PROPORTION + ERASE_PROPORTION + GET_PROPORTION == 100);

    int opt;
    static struct option long_options[] = {{"help", 0, 0, 'h'},
                                           {"thread", 1, 0, 't'},
                                           {"version", 1, 0, 'v'},
                                           {0, 0, 0, 0}};

    int n_threads;
    VERSION_TYPE version;

    // start parsing arguments
    while ((opt = getopt_long(argc, argv, "t:v:h", long_options, NULL)) !=
           EOF) {
        switch (opt) {
        case 't':
            n_threads = atoi(optarg);
            break;
        case 'v':
            version = VERSION_TYPE(atoi(optarg));
            break;
        case 'h':
        default:
            printf("  -t  --thread  <INT> number of threads\n");
            printf("  -v  --version <INT> 0: global lock, 1: pointer lock, 2: "
                   "lock free\n");
            printf("  -h  --help    This message\n");
            return 1;
        }
    }

    assert(version >= 0 && version <= 2);
    assert(n_threads >= 1);

    double execution_time = 0.f;

    // perform several rounds and get the average execution time
    for (size_t i = 1; i <= NUM_ROUNDS; i++) {
        skip_list<int, int> *s;
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

        execution_time += start_test(s, OPT_SIZE, n_threads);

        delete s;
    }

    execution_time /= NUM_ROUNDS;
    print_throughput(version, n_threads, execution_time);

    return 0;
}
