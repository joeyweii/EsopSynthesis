#include "base/main/main.h"
#include "base/main/mainInt.h"

#include <iostream>
#include <cstring>
#include <vector>

extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ESOP);

void DivideConquerRecur(Abc_Ntk_t* pNtk, int level, std::vector<std::string>& ESOP)
{
    if(level == 0)
    {
        BddExtractSingleOutput(pNtk, ESOP);
    }
    else
    {
        Abc_Ntk_t* p0 = AIGCOF(pNtk, level-1, 0);
        DivideConquerRecur(p0, level-1, ESOP);
        Abc_NtkDelete(p0);
        Abc_Ntk_t* p1 = AIGCOF(pNtk, level-1, 1);
        DivideConquerRecur(p1, level-1, ESOP);
        Abc_NtkDelete(p1);
    }
}

void DcExtractMain(Abc_Ntk_t* pNtk, int fNumCofVar, char* filename)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    DivideConquerRecur(pNtk, fNumCofVar, ESOP);

	std::cout << "Time used: \t" <<  static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC << " sec" << std::endl;
    std::cout << "Number of terms: " << ESOP.size() << std::endl;

}
