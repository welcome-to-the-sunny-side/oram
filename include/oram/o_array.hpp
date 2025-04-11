#pragma once

#include <vector>
#include <cmath>

//include network stuff over here later
#include "rng.hpp"
#include "../net/client_network_communicator.hpp"

using namespace std;

// array stored on the cloud obliviously
template<typename block_type>
class o_array
{
public:
    static int o_array_id_cntr = 0;
    int n;      //number of elements
    int L, N;   //tree parameters
    int id;
    vector<int> leaf_map;           //leaf_map[i] = the index of the leaf that the i'th element is mapped to
    vector<bool> in_stash;          //in_stash[i] = true => i'th element is in the stash
    client_network_communicator<block_type> &communicator;
    unordered_map<int, block_type> stash;

    //auxiliary encryption stuff, remove later
    vector<int> encrypt_val, encrypt_idx;
    
    o_array(int n, client_network_communicator<block_type>& given_communicator) :
    n(n), L(static_cast<int>(log2(n)) + 1), N (1 << L), id(o_array_id_cntr ++), leaf_map(N), in_stash(N, true), communicator(given_communicator), encrypt_val(N), encrypt_idx(N)
    {
        //send message to server, informing it about the creation of this array
        
    };

    //pass any lambda `use` you wish here, note that you may want to take the block in by reference
    void access(int i, auto use)
    {
        if(in_stash[i])
        {
            blk.decrypt();
            use(stash[i]);
            blk.encrypt();
            return;
        }
        
        int l = leaf_map[i];

        //request blocks on the relevant path from the server 
        vector<block_type> path_blocks = communicator.request_path(id, l);

        for(auto &blk : path_blocks)
        {
            blk.decrypt();

            if(blk.idx == -1)   //dummy block
                continue;

            if(blk.idx == i)
                use(blk); //modify or do whatever you want with this block
            //we re-encrypt the block to preserve privacy
            blk.encrypt();
            in_stash[idx] = true;
            stash[idx] = blk;
        }

        leaf_map[i] = rng(N);

        for(int d = L; d >= 0; d --)
        {
            vector<block_type> wrt;
            for(auto [idx, ])
        }
    }


};