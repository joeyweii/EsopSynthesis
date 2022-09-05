#include <iostream>
#include <cassert>
#include <fstream>

#include <easy/esop/esop.hpp>
#include <easy/io/read_esop.hpp>

/*********** USAGE
 *  ./verify_pla_dc <function.pla> <careset.pla> <esop.pla>
************/

int main(int argc, char* argv[])
{
	assert(argc == 4);

	std::ifstream functionFile, caresetFile, esopFile;
	functionFile.open(argv[1], std::ios::in);
	caresetFile.open(argv[2], std::ios::in);
	esopFile.open(argv[3], std::ios::in);

    if(!functionFile.is_open())
    {
        std::cerr << "Function PLA file cannot be opened." << std::endl;
    }

    if(!caresetFile.is_open())
    {
        std::cerr << "Careset PLA file cannot be opened." << std::endl;
    }

    if(!esopFile.is_open())
    {
        std::cerr << "ESOP PLA file cannot be opened." << std::endl;
    }

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

	std::stringstream functionss;
	functionss << functionFile.rdbuf();
	std::istringstream functioniss(functionss.str());

	easy::esop::esop_t function;
	unsigned nVars_function;

	easy::esop_storage_reader function_reader(function, nVars_function);
	result = read_pla(functioniss, function_reader);

	if(result != lorina::return_code::success)
	{
		std::cout << "Function pla file parse error!" << std::endl;
		return 0;
	}

	std::stringstream caresetss;
	caresetss << caresetFile.rdbuf();
	std::istringstream caresetiss(caresetss.str());

	easy::esop::esop_t careset;
	unsigned nVars_careset;

	easy::esop_storage_reader careset_reader(careset, nVars_careset);
	result = read_pla(caresetiss, careset_reader);

	if(result != lorina::return_code::success)
	{
		std::cout << "Careset pla file parse error!" << std::endl;
		return 0;
	}

    assert(nVars_esop == nVars_function);
    assert(nVars_careset == nVars_function);

	kitty::dynamic_truth_table function_tt( nVars_function );
	kitty::create_from_cubes( function_tt, function, false );

	kitty::dynamic_truth_table careset_tt( nVars_careset );
	kitty::create_from_cubes( careset_tt, careset, false );

    if(easy::esop::implements_function(esop, function_tt, careset_tt, nVars_esop))
        std::cout << "Equivalent." << std::endl;
    else
        std::cout << "Not equivalent." << std::endl;

	return 0;
}

