#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>

#include "include/oram_lib.hpp"

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
    for(int i = 0; i < n; i ++)
        std::cin >> oa[i];
    
    for(int i = 1; i < n; i ++)
        oa[0] += oa[i] * oa[i];
    oa[0] += oa[0];
    std::cout << oa[0] << std::endl;

    cnc.end_session();

    return 0;
}