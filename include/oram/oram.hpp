#pragma once

#include <cassert>
#include <vector>
#include <cmath>

#include "block.hpp"
#include "bucket.hpp"

namespace oram_lib
{
    //internal representation of the path oram tree
    class oram
    {
    public:
        using bckt = bucket<std::string>;
        int n;
        int L;
        int N;
        std::vector<bckt> tree;
        oram(int n) : n(n), L(1 + static_cast<int>(std::log2(n))), N(1 << (L + 1)), tree(N)
        {
        }
        oram() : oram(1){};

        int get_node_idx (int leaf_idx, int depth)
        {
            return (1 << depth) + (leaf_idx/(1 << (L - depth))); 
        }

        //unused
        std::vector<std::string> read_bucket(int leaf_idx, int depth)
        {
            int node_idx = get_node_idx(leaf_idx, depth);
            return std::vector<std::string> (tree[node_idx].blocks, tree[node_idx].blocks + bckt::bucket_size);
        }

        std::vector<std::string> read_path(int leaf_idx)
        {
            std::vector<std::string> path_blocks;
            for(int d = 0; d <= L; d ++)
            {
                int idx = get_node_idx(leaf_idx, d);
                std::cerr << "fetching node " << idx << std::endl;
                for(int i = 0; i < bckt::bucket_size; i ++)
                    path_blocks.push_back(tree[idx].blocks[i]);
            }
            return path_blocks;
        }

        void write_to_bucket(int leaf_idx, int depth, bckt bc)
        {
            int node_idx = get_node_idx(leaf_idx, depth);
            std::cerr << "writing to node " << node_idx << std::endl;
            copy(bc.blocks, bc.blocks + bckt::bucket_size, tree[node_idx].blocks);
        }
    };
}