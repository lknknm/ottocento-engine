#include <randutils.hpp>
#include <random>
#include "utils.hxx"
namespace Utils {

std::string GetBaseDir(const std::string& filepath)
{
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

int random_nr(int min, int max) 
{
	std::uniform_int_distribution dist{min, max};
	static std::mt19937           engine{randutils::auto_seed_256{}.base()};
	return dist(engine);
}
} // namespace Utils