#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include "../external/randutils/randutils.hpp"
#include "macros.h"

namespace Utils
{
    //----------------------------------------------------------------------------
    /** Basic method to get the base dir from a string. Used to get texture paths from models. **/
    inline static std::string GetBaseDir(const std::string& filepath)
    {
        if (filepath.find_last_of("/\\") != std::string::npos)
            return filepath.substr(0, filepath.find_last_of("/\\"));
        return "";
    }
    
    //----------------------------------------------------------------------------
    /** Random number generator that implements the randutils auto_seed_256 seed engine.
     *  More information on the seed engine in https://www.pcg-random.org/posts/simple-portable-cpp-seed-entropy.html **/
    inline int random_nr(int min, int max) 
    {
        std::uniform_int_distribution dist{min, max};
        static std::mt19937           engine{randutils::auto_seed_256{}.base()};
        return dist(engine);
    }

    //----------------------------------------------------------------------------
    /** Helper function to load the binary data from the shader files
     *  std::ios::ate will start reading the file at the end.
     *  The advantage of starting to read at the end of the file is
     *  that we can use the read position to determine the size of the file and allocate a buffer. **/
    inline std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        
        LOG(DASHED_SEPARATOR);
        LOG_INFO("Loaded:     %s", filename.c_str());
        LOG_INFO("FileSize:   %i", fileSize);
        LOG_INFO("BufferSize: %i", buffer.size());
        LOG(DASHED_SEPARATOR);

        file.close();
        return buffer;
    }
} // namespace Utils
