#pragma once

#include <random>

inline int rng(int x)
{
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, x - 1);
    return dist(gen);
}
