#pragma once

#include <vector>

namespace oram_lib
{
    template<typename block_type>
    class bucket
    {
    public:
        static constexpr int bucket_size = 4;
        block_type blocks[bucket_size];
    };
}