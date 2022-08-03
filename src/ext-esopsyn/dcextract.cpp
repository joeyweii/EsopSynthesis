#include "base/main/main.h"
#include "base/main/mainInt.h"

#include <iostream>
#include <cstring>
#include <vector>
#include <cassert>
#include <climits>

extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ESOP);

// Recursively dividing problem using shannon expansion and BddExtract  
void DivideConquerRecur(Abc_Ntk_t* pNtk, int nCofVar, std::vector<bool>& CofVarList, std::vector<std::string>& ESOP)
{
    if(nCofVar == 0)
    {
        BddExtractSingleOutput(pNtk, ESOP);
    }
    else
    {
        size_t minNode = std::numeric_limits<size_t>::max();
        int minIdx = -1;
       
        Abc_Ntk_t *p0 = NULL, *p1 = NULL;

        for(size_t i = 0, end_i = Abc_NtkPiNum(pNtk); i < end_i; ++i)
        {
            if(CofVarList[i]) continue;
           
            size_t nNode = 0; 
            p0 = AIGCOF(pNtk, i, 0);
            nNode += Abc_NtkNodeNum(p0);
            Abc_NtkDelete(p0); p0 = NULL;
            p1 = AIGCOF(pNtk, i, 1);
            nNode += Abc_NtkNodeNum(p1);
            Abc_NtkDelete(p1); p1 = NULL;
            if(nNode < minNode)
            {
                minNode = nNode;
                minIdx = i;
            }
        }

        assert(minIdx != -1);

        CofVarList[minIdx] = true;
        p0 = AIGCOF(pNtk, minIdx, 0);
        DivideConquerRecur(p0, nCofVar-1, CofVarList, ESOP);
        Abc_NtkDelete(p0); p0 = NULL;
        p1 = AIGCOF(pNtk, minIdx, 1);
        DivideConquerRecur(p1, nCofVar-1, CofVarList, ESOP);
        Abc_NtkDelete(p1); p1 = NULL;
        CofVarList[minIdx] = false;
            
    }
}

void DcExtractMain(Abc_Ntk_t* pNtk, int fNumCofVar, int fVerbose, char* filename)
{
    assert(fNumCofVar <= Abc_NtkPiNum(pNtk)); 

    std::vector<std::string> ESOP; // final ESOP
    std::vector<bool> CofVarList(Abc_NtkPiNum(pNtk), false); // Variables that have been cofactored

    abctime clk = Abc_Clock();

    DivideConquerRecur(pNtk, fNumCofVar, CofVarList, ESOP);

	std::cout << "Time used: " <<  static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC << " sec" << std::endl;
    std::cout << "Number of terms: " << ESOP.size() << std::endl;

    if(fVerbose)
    {
        std::cout << "ESOP: " << '\n';
        for(auto& cube: ESOP)
            std::cout << cube << '\n';
    }
}
