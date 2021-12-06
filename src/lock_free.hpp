#ifndef _LOCK_FREE_SKIP_LIST_H
#define _LOCK_FREE_SKIP_LIST_H

#include "base.hpp"

#define CAS(ptr, old_val, new_val) __sync_val_compare_and_swap(ptr, old_val, new_val)

template<class _Key, class _Val>
class lock_free_skip_list : public skip_list<_Key, _Val> {
private:
    typedef skip_list_node<_Key, _Val> _node;

    // pointer lowest bit markint functions
    _node* mark(_node* p) { return (_node*)(uintptr_t(p) | 1);}
    _node* unmark(_node* p) { return (_node*)(uintptr_t(p) & (~1));}
    bool is_marked(_node* p) { return uintptr_t(p) & 1;}

    // mark all next ptrs of a node
    void mark_node_ptrs(_node* x) {
        for (ssize_t i = x->level - 1; i >= 0; i--) {
            _node* x_next;
            do {
                x_next = x->next[i];
                if (is_marked(x_next)) break;
            } while (CAS(&x->next[i], x_next, mark(x_next)) != x_next);
        }
    }

    void search(_node** left_list, _node** right_list, _Key key) {
retry:  
        _node* left = this->header;
        _node* left_next;

        for (ssize_t i = this->max_level - 1; i >= 0; i--) {
            left_next = left->next[i];
            if (is_marked(left_next)) goto retry;

            _node *right, *right_next;
            for (right = left_next; ; right = right_next) {
                while (true) {
                    right_next = right->next[i];
                    if (!is_marked(right_next)) break;
                    right = unmark(right_next);
                }
                if (!(*(right) < key)) break;
                left = right, left_next = right_next;
            }

            if (left_next != right && CAS(&left->next[i], left_next, right) != left_next) goto retry;
            
            left_list[i] = left;
            right_list[i] = right;
        }
    }
    
public:
    lock_free_skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) : 
    skip_list<_Key, _Val>(_max_level) {}

    virtual ~lock_free_skip_list() {}

    virtual _Val get(_Key key) {
        _node** left_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        search(left_list, right_list, key);

        if (!(*right_list[0] == key)) {
            GC_FREE(left_list);
            GC_FREE(right_list);
            return _Val();
        }

        _Val* v_ptr = right_list[0]->v_ptr;
        _Val val = v_ptr ? *v_ptr : _Val();

        GC_FREE(left_list);
        GC_FREE(right_list);
        return val;
    };

    virtual void insert(_Key key, _Val val) {
        _node** left_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));

        size_t new_level = this->rand_level();
        _node* new_node = (_node*)GC_MALLOC(sizeof(_node));
        _Val* v_ptr = (_Val*)GC_MALLOC(sizeof(_Val));
        *v_ptr = val;

        new_node->init(new_level);
        new_node->set_key_val(key, v_ptr);
retry:
        search(left_list, right_list, key);

        if (*(right_list[0]) == key) {
            _Val* old_v;
            do {
                if ((old_v = right_list[0]->v_ptr) == nullptr) {
                    mark_node_ptrs(right_list[0]);
                    // printf("goto retry 1, key=%d\n", key);
                    goto retry;
                }
                // printf("spin 1, key=%d\n", key);
            } while(CAS(&right_list[0]->v_ptr, old_v, v_ptr) != old_v);
            return;
        }

        for (size_t i = 0; i < new_level; i++) {
            new_node->next[i] = right_list[i];
        }

        if (CAS(&left_list[0]->next[0], right_list[0], new_node) != right_list[0]) {
            // printf("goto retry 2, key=%d\n", key);
            goto retry;
        }

        for (size_t i = 1; i < new_level; i++) {
            _node *left, *right, *new_next;
            while (true) {
                // printf("spin 3, key=%d, level=%d\n", key, i);
                left = left_list[i], right = right_list[i];

                new_next = new_node->next[i];
                if (new_next != right && CAS(&new_node->next[i], unmark(new_next), right) != unmark(new_next))
                    break;
                if (*right == key) {
                    right = unmark(right->next[i]);
                }
                if (CAS(&left_list[i]->next[i], right, new_node) == right) break;

                search(left_list, right_list, key);
            }
        }

        GC_FREE(left_list);
        GC_FREE(right_list);
    };
    
    virtual void erase(_Key key) {
        _node** left_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        _node** right_list = (_node**)GC_MALLOC(this->max_level * sizeof(_node*));
        search(left_list, right_list, key);
        if (!(*right_list[0] == key)) {
            GC_FREE(left_list);
            GC_FREE(right_list);
            return;
        }

        _Val* v;
        do {
            if ((v = right_list[0]->v_ptr) == nullptr) {
                GC_FREE(left_list);
                GC_FREE(right_list);
                return;
            }
            //printf("spin 2, key=%d\n", key);
        } while (CAS(&right_list[0]->v_ptr, v, nullptr) != v);

        mark_node_ptrs(right_list[0]);
        search(left_list, right_list, key);

        GC_FREE(left_list);
        GC_FREE(right_list);
        return;
    };
};

#endif