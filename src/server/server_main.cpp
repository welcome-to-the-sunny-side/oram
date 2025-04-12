#include <iostream>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>

#include "include/oram/rng.hpp"
#include "include/oram/oram.hpp"
#include "include/oram/bucket.hpp"

using namespace oram_lib;
using namespace boost::asio;
using ip::tcp;

int main()
{
    std::vector<oram> orams;
    std::unordered_map<int, int> oram_id_to_idx;

    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1234));

    std::cout << "Server is running on port 1234" << std::endl;

    try
    {
        // Create one persistent socket
        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Client connected" << std::endl;

        while (1)
        {
            try
            {
                uint8_t msg_code;
                boost::asio::read(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)));
                
                // std::cerr << "Received message code: " << int32_t(msg_code) << std::endl;

                switch (msg_code)
                {
                case 0:
                {
                    // exit
                    return 0;
                }
                case 5:
                {
                    // create array
                    int array_id, array_size;
                    boost::asio::read(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                    boost::asio::read(socket, boost::asio::buffer(&array_size, sizeof(array_size)));

                    orams.push_back(oram(array_size));
                    oram_id_to_idx[array_id] = orams.size() - 1;

                    break;
                }
                case 6:
                {
                    // handle request for path
                    int array_id, leaf_idx;
                    boost::asio::read(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                    boost::asio::read(socket, boost::asio::buffer(&leaf_idx, sizeof(leaf_idx)));

                    int idx = oram_id_to_idx[array_id];
                    std::vector<std::string> path = orams[idx].read_path(leaf_idx);

                    // send the size of the vector of strings
                    size_t size = path.size();
                    boost::asio::write(socket, boost::asio::buffer(&size, sizeof(size)));

                    // send the strings
                    for (const auto &str : path)
                    {
                        size_t str_len = str.length();
                        boost::asio::write(socket, boost::asio::buffer(&str_len, sizeof(str_len)));
                        boost::asio::write(socket, boost::asio::buffer(str.data(), str_len));
                    }
                    break;
                }
                case 7:
                {
                    // handle write to bucket
                    int array_id, leaf_idx, level;
                    boost::asio::read(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                    boost::asio::read(socket, boost::asio::buffer(&leaf_idx, sizeof(leaf_idx)));
                    boost::asio::read(socket, boost::asio::buffer(&level, sizeof(level)));

                    // read vector size
                    size_t size;
                    boost::asio::read(socket, boost::asio::buffer(&size, sizeof(size)));

                    // read each string individually
                    std::vector<std::string> bucket_content;
                    for (size_t i = 0; i < size; i++)
                    {
                        size_t str_len;
                        boost::asio::read(socket, boost::asio::buffer(&str_len, sizeof(str_len)));
                        std::string str(str_len, '\0');
                        boost::asio::read(socket, boost::asio::buffer(&str[0], str_len));
                        bucket_content.push_back(str);
                    }

                    if(bucket_content.size() != bucket<std::string>::bucket_size)
                    {
                        throw std::runtime_error("inappropriate bucket content size");
                        break;
                    }

                    bucket<std::string> bkt;
                    for(int i = 0; i < bucket<std::string>::bucket_size; i ++)
                        bkt.blocks[i] = bucket_content[i];

                    // write to bucket
                    int idx = oram_id_to_idx[array_id];
                    orams[idx].write_to_bucket(leaf_idx, level, bkt);
                    break;
                }
                default:
                {
                    throw std::runtime_error("invalid message code: " + std::to_string(msg_code));
                    break;
                }
                }
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("message handling error: " + std::string(e.what()));
                break;
            }
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("connection error: " + std::string(e.what()));
    }

    return 0;
}