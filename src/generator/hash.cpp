#include "hash.h"

__uint128_t generator::hash(const uint8_t *data, size_t size) // very simple and basic for minor changes due to compression of video etc.
{
    const __uint128_t prime = (__uint128_t)0x811C9DC5ULL;
    __uint128_t hash = prime;

    for (size_t i = 0; i < size; ++i)
    {
        hash ^= ((unsigned __int128)data[i]);
        hash *= prime;
    }

    logger.debug("Hash: " + bytes_to_hex_string((uint8_t *)&hash, 16)); // Adjusted for standard string conversion

    return hash;
}