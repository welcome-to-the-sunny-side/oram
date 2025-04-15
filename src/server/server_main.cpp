#include <iostream>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>

#include "include/oram_lib.hpp"

using namespace oram_lib;
using namespace boost::asio;
using ip::tcp;

int main()
{
    std::vector<oram> orams;
    std::unordered_map<int, int> oram_id_to_idx;

    boost::asio::io_context io_context;

    const std::string server_address = "127.0.0.1";
    const int server_port = 1234;

    tcp::acceptor acceptor(io_context, tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), server_port));
    std::cout << "Server is running on port " << server_port << std::endl;

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
                boost::system::error_code error;
                size_t bytes_transferred = boost::asio::read(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)), error);
                
                if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset)
                {
                    std::cout << "Client disconnected" << std::endl;
                    break;
                }
                else if (error)
                {
                    throw boost::system::system_error(error);
                }

                // std::cerr << "Received message code: " << int32_t(msg_code) << std::endl;

                if(msg_code == 0)
                {
                    // exit
                    return 0;
                }
                else if(msg_code == 5)
                {
                    // create array
                    int array_id, array_size;
                    boost::asio::read(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                    boost::asio::read(socket, boost::asio::buffer(&array_size, sizeof(array_size)));

                    std::string dummy_block_str(block::str_len, '\0');
                    boost::asio::read(socket, boost::asio::buffer(&dummy_block_str[0], block::str_len));

                    orams.push_back(oram(array_size));
                    oram_id_to_idx[array_id] = orams.size() - 1;

                    bucket<std::string> bkt;
                    for(int i = 0; i < bucket<std::string>::bucket_size; i ++)
                        bkt.blocks[i] = dummy_block_str;
                    
                    int idx = oram_id_to_idx[array_id];
                    for(int i = 0; i < orams[idx].N; i ++)
                        orams[idx].tree[i] = bkt;
                }
                else if(msg_code == 6)
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
                        // size_t str_len = str.length();
                        // boost::asio::write(socket, boost::asio::buffer(&str_len, sizeof(str_len)));
                        boost::asio::write(socket, boost::asio::buffer(str.data(), block::str_len));
                    }
                }
                else if(msg_code == 7)
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
                        // size_t str_len;
                        // boost::asio::read(socket, boost::asio::buffer(&str_len, sizeof(str_len)));
                        std::string str(block::str_len, '\0');
                        boost::asio::read(socket, boost::asio::buffer(&str[0], block::str_len));
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
                }
                else
                {
                    throw std::runtime_error("invalid message code: " + std::to_string(msg_code));
                }
            }
            catch (std::exception &e)
            {
                std::cerr << "Message handling error: " << e.what() << std::endl;
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