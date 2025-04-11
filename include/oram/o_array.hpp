#pragma once

#include <vector>
#include <cmath>

#include "bucket.hpp"
#include "rng.hpp"
#include "encrypt.hpp"
#include "../net/client_network_communicator.hpp"

using namespace std;

// array stored on the cloud obliviously
template<typename block_type>
class o_array
{
public:
    using bckt = bucket<block_type>;

    static int o_array_id_cntr = 0;

    int n;      //number of elements
    int L, N;   //tree parameters
    int id;
    vector<int> leaf_map;           //leaf_map[i] = the index of the leaf that the i'th element is mapped to
    vector<bool> in_stash;          //in_stash[i] = true => i'th element is in the stash
    client_network_communicator<block_type> &communicator;
    unordered_map<int, block_type> stash;

    //initialize an o_array, initially filled with 0s
    o_array(int n, client_network_communicator<block_type>& given_communicator) :
    n(n), L(static_cast<int>(log2(n)) + 1), N (1 << L), id(o_array_id_cntr ++), leaf_map(N, -1), in_stash(N, true), communicator(given_communicator)
    {
        communicator.create_array(id, n);
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

        if(l == -1)
        {
            //uninitialized block
            leaf_map[i] = rng(N/2);
            stash[i] = block_type(0, i);
            use(stash[i]);
            stash[i].encrypt();
        }
        else
        {
            //request blocks on the relevant path from the server 
            vector<block_type> path_blocks = communicator.request_path(id, l);

            for(auto &blk : path_blocks)
            {
                blk.decrypt();

                if(blk.idx == N + 1)   //dummy block
                    continue;

                if(blk.idx == i)
                    use(blk); //modify or do whatever you want with this block

                //we re-encrypt the block to preserve privacy
                blk.encrypt();

                in_stash[idx] = true;
                stash[idx] = blk;
            }
        }

        leaf_map[i] = rng(N/2);

        for(int d = L; d >= 0; d --)
        {
            int node = get_node_idx(l, d);

            vector<int> chosen;
            for(auto [idx, blk] : stash)
            {
                if(node == get_node_idx(leaf_map[idx], d))
                    chosen.push_back(idx);
                if(chosen.size() >= bckt::bucket_size)
                    break;
            }

            for(int i = 0; i < chosen.size(); i ++)
            {
                bkt[i] = stash[chosen[i]];
                stash.erase(chosen[i]);
            }
            for(int i = int(chosen.size()) + 1; i < bckt::bucket_size; i ++)
            {
                //just use a dummy node with index N + 1
                block_type b (rng(P), N + 1);
                b.encrypt();
                bkt[i] = b;
            }

            communicator.write_to_bucket(id, l, d, bkt);
        }
    }

private:
    int get_node_idx (int leaf_idx, int depth)
    {
        return (1 << depth) + (leaf_idx/(1 << (L - 1 - depth))); 
    }
};