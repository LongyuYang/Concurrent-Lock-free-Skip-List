#ifndef _SKIP_LIST_BASE_H
#define _SKIP_LIST_BASE_H

#include <stddef.h>
#include <iostream>
#include <iomanip>
#include <memory>

#define _SKIP_LIST_MAX_LEVEL 8

template<class _Key, class _Val> 
class skip_list_node {
    typedef skip_list_node<_Key, _Val> _node;
public:
    _Key key;
    _Val val;

    std::shared_ptr<_node>* next;
    
    size_t level = 0;

    bool is_header = false, is_tail = false;

    skip_list_node(size_t _level) {
        next = new std::shared_ptr<_node>[_level];
        level = _level;
    }

    ~skip_list_node() {
        delete[] next;
    }

    void mark_as_header() { is_header = true; }
    void mark_as_tail() { is_tail = true; }
    void set_level(size_t _level) { level = _level; }
    void set_key_val(_Key _key, _Val _val) { key = _key, val = _val; }

    bool operator<(_node another_node) {
        return is_header ? true : (is_tail ? false : key < another_node.key);
    }

    bool operator<(_Key another_key) {
        return is_header ? true : (is_tail ? false : key < another_key);
    }

    bool operator==(_Key another_key) {
        return is_header ? false : (is_tail ? false : key == another_key);
    }
};

template<class _Key, class _Val>
class skip_list {
protected:
    typedef skip_list_node<_Key, _Val> _node;

    std::shared_ptr<_node> header;
    std::shared_ptr<_node> tail;

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
        max_level = _max_level;

        header = std::make_shared<_node>(max_level);
        tail = std::make_shared<_node>(max_level);

        header->mark_as_header();
        tail->mark_as_tail();

        for (int i = 0; i < max_level; i++) {
            header->next[i] = tail;
        }

        srand(time(NULL));
    }

    virtual ~skip_list() {
        header.reset();
        tail.reset();
    };

    virtual _Val get(_Key key) = 0;
    virtual void insert(_Key key, _Val val) = 0;
    virtual void erase(_Key key) = 0;

    void debug_print() {
        _node* x = header;
        while (x != tail) {
            for (size_t i = 0; i < x->level; i++) {
                std::cout << std::setw(6) << x->key;
            }
            std::cout << std::endl;
            x = x->next[0];
        }
    }
};

#endif