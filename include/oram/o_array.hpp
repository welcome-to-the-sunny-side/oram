#pragma once

#include <vector>
#include <cmath>
#include <unordered_map>

#include "block.hpp"
#include "bucket.hpp"
#include "rng.hpp"
#include "../net/client_network_communicator.hpp"

namespace oram_lib
{
    // array stored on the cloud obliviously
    class o_array
    {
    public:
        using bckt = bucket<block>;

        static constexpr int P = 1000000007;
        inline static int o_array_id_cntr = 0;

        int n;      //number of elements
        int L, N;   //tree parameters
        int id;     //array id
        std::vector<int> leaf_map;           //leaf_map[i] = the index of the leaf that the i'th element is mapped to
        std::vector<bool> in_stash;          //in_stash[i] = true => i'th element is in the stash
        client_network_communicator &communicator;
        std::unordered_map<int, block> stash;   //local stash of blocks

        //initialize an o_array, initially filled with 0s
        o_array(int n, client_network_communicator& given_communicator) :
        n(n), L(static_cast<int>(log2(n)) + 1), N (1 << (L + 1)), id(o_array_id_cntr ++), leaf_map(N, -1), in_stash(N, false), communicator(given_communicator)
        {
            communicator.create_array(id, n);

            //fill it with dummy blocks initially 

            for(int leaf_idx = 0; leaf_idx < N/2; leaf_idx ++)
                for(int depth = 0; depth <= L; depth ++)
                {
                    bucket<block> b;
                    for(int i = 0; i < bckt::bucket_size; i ++)
                        b.blocks[i] = block(0, N + 1);
                    communicator.write_to_bucket(id, leaf_idx, depth, b);
                }
        }

        //pass any lambda `use` you wish here, note that you may want to take the block in by reference
        template<typename F>
        void access(int i, F use)
        {
            int l = leaf_map[i];

            if(l == -1)
            {
                //uninitialized block
                l = leaf_map[i] = rng(N/2);
                stash[i] = block(0, i);
                use(stash[i]);
                in_stash[i] = true;
            }
            else if(in_stash[i])
            {
                use(stash[i]);
            }

            //request blocks on the relevant path from the server 
            //we assume that we receive decrypted blocks from the server
            std::vector<block> path_blocks = communicator.request_path(id, l);

            for(auto &blk : path_blocks)
            {
                if(blk.idx == N + 1)    //dummy block
                    continue;

                if(blk.idx == i)
                    use(blk);           //bla bla bla

                in_stash[blk.idx] = true;
                stash[blk.idx] = blk;
            }

            leaf_map[i] = rng(N/2);

            //trivial write-back algorithm for now, optimize it later
            for(int d = L; d >= 0; d --)
            {
                int node = get_node_idx(l, d);

                std::vector<int> chosen;
                for(auto [idx, blk] : stash)
                {
                    if(node == get_node_idx(leaf_map[idx], d))
                        chosen.push_back(idx);
                    if(chosen.size() == bckt::bucket_size)
                        break;
                }

                bckt bkt;
                for(int i = 0; i < chosen.size(); i ++)
                {
                    bkt.blocks[i] = stash[chosen[i]];
                    stash.erase(chosen[i]);
                    in_stash[chosen[i]] = false;
                }
                for(int i = int(chosen.size()); i < bckt::bucket_size; i ++)
                {
                    //just use a dummy node with index N + 1
                    block b (0, N + 1);
                    bkt.blocks[i] = b;
                }

                communicator.write_to_bucket(id, l, d, bkt);
            }
        }

    // private:
        int get_node_idx (int leaf_idx, int depth)
        {
            return (1 << depth) + (leaf_idx/(1 << (L - depth))); 
        }
    };
}