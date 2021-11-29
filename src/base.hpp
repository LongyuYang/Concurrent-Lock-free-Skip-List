#ifndef _SKIP_LIST_BASE_H
#define _SKIP_LIST_BASE_H

#include <stddef.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <vector>

#include <pthread.h>

#define GC_THREADS
#include "gc.h"

#define _SKIP_LIST_MAX_LEVEL 16

template<class _Key, class _Val, bool _init_lock = true> 
class skip_list_node {
private:
    typedef skip_list_node<_Key, _Val, _init_lock> _node;
    std::mutex* pointer_locks;
public:
    _Key key;
    _Val val;

    _Val *v_ptr; // used by lock-free implementation only

    _node** next;
    
    size_t level = 0;

    bool is_header = false, is_tail = false;

    skip_list_node() {}

    void init(size_t _level) {
        next = (_node**)GC_MALLOC(_level * sizeof(_node*));
        level = _level;

        if (_init_lock) {
            pointer_locks = (std::mutex*)GC_MALLOC((_level + 1)*sizeof(std::mutex));
        }
    }

    virtual ~skip_list_node() {
        if (_init_lock) {
            GC_FREE(pointer_locks);
        }
    }

    void mark_as_header() { is_header = true; }
    void mark_as_tail() { is_tail = true; }
    void set_level(size_t _level) { level = _level; }
    void set_key_val(_Key _key, _Val _val) { key = _key, val = _val; }
    void set_key_val(_Key _ket, _Val *_v_ptr) { key = _ket, v_ptr = _v_ptr; }

    bool operator<(const _node& another_node) {
        return (is_header || another_node.is_tail) ? true : 
        ((is_tail || another_node.is_header)? false : key < another_node.key);
    }

    bool operator>(const _node& another_node) {
        return !this->operator<(another_node);
    }

    bool operator<(_Key another_key) {
        return is_header ? true : (is_tail ? false : key < another_key);
    }

    bool operator>(_Key another_key) {
        return is_header ? false : (is_tail ? true : key > another_key);
    }

    bool operator==(_Key another_key) {
        return is_header ? false : (is_tail ? false : key == another_key);
    }

    // lock a next pointer on the given level 0 to level - 1
    // if i == level, it means lock the whole level
    void lock(int i) {
        if (_init_lock) {
            pointer_locks[i].lock();
        } else {
            std::cerr << "Warning: pointer lock uninitialized!" << std::endl;
        }
    }

    // unlock a next pointer on the given level 0 to level - 1
    // if i == level, it means unlock the whole level
    void unlock(int i) {
        if (_init_lock) {
            pointer_locks[i].unlock();
        } else {
            std::cerr << "Warning: pointer lock uninitialized!" << std::endl;
        }
    }
};

template<class _Key, class _Val, bool _init_lock = true>
class skip_list {
protected:
    typedef skip_list_node<_Key, _Val, _init_lock> _node;

    _node* header;
    _node* tail;

    size_t max_level;

    size_t rand_level() {
        size_t new_level = 1;
        while(rand() % 2 == 0 && new_level <= max_level) {
            new_level++;
        }
        return std::min(new_level, max_level);
    }
public:
    skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) {
        GC_INIT();

        max_level = _max_level;

        header = (_node*)GC_MALLOC_UNCOLLECTABLE(sizeof(_node));
        tail = (_node*)GC_MALLOC_UNCOLLECTABLE(sizeof(_node));

        header->init(max_level);
        tail->init(max_level);

        header->mark_as_header();
        tail->mark_as_tail();

        for (int i = 0; i < max_level; i++) {
            header->next[i] = tail;
        }
    }

    virtual ~skip_list() {
        GC_FREE(header);
        GC_FREE(tail);
    }

    virtual _Val get(_Key key) = 0;
    virtual void insert(_Key key, _Val val) = 0;
    virtual void erase(_Key key) = 0;

    void debug_print() {
        _node* x = header->next[0];
        while (x != tail) {
            for (size_t i = 0; i < x->level; i++) {
                std::cout << std::setw(10) << x->key;
            }
            std::cout << std::endl;
            x = x->next[0];
        }
    }

    bool is_empty() {
        for (size_t i = 0; i < max_level; i++) {
            if (header->next[i] != tail) {
                return false;
            }
        }

        return true;
    }
};

#endif