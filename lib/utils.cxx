#include "utils.hxx"

#include <randutils.hpp>
#include <random>
#include <fstream>
#include <vector>
#include <fmt/os.h>


namespace Utils {
    
    //----------------------------------------------------------------------------
    /** Random number generator that implements the randutils auto_seed_256 seed engine.
     *  More information on the seed engine in https://www.pcg-random.org/posts/simple-portable-cpp-seed-entropy.html **/
    int random_nr(int min, int max) 
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
    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file)
            throw fmt::system_error(errno, "cannot open file '{}'", filename);

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

		fmt::print("\n ---- Loaded: {}\n", filename);
		fmt::print("FileSize: {}\n", fileSize);
		fmt::print("BufferSize: {}\n", buffer.size());
        if (buffer.size() == fileSize)
             fmt::print("ASSERT: file/shader loaded correctly\n");

        file.close();
        return buffer;
    }

} // namespace Utils
