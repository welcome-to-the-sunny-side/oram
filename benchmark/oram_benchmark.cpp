#include <iostream>

#include "include/oram_lib.hpp"

using namespace oram_lib;

int main()
{
    int n, q;
    std::cin >> n >> q;

    std::cout << "n = " << n << ", q = " << q << std::endl;

    //initialize and populate the oram
    //benchmark the initialization time
    auto start1 = std::chrono::high_resolution_clock::now();
    oram o(n);
    auto end1 = std::chrono::high_resolution_clock::now();
    std::cout << "initialization time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms" << std::endl;


    //perform write queries
    auto start2 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < q; i ++)
    {
        int leaf_idx = rng(o.N/2);
        int depth = rng(o.L + 1);

        bucket<std::string> bc;
        for(int i = 0; i < bucket<std::string>::bucket_size; i ++)
            bc.blocks[i] = std::string(32, 'a');
        o.write_to_bucket(leaf_idx, depth, bc);
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    int time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    std::cout << "total write query time: " << time_taken << "ms" << std::endl;
    std::cout << "average write query time: " << ((long double)time_taken) / ((long double)q) << "ms" << std::endl;

    //perform mixed queries
    auto start3 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < q; i ++)
    {
        int op = rng(2);
        if(op == 0)
        {
            int leaf_idx = rng(o.N/2);
            std::vector<std::string> path = o.read_path(leaf_idx);
        }
        else
        {
            int leaf_idx = rng(o.N/2);
            int depth = rng(o.L + 1);

            bucket<std::string> bc;
            for(int i = 0; i < bucket<std::string>::bucket_size; i ++)
                bc.blocks[i] = std::string(32, 'a');
            o.write_to_bucket(leaf_idx, depth, bc);
        }
    }
    auto end3 = std::chrono::high_resolution_clock::now();

    time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3).count();

    std::cout << "total random query time: " << time_taken << "ms" << std::endl;
    std::cout << "average random query time: " << ((long double)time_taken) / ((long double)q) << "ms" << std::endl;

    //perform only read queries
    auto start4 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < q; i ++)
    {
        int leaf_idx = rng(o.N/2);
        std::vector<std::string> path = o.read_path(leaf_idx);
    }   
    auto end4 = std::chrono::high_resolution_clock::now();

    time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end4 - start4).count();
    std::cout << "total read query time: " << time_taken << "ms" << std::endl;
    std::cout << "average read query time: " << ((long double)time_taken) / ((long double)q) << "ms" << std::endl;

    return 0;
}