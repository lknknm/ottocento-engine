#pragma once
#include <string>
#include "../external/randutils/randutils.hpp"

namespace Utils
{
    //----------------------------------------------------------------------------
    // Basic method to get the base dir from a string. Used to get texture paths from models.
    static std::string GetBaseDir(const std::string& filepath)
    {
        if (filepath.find_last_of("/\\") != std::string::npos)
            return filepath.substr(0, filepath.find_last_of("/\\"));
        return "";
    }
    
    //----------------------------------------------------------------------------
    // Random number generator that implements the randutils auto_seed_256 seed engine.
    // More information on the seed engine in https://www.pcg-random.org/posts/simple-portable-cpp-seed-entropy.html
    int random_nr(int min, int max) 
    {
        std::uniform_int_distribution dist{min, max};
        static std::mt19937           engine{randutils::auto_seed_256{}.base()};
        return dist(engine);
    }
}
