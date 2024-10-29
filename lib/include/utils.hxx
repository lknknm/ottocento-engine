#pragma once
#include <string>
#include <vector>


namespace Utils
{

    //----------------------------------------------------------------------------
    /** Random number generator that implements the randutils auto_seed_256 seed engine.
     *  More information on the seed engine in https://www.pcg-random.org/posts/simple-portable-cpp-seed-entropy.html **/
    int random_nr(int min, int max);

    //----------------------------------------------------------------------------
    /** Helper function to load the binary data from the shader files
     *  std::ios::ate will start reading the file at the end.
     *  The advantage of starting to read at the end of the file is
     *  that we can use the read position to determine the size of the file and allocate a buffer. **/
    std::vector<char> readFile(const std::string& filename);
}