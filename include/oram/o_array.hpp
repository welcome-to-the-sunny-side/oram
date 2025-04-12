#pragma once

#include <vector>
#include <iostream>
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
        // Forward declare stream operators as friends
        using bckt = bucket<block>;

        class element_proxy;
        friend std::istream& operator>>(std::istream& is, element_proxy& proxy);
        friend std::ostream& operator<<(std::ostream& os, const element_proxy& proxy);

        static constexpr int P = 1000000007;
        inline static int o_array_id_cntr = 0;

        int n;                      // number of elements
        int L, N;                   // tree parameters
        int id;                     // array id
        std::vector<int> leaf_map;  // leaf_map[i] = the index of the leaf that the i'th element is mapped to
        std::vector<bool> in_stash; // in_stash[i] = true => i'th element is in the stash
        client_network_communicator &communicator;
        std::unordered_map<int, block> stash; // local stash of blocks

        // initialize an o_array, initially filled with 0s
        o_array(int n, client_network_communicator &given_communicator) : n(n), L(static_cast<int>(log2(n)) + 1), N(1 << (L + 1)), id(o_array_id_cntr++), leaf_map(N, -1), in_stash(N, false), communicator(given_communicator)
        {
            communicator.create_array(id, n);

            // fill it with dummy blocks initially

            for (int leaf_idx = 0; leaf_idx < N / 2; leaf_idx++)
                for (int depth = 0; depth <= L; depth++)
                {
                    bucket<block> b;
                    for (int i = 0; i < bckt::bucket_size; i++)
                        b.blocks[i] = block(0, N + 1);
                    communicator.write_to_bucket(id, leaf_idx, depth, b);
                }
        }

        // touch the i'th element of the array (populate stash with it)
        void touch(int i)
        {
            if (leaf_map[i] == -1)
            {
                // uninitialized block
                leaf_map[i] = rng(N / 2);
                stash[i] = block(0, i);
                in_stash[i] = true;
            }
            else if (in_stash[i])
            {
                // shhh
            }

            // request blocks on the relevant path from the server
            // we assume that we receive decrypted blocks from the server
            std::vector<block> path_blocks = communicator.request_path(id, leaf_map[i]);

            for (auto &blk : path_blocks)
            {
                if (blk.idx == N + 1) // dummy block
                    continue;

                in_stash[blk.idx] = true;
                stash[blk.idx] = blk;
            }
        }

        // write back the contents of the stash that fit into the path from the root to leaf_idx
        void write_back(int leaf_idx)
        {
            for (int d = L; d >= 0; d--)
            {
                int node = get_node_idx(leaf_idx, d);

                std::vector<int> chosen;
                for (auto [idx, blk] : stash)
                    if (!blk.rsrvd) // prevent blocks we need in the current context from being removed from the stash
                    {
                        if (node == get_node_idx(leaf_map[idx], d))
                            chosen.push_back(idx);
                        if (chosen.size() == bckt::bucket_size)
                            break;
                    }

                bckt bkt;
                for (int i = 0; i < chosen.size(); i++)
                {
                    bkt.blocks[i] = stash[chosen[i]];
                    stash.erase(chosen[i]);
                    in_stash[chosen[i]] = false;
                }
                for (int i = int(chosen.size()); i < bckt::bucket_size; i++)
                {
                    // just use a dummy node with index N + 1
                    block b(0, N + 1);
                    bkt.blocks[i] = b;
                }

                communicator.write_to_bucket(id, leaf_idx, d, bkt);
            }
        }

        template <typename F>
        void access(int i, F use)
        {
            touch(i);
            use(stash[i]);
            int l = leaf_map[i];
            leaf_map[i] = rng(N / 2);
            write_back(l);
        }

        // proxy class to handle array element access
        class element_proxy
        {
        private:
            o_array &arr;
            int index;

        public:
            element_proxy(o_array &a, int i) : arr(a), index(i) {}

            // Assignment operator
            element_proxy &operator=(int val)
            {
                arr.touch(index);
                arr.stash[index].rsrvd = true;
                arr.stash[index].val = val;
                arr.stash[index].rsrvd = false;
                int l = arr.leaf_map[index];
                arr.leaf_map[index] = rng(arr.N / 2);
                arr.write_back(l);
                return *this;
            }

            // Conversion operator to int for reading values
            operator int() const
            {
                int result;
                arr.access(index, [&](block &b)
                           { result = b.val; });
                return result;
            }

            friend std::istream& operator>>(std::istream& is, element_proxy&& proxy)
            {
                int val;
                is >> val;
                proxy = val; // Use existing assignment operator
                return is;
            }

            friend std::istream& operator>>(std::istream& is, element_proxy& proxy)
            {
                int val;
                is >> val;
                proxy = val; // Use existing assignment operator
                return is;
            }

            friend std::ostream& operator<<(std::ostream& os, const element_proxy& proxy)
            {
                os << static_cast<int>(proxy); // Use existing conversion operator
                return os;
            }


            // Compound assignment operators
            element_proxy &operator+=(int val)
            {
                arr.touch(index);
                arr.stash[index].rsrvd = true;
                arr.stash[index].val += val;
                arr.stash[index].rsrvd = false;
                int l = arr.leaf_map[index];
                arr.leaf_map[index] = rng(arr.N / 2);
                arr.write_back(l);
                return *this;
            }

            element_proxy &operator-=(int val)
            {
                arr.touch(index);
                arr.stash[index].rsrvd = true;
                arr.stash[index].val -= val;
                arr.stash[index].rsrvd = false;
                int l = arr.leaf_map[index];
                arr.leaf_map[index] = rng(arr.N / 2);
                arr.write_back(l);
                return *this;
            }

            element_proxy &operator*=(int val)
            {
                arr.touch(index);
                arr.stash[index].rsrvd = true;
                arr.stash[index].val *= val;
                arr.stash[index].rsrvd = false;
                int l = arr.leaf_map[index];
                arr.leaf_map[index] = rng(arr.N / 2);
                arr.write_back(l);
                return *this;
            }

            element_proxy &operator/=(int val)
            {
                arr.touch(index);
                arr.stash[index].rsrvd = true;
                arr.stash[index].val /= val;
                arr.stash[index].rsrvd = false;
                int l = arr.leaf_map[index];
                arr.leaf_map[index] = rng(arr.N / 2);
                arr.write_back(l);
                return *this;
            }

        };

        // Array subscript operator that returns the proxy object
        element_proxy operator[](int i)
        {
            return element_proxy(*this, i);
        }

    private:
        int get_node_idx(int leaf_idx, int depth)
        {
            return (1 << depth) + (leaf_idx / (1 << (L - depth)));
        }
    };

}