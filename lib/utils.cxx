#include <randutils.hpp>
#include <random>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils.hxx"

namespace Utils {

    int random_nr(int min, int max) 
    {
        std::uniform_int_distribution dist{min, max};
        static std::mt19937           engine{randutils::auto_seed_256{}.base()};
        return dist(engine);
    }

    /** Basic method to get the base dir from a string. Used to get texture paths from models. **/
    std::string GetBaseDir(const std::string& filepath)
    {
        if (filepath.find_last_of("/\\") != std::string::npos)
            return filepath.substr(0, filepath.find_last_of("/\\"));
        return "";
    }
    
    //----------------------------------------------------------------------------
    /** Helper function to load the binary data from the shader files
     *  std::ios::ate will start reading the file at the end.
     *  The advantage of starting to read at the end of the file is
     *  that we can use the read position to determine the size of the file and allocate a buffer. **/
    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        std::cout << std::endl;
        std::cout << "---- Loaded: " << filename << std::endl;
        std::cout << "FileSize:" << fileSize << std::endl;
        std::cout << "BufferSize:" << buffer.size() << std::endl;
        if (buffer.size() == fileSize)
            std::cout << "ASSERT: file/shader loaded correctly" << std::endl;

        file.close();
        return buffer;
    }

} // namespace Utils
