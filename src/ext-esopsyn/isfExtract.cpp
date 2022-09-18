#include "isfExtract.h"
#include "bddExtract.h"

#include "stdio.h"

extern "C" DdNode * extraComposeCover (DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar);

IsfExtractManager::IsfExtractManager(DdManager* ddManager, DdNode* fRoot, DdNode* fcRoot, std::uint32_t nVars)
: _ddManager(ddManager), _fRoot(fRoot), _fcRoot(fcRoot), _zRoot(nullptr), _nVars(nVars) 
{
    Cudd_zddVarsFromBddVars( _ddManager, 2 );
}

void IsfExtractManager::extract()
{
    _zRoot = std::get<1>(expandExactRecur(_fRoot, _fcRoot));
}

// Argument - f: function   fc: careset
// Return   - first: new function   second: ZDD node of cover
std::tuple<DdNode*, DdNode*, uint32_t> IsfExtractManager::expandExactRecur(DdNode* f, DdNode* fc)
{
    if (f == Cudd_ReadLogicZero(_ddManager))
		return std::make_tuple(Cudd_ReadLogicZero(_ddManager), Cudd_ReadZero(_ddManager), 0u);
	if (f == Cudd_ReadOne(_ddManager))
		return std::make_tuple(Cudd_ReadOne(_ddManager), Cudd_ReadOne(_ddManager), 1u);
		
    int varIdx = Cudd_NodeReadIndex(f);

    DdNode *fc0, *fc1, *f0, *f1;

    fc0 = Cudd_Cofactor(_ddManager, fc, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(fc0);
    fc1 = Cudd_Cofactor(_ddManager, fc, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(fc1);

	f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
    
    if(fc0 == Cudd_ReadLogicZero(_ddManager))
        return expandExactRecur(f1, fc1); 
    if(fc1 == Cudd_ReadLogicZero(_ddManager))
        return expandExactRecur(f0, fc0); 

    DdNode *zRes, *fRes, *tem, *tem2, *f2, *f0_new, *f1_new, *f20_new, *f21_new, *zf0, *zf1, *zf20, *zf21;
    uint32_t c0, c1, c20, c21, c_pD, c_nD, c_Sh, c_min, cRes;
	    
    std::tie(f0_new, zf0, c0) = expandExactRecur(f0, fc0);
    std::tie(f1_new, zf1, c1) = expandExactRecur(f1, fc1);
    
	f2 = Cudd_bddXor(_ddManager, f0_new, f1_new); Cudd_Ref(f2);
    
    std::tie(f20_new, zf20, c20) = expandExactRecur(f2, fc0);
    std::tie(f21_new, zf21, c21) = expandExactRecur(f2, fc1);

    c_pD = c0 + c21;
    c_nD = c1 + c20;
    c_Sh = c0 + c1;
    c_min = std::min( std::min( c_pD, c_nD), c_Sh);

	if (c_min == c_nD) 
    {
        if(f1_new == f20_new)
        {
            zRes = cuddZddGetNode(_ddManager, varIdx*2, zf1, Cudd_ReadZero(_ddManager));

            fRes = Cudd_bddAnd(_ddManager, f1_new, Cudd_bddIthVar(_ddManager, varIdx));
            Cudd_Ref(fRes);
            
            cRes = c1;
        }
        else
        {
            zRes = cuddZddGetNode(_ddManager, varIdx*2+1, zf20, zf1);

            tem = Cudd_bddAnd(_ddManager, f20_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            fRes = Cudd_bddXor(_ddManager, tem, f1_new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, tem); 

            cRes = c20 + c1;
        }
    }
	else if (c_min == c_pD)
    {
        if(f0_new == f21_new)
        {
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf0, Cudd_ReadZero(_ddManager));

            fRes = Cudd_bddAnd(_ddManager, f0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx)));
            Cudd_Ref(fRes);

            cRes = c0;
        }
        else
        { 
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf21, zf0);

            tem = Cudd_bddAnd(_ddManager, f21_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
            fRes = Cudd_bddXor(_ddManager, tem, f0_new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, tem); 

            cRes = c21 + c0;
        }
    }
	else
    {
        if(f0_new == f1_new)
        {
            zRes = zf0; 
            fRes = f0_new;
            cRes = c0;
        }
        else
        {
            Cudd_Ref(Cudd_ReadZero(_ddManager));
            zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);

            tem = Cudd_bddAnd(_ddManager, f0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            tem2 = Cudd_bddAnd(_ddManager, f1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
            fRes = Cudd_bddXor(_ddManager, tem, tem2);
            Cudd_RecursiveDeref(_ddManager, tem);
            Cudd_RecursiveDeref(_ddManager, tem2);

            cRes = c0 + c1;
        }
    }

    Cudd_Ref(fRes);
    Cudd_Ref(zRes);    
	return std::make_tuple(fRes, zRes, cRes);
}

void IsfExtractManager::printZddCubes()
{
    std::cout << "ESOP:" << '\n';
    Cudd_zddPrintCover(_ddManager, _zRoot);
}

void IsfExtractManager::printZddNumCubes()
{
    std::cout << "Number of terms: \t\t" << Cudd_CountPathsToNonZero(_zRoot) << std::endl;
}

void IsfExtractManager::writePlaFile(char* filename)
{
    std::fstream outFile;
    outFile.open(filename, std::ios::out);

    if(!outFile.is_open())
    {
        std::cerr << "Output file cannot be opened." << std::endl;
        return;
    }

    outFile << ".i " << _nVars << '\n';
    outFile << ".o 1\n";
    outFile << ".type esop\n";

    DdGen* gen = nullptr;
    int*   cube = nullptr;
    std::string s;

    Cudd_zddForeachPath(_ddManager, _zRoot, gen, cube)
    {
        s = "";
        for(int i = 0; i < 2*_nVars; i += 2)
        {
            if(cube[i] != 1 && cube[i+1] != 1)
                s += '-';
            else if(cube[i+1] != 1)
            {
                assert(cube[i] == 1);
                s += '1';
            }
            else
            {
                assert(cube[i] != 1);
                assert(cube[i+1] == 1);
                s += '0';
            }
        } 

        outFile << s << " 1\n";
    }

    outFile << ".e\n";
    outFile.close();
}

void IsfExtractMain(Abc_Ntk_t* pNtk, int fVerbose, char* filename)
{
    assert(Abc_NtkPoNum(pNtk) == 2);

    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

    int nVars = Abc_NtkPiNum(pNtk);

    abctime clk = Abc_Clock();

    DdManager *ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);
    
    Abc_Obj_t *f = Abc_NtkPo(pNtk, 0);
    Abc_Obj_t *fc = Abc_NtkPo(pNtk, 1);
    DdNode *fRoot = (DdNode*) Abc_ObjGlobalBdd(f);
    DdNode *fcRoot = (DdNode*) Abc_ObjGlobalBdd(fc);

    IsfExtractManager m(ddManager, fRoot, fcRoot, nVars);
    m.extract();

	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);
	std::cout << "Time used: \t\t" << runtime << " sec" << std::endl;
	std::cout << "Memory used: \t\t" << memory << " GB" << std::endl;

    if(fVerbose)
        m.printZddCubes();

    if(filename)
        m.writePlaFile(filename);

    m.printZddNumCubes();
    return;
}
