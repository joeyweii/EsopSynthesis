#include <iostream>
#include <cassert>
#include <fstream>

#include <easy/esop/esop.hpp>
#include <easy/io/read_esop.hpp>

/*********** USAGE
 *  ./verify_pla <function.pla> <esop.pla>
************/

int main(int argc, char* argv[])
{
	assert(argc == 3);

	std::ifstream sopFile, esopFile;
	sopFile.open(argv[1], std::ios::in);
	esopFile.open(argv[2], std::ios::in);

	std::stringstream esopss;
	esopss << esopFile.rdbuf();
	std::istringstream esopiss(esopss.str());

	easy::esop::esop_t esop;
	unsigned nVars_esop;

	easy::esop_storage_reader esop_reader(esop, nVars_esop);
	auto result = read_pla(esopiss, esop_reader);

	if(result != lorina::return_code::success)
	{
		std::cout << "ESOP pla file parse error!" << std::endl;
		return 0;
	}

	std::stringstream sopss;
	sopss << sopFile.rdbuf();
	std::istringstream sopiss(sopss.str());

	easy::esop::esop_t sop;
	unsigned nVars_sop;

	easy::esop_storage_reader sop_reader(sop, nVars_sop);
	result = read_pla(sopiss, sop_reader);

	if(result != lorina::return_code::success)
	{
		std::cout << "SOP pla file parse error!" << std::endl;
		return 0;
	}

	kitty::dynamic_truth_table esop_tt( nVars_esop );
	kitty::create_from_cubes( esop_tt, esop, true );

	kitty::dynamic_truth_table sop_tt( nVars_sop );
	kitty::create_from_cubes( sop_tt, sop, false );

	if(kitty::equal(sop_tt, esop_tt))
		std::cout << "Equivalent" << std::endl;
	else
		std::cout << "Not equivalent" << std::endl;
	return 0;
}

