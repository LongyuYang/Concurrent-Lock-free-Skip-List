#ifndef _SKIP_LIST_BASE_H
#define _SKIP_LIST_BASE_H

#include <stddef.h>
#include <iostream>

#define _SKIP_LIST_MAX_LEVEL 32

template<class _Key, class _Val> 
class skip_list_node {
    typedef skip_list_node<_Key, _Val> _node;
public:
    _Key key;
    _Val val;
    _node* next;
    

    size_t level = 0;

    bool is_header = false, is_tail = false;

    skip_list_node() {}

    void mark_as_header() {
        is_header = true;
    }

    void mark_as_tail() {
        is_tail = true;
    }

    bool operator<(_node another_node) {
        return is_header ? true : (is_tail ? false : key < another_node.key);
    }
};

template<class _Key, class _Val>
class skip_list {
protected:
    typedef skip_list_node<_Key, _Val> _node;

    _node* header;
    _node* tail;

    size_t max_level;
public:
    skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) {
        max_level = _max_level;

        header = new _node[max_level];
        tail = new _node[max_level];

        for (int i = 0; i < max_level; i++) {
            header[i].mark_as_header();
            tail[i].mark_as_tail();
        }
    }

    virtual ~skip_list() {
        delete[] header;
        delete[] tail;
    };

    virtual _Val* get(_Key key) = 0;
    virtual void insert(_Key key, _Val val) = 0;
    virtual void erase(_Key key) = 0;
};

#endif