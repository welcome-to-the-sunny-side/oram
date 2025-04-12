#pragma once

#include<string>

#include "encrypt.hpp"

namespace oram_lib
{
    class block
    {
    public:
        int val;
        int idx;
        bool rsrvd = false;

        block(int v, int i) : val(v), idx(i), rsrvd(false) {};
        block() : block(0, 0) {};
        block(std::string serialized_block)
        {
            size_t delimiter_pos = serialized_block.find("|");
            if (delimiter_pos != std::string::npos)
            {
                val = std::stoi(serialized_block.substr(0, delimiter_pos));
                idx = std::stoi(serialized_block.substr(delimiter_pos + 1));
            }
            else
            {
                val = 0;
                idx = 0;
            }
            rsrvd = false;
        };

        std::string encrypt()
        {
            return Encryptor::encrypt(serialize());
        }

        static std::string decrypt(std::string encrypted_block)
        {
            return Encryptor::decrypt(encrypted_block);
        }
        
        std::string serialize()
        {
            return (std::to_string(val) + "|" + std::to_string(idx));
        }
    };
}