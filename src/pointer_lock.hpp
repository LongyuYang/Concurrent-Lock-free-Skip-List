#ifndef _POINTER_LOCK_SKIP_LIST_H
#define _POINTER_LOCK_SKIP_LIST_H

#include "base.hpp"

template<class _Key, class _Val>
class pointer_lock_skip_list : public skip_list<_Key, _Val> {
private:
    typedef skip_list_node<_Key, _Val, true> _node;

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

    _node* get_lock(_node* x, _Key key, int i) {
        _node* y = x->next[i];
        while (*y < key) {
            x = y;
            y = x->next[i];
        }

        x->lock(i);
        y = x->next[i];
        while (*y < key) {
            x->unlock(i);
            x = y;
            
            x->lock(i);
            y = x->next[i];
        }

        return x;
    }
public:
    pointer_lock_skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) : 
    skip_list<_Key, _Val, true>(_max_level) {}

    virtual ~pointer_lock_skip_list() {}

    virtual _Val get(_Key key) {
        _node** left = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node* target = search(left, right, key);
        GC_FREE(left);
        GC_FREE(right);
        return *target == key ? target->val : _Val();
    };

    virtual void insert(_Key key, _Val val) {
        _node** left = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node* target = search(left, right, key); 
        _node* x = left[0];
        
        x = get_lock(x, key, 0);
        if (*(x->next[0]) == key) {
            x->next[0]->val = val;
            x->unlock(0);
            GC_FREE(left);
            GC_FREE(right);
            return;
        }

        size_t new_level = this->rand_level();
        _node* new_node = (_node*)GC_MALLOC(sizeof(_node));

        new_node->init(new_level);
        new_node->set_key_val(key, val);

        new_node->lock(new_level);
        for (size_t i = 0; i < new_level; i++) {
            if (i != 0) {
                x = get_lock(left[i], key, i);
            }

            new_node->next[i] = x->next[i];
            x->next[i] = new_node;
            x->unlock(i);
        }
        new_node->unlock(new_level);

        GC_FREE(left);
        GC_FREE(right);
    };
    
    virtual void erase(_Key key) {
        _node** left = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node* target = search(left, right, key);
        _node* y = left[0];

        bool is_garbage;
        do {
            y = y->next[0];
            if (*y > key) {
                GC_FREE(left);
                GC_FREE(right);
                return;
            }
            y->lock(y->level);
            is_garbage = *y > *(y->next[0]);
            if (*y == key && !is_garbage) {
                break;
            }

            y->unlock(y->level);
        } while (true);

        for (ssize_t i = y->level - 1; i >= 0; i--) {
            _node* x = get_lock(left[i], key, i);
            y->lock(i);
            x->next[i] = y->next[i];
            y->next[i] = x;
            x->unlock(i);
            y->unlock(i);
        }
        y->unlock(y->level);

        GC_FREE(left);
        GC_FREE(right);
    };
};

#endif