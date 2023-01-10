/*
 * hashmap.hpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <vector>

/**
 * @brief Hashmap maintains an array of key-value items using open addressing
 * with linear probing to handle collision resolution of keys with the same
 * hash value.
 *
 * During insertion, the key is hashed and the resulting hash value indicates
 * where the corresponding value is to be stored.
 * Collision resolution is handled by iterating through the hash table array,
 * starting at the key's initial slot location.
 * At each slot in the table, perform an atomic compare-and-swap of the slot
 * key value with empty. If true (ie if the slot key value is empty), update
 * the slot with the new key value and return the slot's original key (empty
 * or not).
 */
struct Hashmap {
    /** Hashmap key-value item type. */
    struct KeyValue {
        uint32_t key;
        uint32_t value;
    };

    /** Capacity range as a power of two */
    static const uint32_t kMinBits = 3;   /* 8 items */
    static const uint32_t kMaxBits = 31;  /* 2147483648 items */
    static const uint32_t kMinSize = 1 << kMinBits;
    static const uint32_t kMaxSize = 1 << kMaxBits;

    /** State flag indicating an empty slot. */
    static const uint32_t kEmpty = 0xffffffff;

    /** Member variables. */
    uint32_t m_capacity;            /* max number of items in the table */
    uint32_t m_numitems;            /* number of items in the table */
    std::vector<KeyValue> m_data;   /* hashmap table */

    /** Return the max numnber of key-value items in the hash table. */
    const uint32_t &capacity(void) const { return m_capacity; }

    /** Return the number of key-value items in the hash table. */
    const uint32_t &size(void) const { return m_numitems; }

    /** Return a reference to the hashmap underlying data array. */
    const std::vector<KeyValue> &data(void) const { return m_data; }

    /** Clear the hash table key-value items and set their state to empty. */
    void clear(void);

    /** Insert a key-value item into the table. */
    void insert(const uint32_t key, const uint32_t value);

    /** Return the first slot containing the specified key. */
    uint32_t begin(const uint32_t key) const;

    /** Return the past-the-end element indicating an empty slot. */
    uint32_t end(void) const { return kEmpty; }

    /** Return the next slot containing the specified key. */
    uint32_t next(const uint32_t key, uint32_t slot) const;

    /** Return the value of the current slot. */
    uint32_t get(const uint32_t slot) const { return m_data[slot].value; }

    /** Compare key with old value and swap with new value. */
    uint32_t compare_and_swap(uint32_t *key, uint32_t oldval, uint32_t newval);

    /** Hashmap factory function. */
    static Hashmap Create(const uint32_t min_capacity);
};

#endif /* HASHMAP_H_ */
