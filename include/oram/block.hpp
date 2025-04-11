#pragma once

#include<string>

class block
{
public:
    int value;
    int idx;

    block(int v, int i) : value(v), idx(i) {};
    block(std::string serialized_block)
    {

    };

    void encrypt(int x, int y)
    {
        value ^= x, idx ^= y;
    }
    void decrypt(int x, int y)
    {
        value ^= x, idx ^= y;
    }
    std::string serialize()
    {

    }
};