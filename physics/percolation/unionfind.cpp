/*
 * unionfind.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include "unionfind.hpp"
using namespace atto;

/**
 * UnionFind::UnionFind
 * @brief Create a union find object with a specified number of sets.
 */
UnionFind::UnionFind(size_t capacity)
    : m_id(capacity)
    , m_sz(capacity)
    , m_count(capacity)
    , m_capacity(capacity)
{
    clear();
}

/**
 * UnionFind::clear
 * @brief Clear the components and reset the disjoint sets to their
 * original state.
 */
void UnionFind::clear(void)
{
    m_count = m_capacity;   /* reset number of disjoint sets. */
    for (size_t v = 0; v < m_capacity; ++v) {
        m_id[v] = v;        /* parent id of the set. */
        m_sz[v] = 1;        /* with a single element. */
    }
}

/**
 * UnionFind::size
 * @brief Return the size of the component v-key belongs to.
 */
size_t UnionFind::size(size_t v) const
{
    return m_sz[find(v)];
}

/**
 * UnionFind::find
 * @brief Find the component to which v-key belongs to.
 */
size_t UnionFind::find(size_t v) const
{
    while (v != m_id[v]) {
        v = m_id[v];
    }
    return v;
}

/**
 * UnionFind::connected
 * @brief Are sets v and w connected by the same root id?
 */
bool UnionFind::connected(size_t v, size_t w) const
{
    return (find(v) == find(w));
}

/**
 * UnionFind::join
 * @brief Join component of v-set and the component of w-set. If they are
 * already connected, do nothing. Otherwise, merge components of v and w.
 */
void UnionFind::join(size_t v, size_t w)
{
    /* Get the components of v and w. */
    size_t root_v = find(v);
    size_t root_w = find(w);

    /* Sets v and w belong to the same component - nothing to do. */
    if (root_v == root_w) {
        return;
    }

    /*
     * Join the smaller set into the larger set to minimize tree depth after
     * merge operation. If v-set is smaller than w-set, merge v-set and
     * increment w-size. Otherwise, merge w-set and increment v-size.
     */
    if (m_sz[root_v] < m_sz[root_w]) {
        m_id[root_v] = root_w;
        m_sz[root_w] += m_sz[root_v];
    } else {
        m_id[root_w] = root_v;
        m_sz[root_v] += m_sz[root_w];
    }

    /* Update the number of components. */
    m_count--;
}

/**
 * UnionFind::sets
 * @brief Return a key-value map of the components in the ensemble.
 * Keys represent the parent identifier of each component.
 * Values are vectors containing the sets in each component.
 */
std::map<size_t, std::vector<size_t>> UnionFind::sets(void)
{
    std::map<size_t, std::vector<size_t>> components;
    for (size_t v = 0; v < m_capacity; ++v) {
        components[find(v)].push_back(v);
    }
    return components;
}
