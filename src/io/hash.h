#pragma once

#include <cstdint>
#include <cstddef>

#include "utils/logger.h"
#include "utils/general.h"

namespace io
{
    __uint128_t hash(const uint8_t *data, size_t size);
}