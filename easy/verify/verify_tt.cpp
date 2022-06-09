#include <iostream>
#include <cassert>
#include <fstream>

#include <easy/esop/esop.hpp>
#include <easy/io/read_esop.hpp>

/*********** USAGE
 *  ./verify_tt <function.tt> <esop.pla>
************/

inline size_t twopow(size_t n)
{
	size_t ret = 1;
	for(size_t i = 0; i < n; ++i)
		ret *= 2;
	return ret;
}

int main(int argc, char* argv[])
{
	assert(argc == 3);

	std::ifstream plaFile, ttFile;
	plaFile.open(argv[1], std::ios::in);
	ttFile.open(argv[2], std::ios::in);

	std::stringstream plass;
	plass << plaFile.rdbuf();
	std::istringstream iss(plass.str());

	easy::esop::esop_t esop;
	unsigned nVars;

	easy::esop_storage_reader reader(esop, nVars);
	auto result = read_pla(iss, reader);

	if(result != lorina::return_code::success)
	{
		std::cout << "ESOP pla file parse error!" << std::endl;
		return 0;
	}

	std::stringstream ttss;
	ttss << ttFile.rdbuf();
	std::string tt(ttss.str());
	tt.erase(std::remove(tt.begin(), tt.end(), '\n'), tt.end());
	tt.erase(std::remove(tt.begin(), tt.end(), ' '), tt.end());
	std::reverse(tt.begin(), tt.end());

	std::string care;
	care.resize(twopow(nVars), '1');
	assert(care.size() == tt.size());

	bool eq = easy::esop::verify_esop(esop, tt, care);
	if(eq)
		std::cout << "Equivalent" << std::endl;
	else
		std::cout << "Not equivalent" << std::endl;

	return 0;
}

