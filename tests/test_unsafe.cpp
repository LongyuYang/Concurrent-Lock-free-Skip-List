#include "unsafe.hpp"

#include <map>

int main() {
    srand(15618);

    // our thread-unsafe skip list
    unsafe_skip_list<int, int> s;

    // reference
    std::map<int, int> s_ref;

    int k = 1 << 19; // average number of unique key values in the set
    int n = 1 << 20; // operation number

    // initialization
    for (int i = 0; i < k; i++) {
        s.insert(k, k);
        s_ref.insert(std::pair<int, int>(k, k));
    }

    // perform operations
    for (int i = 0; i < n; i++) {
        int rand_key = rand() % k;
        int rand_opt = rand() % 1000 + 1;
 
        // 75% lookup
        if (rand_opt <= 750) {
            int val = s.get(rand_key);
            int val_ref = s_ref[rand_key];

            if (val != val_ref) {
                std::cout << "Fail: value mismatch for key=" << rand_key << std::endl;

                exit(1);
            }
        }

        // 12.5% update
        else if (rand_opt > 750 && rand_opt <= 875) {
            int rand_val = rand() % INT32_MAX;
            s.insert(rand_key, rand_val);
            s_ref[rand_key] = rand_val;
        }

        // 12.5% remove
        else {
            s.erase(rand_key);
            s_ref.erase(rand_key);
        }
    }
    
    std::cout << "passed" << std::endl;

    return 0;
}