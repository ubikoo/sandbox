/*
 * unionfind.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef UNION_FIND_H_
#define UNION_FIND_H_

#include "base.hpp"

/**
 * @brief UnionFind implements a disjoint set data structure. It supports union
 * and find operations on the sets, together with a count operation that returns
 * the total number of components.
 */
struct UnionFind {
    /* Member data */
    std::vector<size_t> m_id;        /* parent identifier of each set */
    std::vector<size_t> m_sz;        /* component size of each set */
    size_t m_count;                  /* number of disjoint sets */
    size_t m_capacity;               /* total number of vertex keys */

    /* Member functions */
    void clear(void);
    size_t size(size_t v) const;
    size_t find(size_t v) const;
    bool connected(size_t v, size_t w) const;
    void join(size_t v, size_t w);
    std::map<size_t, std::vector<size_t>> sets(void);

    /* Constructor/destructor */
    explicit UnionFind(size_t capacity);
    ~UnionFind() = default;
};

#endif /* UNION_FIND_H_ */
