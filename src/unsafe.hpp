#ifndef _UNSAFE_SKIP_LIST_H
#define _UNSAFE_SKIP_LIST_H

#include "base.hpp"

template<class _Key, class _Val>
class unsafe_skip_list : public skip_list<_Key, _Val> {
private:
public:
    unsafe_skip_list(size_t _max_level = _SKIP_LIST_MAX_LEVEL) : 
    skip_list<_Key, _Val>(_max_level) {}

    ~unsafe_skip_list() {}

    virtual _Val* get(_Key key) {};

    virtual void insert(_Key key, _Val val) {};
    
    virtual void erase(_Key key) {};
};

#endif