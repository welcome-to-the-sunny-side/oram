#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <boost/asio.hpp>

#include "../oram/block.hpp"
#include "../oram/bucket.hpp"

namespace oram_lib
{
    class client_network_communicator
    {
    private:
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::socket socket;
        static constexpr int server_port = 1234;
        inline static const std::string server_address = "127.0.0.1";

    public:
        using bckt = bucket<block>;

        client_network_communicator() : socket(io_context)
        {
        }

        void connect_to_server(std::string p_server_address = server_address, int p_server_port = server_port)
        {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(server_address), server_port);
            socket.connect(endpoint);
        }

        void end_session()
        {
            uint8_t msg_code = 0;
            boost::asio::write(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)));
        }

        void create_array(int array_id, int array_size, block dummy_block) // message code 5
        {
            try
            {
                uint8_t msg_code = 5;
                boost::asio::write(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)));
                boost::asio::write(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                boost::asio::write(socket, boost::asio::buffer(&array_size, sizeof(array_size)));
                
                //send the encrypted dummy block (security risk since we're sending only one block to be filled everywhere but i'll deal with it later)
                std::string str = dummy_block.encrypt();
                boost::asio::write(socket, boost::asio::buffer(str.data(), block::str_len));
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("Error sending create_array request: " + std::string(e.what()));
            }
        }

        std::vector<block> request_path(int array_id, int leaf_idx) // message code 6
        {
            std::vector<block> res;
            try
            {
                uint8_t msg_code = 6;
                boost::asio::write(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)));
                boost::asio::write(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                boost::asio::write(socket, boost::asio::buffer(&leaf_idx, sizeof(leaf_idx)));

                std::vector<std::string> encrypted_strings;

                // receive the number of strings
                size_t size;
                boost::asio::read(socket, boost::asio::buffer(&size, sizeof(size)));

                // receive every string
                for (size_t i = 0; i < size; i++)
                {
                    // size_t str_len;
                    // boost::asio::read(socket, boost::asio::buffer(&str_len, sizeof(str_len)));
                    std::string str(block::str_len, '\0');
                    boost::asio::read(socket, boost::asio::buffer(&str[0], block::str_len));
                    encrypted_strings.push_back(str);
                }

                // std::cerr << "Received encrypted strings size: " << encrypted_strings.size() << std::endl;

                for (const auto &str : encrypted_strings)
                    res.push_back(block(block::decrypt(str)));
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("Error sending request_path request: " + std::string(e.what()));
            }
            return res;
        }

        void write_to_bucket(int array_id, int leaf_idx, int level, bckt bc) // message code 7
        {
            try
            {
                uint8_t msg_code = 7;
                boost::asio::write(socket, boost::asio::buffer(&msg_code, sizeof(msg_code)));
                boost::asio::write(socket, boost::asio::buffer(&array_id, sizeof(array_id)));
                boost::asio::write(socket, boost::asio::buffer(&leaf_idx, sizeof(leaf_idx)));
                boost::asio::write(socket, boost::asio::buffer(&level, sizeof(level)));

                // encrypt the bucket content
                std::vector<std::string> encrypted_strings;
                for (int i = 0; i < bckt::bucket_size; i++)
                    encrypted_strings.push_back(bc.blocks[i].encrypt());

                // send the size of the vector
                size_t size = encrypted_strings.size();
                boost::asio::write(socket, boost::asio::buffer(&size, sizeof(size)));

                // send the encrypted strings individually
                for (const auto &str : encrypted_strings)
                {
                    boost::asio::write(socket, boost::asio::buffer(str.data(), block::str_len));
                }
            }
            catch (std::exception &e)
            {
                throw std::runtime_error("Error sending write_to_bucket request: " + std::string(e.what()));
            }
        }
    };
}