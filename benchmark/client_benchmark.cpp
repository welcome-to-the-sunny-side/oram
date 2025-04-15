#include <iostream>

#include "include/oram_lib.hpp"

using namespace oram_lib;
std::vector<unsigned char> encryptor::key;
client_network_communicator cnc;
void o_init()
{
    encryptor::initialize();
    cnc.connect_to_server();
    o_array::init_communicator(cnc);
}

int main()
{
    o_init();

    int n, q;
    std::cin >> n >> q;

    std::cout << "n = " << n << ", q = " << q << std::endl;
    std::cout.flush();

    //benchmark
    auto start1 = std::chrono::high_resolution_clock::now();
    o_array a(n);
    auto end1 = std::chrono::high_resolution_clock::now();
   
    std::cout << "initialization time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < q; i ++)
    {
        int idx = rng(n);
        int val = rng(std::numeric_limits<int>::max());
        a[idx] = val;
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    int time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    std::cout << "total update time: " << time_taken << "ms" << std::endl;
    std::cout << "average update time: " << ((long double)time_taken) / ((long double)q) << "ms" << std::endl;
    
    return 0;
}


