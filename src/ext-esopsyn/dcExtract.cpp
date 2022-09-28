#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "memMeasure.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cassert>
#include <climits>

extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ESOP);
extern void ArExtractSingleOutput(Abc_Ntk_t* pNtk, uint32_t level, int fRefine, std::vector<std::string>& ESOP);

// Recursively dividing problem using shannon expansion
static void DivideConquerRecur(Abc_Ntk_t* pNtk, int nCofVar, std::vector<bool>& CofVarList, std::vector<std::string>& ESOP)
{
    if(nCofVar == 0) // Solving subproblems
        BddExtractSingleOutput(pNtk, ESOP);
    else
    {
        size_t minNode = std::numeric_limits<size_t>::max();
        int minIdx = -1;
       
        Abc_Ntk_t *p0 = NULL, *p1 = NULL;

        // selection heuristic depending on AIG size
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

        // Recursively dividing problem
        size_t nCubeBefore, nCubeAfter;
        nCubeBefore = ESOP.size();
        p0 = AIGCOF(pNtk, minIdx, 0);
        DivideConquerRecur(p0, nCofVar-1, CofVarList, ESOP);
        Abc_NtkDelete(p0); p0 = NULL;
        nCubeAfter = ESOP.size();
        for(size_t i = nCubeBefore, end_i = nCubeAfter; i < end_i; ++i)
        {
            assert(ESOP[i][minIdx] == '-');
            ESOP[i][minIdx] = '0';
        }

        nCubeBefore = ESOP.size();
        p1 = AIGCOF(pNtk, minIdx, 1);
        DivideConquerRecur(p1, nCofVar-1, CofVarList, ESOP);
        Abc_NtkDelete(p1); p1 = NULL;
        nCubeAfter = ESOP.size();
        for(size_t i = nCubeBefore, end_i = nCubeAfter; i < end_i; ++i)
        {
            assert(ESOP[i][minIdx] == '-');
            ESOP[i][minIdx] = '1';
        }
        
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

	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);
	std::cout << "Time used: " << runtime << " sec" << std::endl;
    std::cout << "Memory used: " << memory << " GB" << std::endl;
    std::cout << "Number of terms: " << ESOP.size() << std::endl;

    if(fVerbose)
    {
        std::cout << "ESOP: " << '\n';
        for(auto& cube: ESOP)
            std::cout << cube << '\n';
    }

    if(filename)
    {
        std::fstream outFile;
        outFile.open(filename, std::ios::out);

        if(!outFile.is_open())
            std::cerr << "Output file failed to be opened." << std::endl;
        else
        {
            outFile << ".i " << Abc_NtkPiNum(pNtk) << '\n';
            outFile << ".o 1\n";
            outFile << ".type esop\n";
            for(auto& cube: ESOP)
                outFile << cube << " 1\n";
            outFile << ".e\n";
        }
        
        outFile.close();
    }
}
