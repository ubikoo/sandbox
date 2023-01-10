/*
 * hashmap.cpp
 *
 * Copyright (c) 2020 Carlos Braga
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the MIT License.
 *
 * See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
 */

#include <algorithm>
#include "hashmap.hpp"

/**
 * @brief Clear the hash table key-value items and set their state to empty.
 */
void Hashmap::clear(void)
{
    m_numitems = 0;
    std::fill(m_data.begin(), m_data.end(), KeyValue{kEmpty, kEmpty});
}

/**
 * @brief Insert a key-value item into the table. Start iterating at the slot
 * given by the key hash value. If the slot key is empty, insert the value.
 */
void Hashmap::insert(const uint32_t key, const uint32_t value)
{
    uint32_t slot = key % m_capacity;
    while (true) {
        uint32_t prev = compare_and_swap(&m_data[slot].key, kEmpty, key);
        if (prev == kEmpty) {
            m_numitems++;
            m_data[slot].value = value;
            return;
        }
        slot = (slot + 1) % m_capacity; // slot & (m_capacity - 1)
    }
}

/**
 * @brief Return the first slot containing the specified key. If no key is found
 * return the empty state mask. The mask is then used to signal that no further
 * slots in the map contain the specified key.
 */
uint32_t Hashmap::begin(const uint32_t key) const
{
    uint32_t slot = key % m_capacity;   // slot & (m_capacity - 1)
    while (true) {
        if (m_data[slot].key == key) {
            return slot;
        }

        if (m_data[slot].key == kEmpty) {
            return kEmpty;
        }

        slot = (slot + 1) % m_capacity; // slot & (m_capacity - 1)
    }
}

/**
 * @brief Return the next slot containing the specified key.
 */
uint32_t Hashmap::next(const uint32_t key, uint32_t slot) const
{
    while (true) {
        slot = (slot + 1) % m_capacity; // slot & (m_capacity - 1)

        if (m_data[slot].key == key) {
            return slot;
        }

        if (m_data[slot].key == kEmpty) {
            return kEmpty;
        }
    }
}

/**
 * @brief Compare key with old value and swap with new value.
 */
uint32_t Hashmap::compare_and_swap(uint32_t *key,
    uint32_t oldval, uint32_t newval)
{
    uint32_t prev = *key;
    if (*key == oldval) {
        *key = newval;
    }
    return prev;
}

/**
 * @brief Create a hashmap with the next power of two of the given capacity.
 */
Hashmap Hashmap::Create(const uint32_t min_capacity)
{
    /* Compute the next power of two capacity */
    uint32_t capacity = Hashmap::kMinSize;
    while (capacity <= min_capacity) {
        capacity = capacity << 1;
    }

    /* Setup an empty hash table with no items. */
    Hashmap hashmap;
    hashmap.m_capacity = capacity;
    hashmap.m_numitems = 0;
    hashmap.m_data.resize(capacity, KeyValue{Hashmap::kEmpty, Hashmap::kEmpty});
    return hashmap;
}