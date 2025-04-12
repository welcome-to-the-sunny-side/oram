#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>

#include "include/oram/o_array.hpp"
#include "include/net/client_network_communicator.hpp"
#include "include/oram/bucket.hpp"
#include "include/oram/block.hpp"
#include "include/oram/encrypt.hpp"

using namespace oram_lib;

std::vector<unsigned char> Encryptor::key;

int main()
{
    Encryptor::initialize();

    client_network_communicator cnc;
    cnc.connect_to_server();

    std::cout << "Connected to server" << std::endl;

    int n;
    std::cin >> n;

    o_array oa(n, cnc);

    while(1)
    {
        int t;
        std::cin >> t;
        if(t == 1)
        {
            int i, x;
            std::cin >> i >> x;
            oa.access(i, [&](block &b) {b.val = x;});
        }
        else if(t == 2)
        {
            int i;
            std::cin >> i;
            int x = -1;
            oa.access(i, [&](block &b) {x = b.val;});
            std::cout << x << std::endl;
        }
        else if(t == 3)
        {
            std::cout << "displaying all blocks" << std::endl;
            for(int node_idx = 0; node_idx < oa.N/2; node_idx ++)
            {
                std::vector<block> b = cnc.request_path(oa.id, node_idx);
                for(int i = 0; i < b.size(); i ++)
                    std::cout << "val: " << b[i].val << " idx: " << b[i].idx << std::endl;
                std::cout << std::endl;
            }
        }
    }

    cnc.end_session();

    return 0;
}