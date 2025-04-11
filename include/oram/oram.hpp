#pragma once

#include <cassert>
#include <vector>

#include "bucket.hpp"

using namespace std;

//internal representation of the path oram tree
template<typename block_type>
class oram
{
public:
    using bckt = bucket<block_type>
    int n;
    int L;
    int N;
    std::vector<bckt> tree;
    oram(int n)
    {
        assert(n != 0);
        this->n = n;
        int L = 1 + static_cast<int>(std::log2(n));
        N = (1 << L);
        tree.resize(N);
    }
    oram() : oram(1){};

    int get_node_idx (int leaf_idx, int depth)
    {
        return (1 << depth) + (leaf_idx/(1 << (L - 1 - depth))); 
    }

    //unused
    vector<block_type> read_bucket(int leaf_idx, int depth)
    {
        int node_idx = (leaf_idx, depth);
        return vector<block_type> (tree[node_idx].blocks, tree[node_idx].blocks + bucket::bucket_size);
    }

    vector<block_type> read_path(int leaf_idx)
    {
        vector<block_type> path_buckets;
        for(int d = 0; d < L; d ++)
            for(auto b : read_bucket(leaf_idx, d))
                path_buckets.push_back(b);
        return path_buckets;
    }

    void write_to_bucket(int leaf_idx, int depth, vector<block_type> blocks)
    {
        assert(blocks.size() == bucket::bucket_size);
        copy(blocks.begin(), blocks.end(), tree[get_node_idx(leaf_idx, depth)]);
    }
};