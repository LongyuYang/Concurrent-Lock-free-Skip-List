#ifndef _UNSAFE_SKIP_LIST_H
#define _UNSAFE_SKIP_LIST_H

#include "base.hpp"

#include <vector>

template <class _Key, class _Val>
class unsafe_skip_list : public skip_list<_Key, _Val> {
  private:
    typedef skip_list_node<_Key, _Val> _node;

    std::vector<_node *> left;
    std::vector<_node *> right;

    _node *search(_Key key) {
        _node *x = this->header;
        _node *y = nullptr;
        for (ssize_t i = this->max_level - 1; i >= 0; i--) {

            while (true) {
                y = x->next[i];
                if (!(*y < key))
                    break;
                x = y;
            }
            left[i] = x, right[i] = y;
        }

        return y;
    }

  public:
    unsafe_skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL)
        : skip_list<_Key, _Val>(_max_level) {
        left.resize(this->max_level);
        right.resize(this->max_level);
    }

    virtual ~unsafe_skip_list() {}

    virtual _Val get(_Key key) {
        _node *target = search(key);
        return *target == key ? target->val : _Val();
    };

    virtual void insert(_Key key, _Val val) {
        size_t new_level = this->rand_level();
        _node *new_node = (_node *)GC_MALLOC(sizeof(_node));
        new_node->init(new_level);
        new_node->set_key_val(key, val);

        _Val *old_val = NULL;

        _node *target = search(key);
        if (*target == key) {
            old_val = &target->val;
            target->val = val;
        } else {

            for (size_t i = 0; i < new_level; i++) {
                left[i]->next[i] = new_node;
                new_node->next[i] = right[i];
            }
        }

        if (old_val != NULL) {
            GC_FREE(new_node);
        }
    };

    virtual void erase(_Key key) {
        _node *target = search(key);
        if (*target == key) {
            for (size_t i = 0; i < target->level; i++) {
                left[i]->next[i] = target->next[i];
            }

            GC_FREE(target);
        }
    };
};

#endif