#ifndef _UNSAFE_SKIP_LIST_H
#define _UNSAFE_SKIP_LIST_H

#include "base.hpp"

template<class _Key, class _Val>
class unsafe_skip_list : public skip_list<_Key, _Val> {
private:
    typedef skip_list_node<_Key, _Val> _node;

    _node* search(_node** left, _node** right, _Key key) {
        _node* x = this->header;
        _node* y;

        for (ssize_t i = this->max_level - 1; i >= 0; i--) {
            while (true) {
                y = x->next[i];
                if (!(*y < key)) break;
                x = y;
            }
            left[i] = x, right[i] = y;
        }

        return y;
    }
public:
    unsafe_skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) : 
    skip_list<_Key, _Val>(_max_level) {}

    virtual ~unsafe_skip_list() {}

    virtual _Val get(_Key key) {
        _node** left = new _node*[this->max_level];
        _node** right = new _node*[this->max_level];

        _node* target = search(left, right, key);

        delete[] left;
        delete[] right;

        return *target == key ? target->val : _Val();
    };

    virtual void insert(_Key key, _Val val) {
        size_t new_level = this->rand_level();
        _node* new_node = new _node(new_level);
        new_node->set_key_val(key, val);

        _node** left = new _node*[this->max_level];
        _node** right = new _node*[this->max_level];

        _Val* old_val = NULL;

        _node* target = search(left, right, key);
        if (*target == key) {
            old_val = &target->val;
            target->val = val;
        } else {
            std::cout << new_level << std::endl;
            for (size_t i = 0; i < new_level; i++) {
                left[i]->next[i] = new_node;
                new_node->next[i] = right[i];
                std::cout << i << "\n";
            }
        }
        
        if (old_val != NULL) {
            delete new_node;
        }

        delete[] left;
        delete[] right;

        std::cout << "*********\n";
    };
    
    virtual void erase(_Key key) {};
};

#endif