#include "hash_functions.h"

size_t polynomial_rolling_hash::operator() (std::string const & str) const {
    size_t b = 19, m = 3298534883309;
    size_t hash = 0;
    size_t p = 1;

    for(int i = 0; i < str.length(); i++) {
        hash += str[i] * p;
        p = (p * b) % m;
    }

    return hash;
}

size_t fnv1a_hash::operator() (std::string const & str) const {
    size_t prime = 0x00000100000001B3;
    size_t basis = 0xCBF29CE484222325;

    size_t hash = basis;
    for(int i = 0; i < str.length(); i++) {
        hash = hash ^ str[i];
        hash *= prime;
    }
    return hash;
}
