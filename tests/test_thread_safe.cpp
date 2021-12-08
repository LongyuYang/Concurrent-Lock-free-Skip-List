#include "lock_free.hpp"
#include "glock.hpp"
#include "pointer_lock.hpp"

#include <assert.h>
#include <getopt.h>

#define N_THREAD 8

// number of high contention increment to each position
#define HIGH_CONTENTION_INCREMENT 100

enum VERSION_TYPE {
    GLOCK = 0,
    PLOCK,
    LOCK_FREE
};

struct thread_arg {
    size_t tid;
    size_t k;
    skip_list<int, int>* s;

    // specific for high_contention_update
    size_t update_start;
};

void* data_initialize(void* _arg) {
    thread_arg* arg = (thread_arg*)(_arg);
    skip_list<int, int>* s = arg->s;
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
    skip_list<int, int>* s = arg->s;
    for (int i = arg->tid; i < arg->k; i += N_THREAD) {
        s->erase(i);
    }
}

int main(int argc, char** argv) {
        int opt;
    static struct option long_options[] = {
        {"help",     0, 0,  'h'},
        {"version",  1, 0,  'v'},
        {0 ,0, 0, 0}
    };

    int n_threads;
    VERSION_TYPE version;

    // start parsing arguments
    while ((opt = getopt_long(argc, argv, "v:h", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'v':
            version = VERSION_TYPE(atoi(optarg));
            break;
        case 'h':
        default:
            printf("  -t  --thread  <INT> number of threads\n");
            printf("  -v  --version <INT> 0: global lock, 1: pointer lock, 2: lock free\n");
            printf("  -h  --help    This message\n");
            return 1;
        }
    }

    assert(version >= 0 && version <= 2);
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

    srand(15618);

    pthread_t ts[N_THREAD];
    thread_arg arg[N_THREAD];

    size_t k = 1 << 15; // number of unique key values in the set
    
    // concurrent initialization
    for (size_t i = 0; i < N_THREAD; i++) {
        arg[i] = {i, k, s};
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
        arg[i] = {i, k, s};
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
            arg[i] = {i, k, s, start};
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
        arg[i] = {i, k, s};
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
        arg[i] = {i, k, s};
    }
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_create(&ts[i], NULL, data_clear, &arg[i]);
    }
    data_clear(&arg[0]);
    for (size_t i = 1; i < N_THREAD; i++) {
        pthread_join(ts[i], NULL);
    }

    // check if it is empty
    assert(s->is_empty());

    std::cout << "Passed!" << std::endl;

    delete s;

    return 0;
}
