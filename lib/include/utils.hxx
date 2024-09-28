#pragma once
#include <string>

namespace Utils
{
    //----------------------------------------------------------------------------
    // Basic method to get the base dir from a string. Used to get texture paths from models.
    std::string GetBaseDir(const std::string& filepath);
    
    //----------------------------------------------------------------------------
    // Random number generator that implements the randutils auto_seed_256 seed engine.
    // More information on the seed engine in https://www.pcg-random.org/posts/simple-portable-cpp-seed-entropy.html
    int random_nr(int min, int max);
}
