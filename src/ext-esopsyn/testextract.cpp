#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "memory_measure.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <cassert>
#include <climits>

extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2);
extern void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ESOP);

// Recursively dividing problem using shannon expansion and BddExtract  
static void DivideConquerRecur(Abc_Ntk_t* pNtk, int nCofVar, std::vector<bool>& CofVarList, std::vector<std::string>& ESOP)
{
    if(nCofVar == 0)
    {
        BddExtractSingleOutput(pNtk, ESOP);
    }
    else
    {
        size_t minNode = std::numeric_limits<size_t>::max();
        int minIdx = -1;
        int minType = -1; // 0: S  1: pD  2: nD

        Abc_Ntk_t *p0 = NULL, *p1 = NULL, *p2 = NULL;

        // selection heuristic depending on AIG size
        for(size_t i = 0, end_i = Abc_NtkPiNum(pNtk); i < end_i; ++i)
        {
            if(CofVarList[i]) continue;
           
            p0 = AIGCOF(pNtk, i, 0);
            p1 = AIGCOF(pNtk, i, 1);
            p2 = AIGXOR(p0, p1);
            
            Abc_NtkRewrite( p0, 0, 0, 0, 0, 0 );
            Abc_NtkRewrite( p1, 0, 0, 0, 0, 0 );
            Abc_NtkRewrite( p2, 0, 0, 0, 0, 0 );

            size_t n0 = Abc_NtkNodeNum(p0); 
            size_t n1 = Abc_NtkNodeNum(p1); 
            size_t n2 = Abc_NtkNodeNum(p2); 

            Abc_NtkDelete(p0); p0 = NULL;
            Abc_NtkDelete(p1); p1 = NULL;
            Abc_NtkDelete(p2); p2 = NULL;
            
            size_t nmax = std::max(n0, std::max(n1, n2));
            size_t nNode = std::numeric_limits<size_t>::max();
            int type = -1; // 0: S  1: pD  2: nD

            if(nmax == n0)
            {
                type = 2;
                nNode = n1 + n2;
            }
            else if(nmax == n1)
            {
                type = 1;
                nNode = n0 + n2;
            }
            else
            {
                type = 0;
                nNode = n0 + n1;
            }

            assert(nNode != -1);

            if(nNode < minNode)
            {
                minNode = nNode;
                minIdx = i;
                minType = type;
            }
        }

        assert(minIdx != -1);
        CofVarList[minIdx] = true;
        
        // Recursively dividing problem
        if(minType == 0)
        {
            p0 = AIGCOF(pNtk, minIdx, 0);
            DivideConquerRecur(p0, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p0); p0 = NULL;
            p1 = AIGCOF(pNtk, minIdx, 1);
            DivideConquerRecur(p1, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p1); p1 = NULL;
        }
        else if(minType == 1)
        {
            p0 = AIGCOF(pNtk, minIdx, 0);
            p1 = AIGCOF(pNtk, minIdx, 1);
            p2 = AIGXOR(p0, p1);
            Abc_NtkDelete(p1); p1 = NULL;
            DivideConquerRecur(p0, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p0); p0 = NULL;
            DivideConquerRecur(p2, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p2); p2 = NULL;
        }
        else
        {
            p0 = AIGCOF(pNtk, minIdx, 0);
            p1 = AIGCOF(pNtk, minIdx, 1);
            p2 = AIGXOR(p0, p1);
            Abc_NtkDelete(p0); p0 = NULL;
            DivideConquerRecur(p1, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p1); p1 = NULL;
            DivideConquerRecur(p2, nCofVar-1, CofVarList, ESOP);
            Abc_NtkDelete(p2); p2 = NULL;
        }

        CofVarList[minIdx] = false;    
    }
}

void TestExtractMain(Abc_Ntk_t* pNtk, int fNumCofVar, int fVerbose, char* filename)
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
