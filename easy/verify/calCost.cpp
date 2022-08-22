#include <iostream>
#include <cassert>
#include <fstream>

#include <easy/esop/esop.hpp>
#include <easy/esop/cost.hpp>
#include <easy/io/read_esop.hpp>

/*********** USAGE
 *  ./calCost.pla <esop.pla>
************/

int main(int argc, char* argv[])
{
	assert(argc == 2);

	std::ifstream sopFile, esopFile;
	esopFile.open(argv[1], std::ios::in);

	std::stringstream esopss;
	esopss << esopFile.rdbuf();
	std::istringstream esopiss(esopss.str());

	easy::esop::esop_t Esop;
	unsigned nVars;

	easy::esop_storage_reader esopReader(Esop, nVars);
	auto result = read_pla(esopiss, esopReader);

	if(result != lorina::return_code::success)
	{
		std::cout << "ESOP pla file parse error!" << std::endl;
		return 0;
	}

    uint64_t tCount = easy::esop::T_count(Esop, nVars);
    std::cout << "T count: " << tCount << std::endl;
	return 0;
}

