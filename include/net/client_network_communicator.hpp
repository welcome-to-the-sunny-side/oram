#pragma once

#include "../oram/bucket.hpp"

template<typename block_type>
class client_network_communicator
{
    using bckt = bucket<block_type>;

    void create_array(int array_id, int array_size)
    {
        //send message requesting the server to create another instance of o-ram
    }

    vector<block_type> request_path(int array_id, int leaf_idx)
    {
        vector<block_type> b;
        //send some message and get some response which you use to populate b
        return b;
    }

    void write_to_bucket (int array_id, int leaf_idx, int level, bckt bc)
    {
        //send some messages to write this basically
    }
};