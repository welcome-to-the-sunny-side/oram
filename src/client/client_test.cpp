#include <iostream>
#include <vector>

#include "include/oram_lib.hpp"

using namespace oram_lib;
std::vector<unsigned char> Encryptor::key;
client_network_communicator cnc;
void server_init()
{
    Encryptor::initialize();
    cnc.connect_to_server();
}

int main()
{
    server_init();

    int n;
    std::cin >> n;

    o_array a(n, cnc);
    for(int i = 0; i < n; i ++)
        std::cin >> a[i];
    
    o_array prefix_sum_a(n, cnc);
    prefix_sum_a[0] = a[0];
    for(int i = 1; i < n; i ++)
        prefix_sum_a[i] = prefix_sum_a[i - 1] + a[i];
    
    for(int i = 0; i < n; i ++)
        std::cout << prefix_sum_a[i] << " ";
    std::cout << std::endl;

    cnc.end_session();
    return 0;
}