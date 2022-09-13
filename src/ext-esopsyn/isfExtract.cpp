#include "isfExtract.h"
#include "bddExtract.h"

#include "stdio.h"

extern "C" DdNode * extraComposeCover (DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar);
extern "C" void Abc_ShowFile( char * FileNameDot );

IsfExtractManager::IsfExtractManager(DdManager* ddManager, DdNode* fRoot, DdNode* fcRoot, std::uint32_t nVars)
: _ddManager(ddManager), _fRoot(fRoot), _fcRoot(fcRoot), _zRoot(nullptr), _nVars(nVars) 
{
    Cudd_zddVarsFromBddVars( _ddManager, 2 );
}

void IsfExtractManager::extract()
{
    _zRoot = expandExactRecur(_fRoot, _fcRoot).second;
}

// Argument - f: function   fc: careset
// Return   - first: new function   second: ZDD node of cover
std::pair<DdNode*, DdNode*> IsfExtractManager::expandExactRecur(DdNode* f, DdNode* fc)
{
    assert(fc != Cudd_ReadLogicZero(_ddManager));

	// Reach constant 0/1
    if (f == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(Cudd_ReadLogicZero(_ddManager), Cudd_ReadZero(_ddManager));
	if (f == Cudd_ReadOne(_ddManager))
		return std::make_pair(Cudd_ReadOne(_ddManager), Cudd_ReadOne(_ddManager));
		
    int varIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr, *fRes = nullptr, *tem = nullptr, *tem2 = nullptr;

    DdNode *fc0 = Cudd_Cofactor(_ddManager, fc, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(fc0);
    DdNode *fc1 = Cudd_Cofactor(_ddManager, fc, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(fc1);

    assert(fc0 != Cudd_ReadLogicZero(_ddManager) || fc1 != Cudd_ReadLogicZero(_ddManager));

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);
    
    if(fc0 == Cudd_ReadLogicZero(_ddManager))
    {
        return expandExactRecur(f1, fc1); 
    }
    if(fc1 == Cudd_ReadLogicZero(_ddManager))
    {
        return expandExactRecur(f0, fc0); 
    }

    DdNode *f0_new, *f1_new, *f2_0c_new, *f2_1c_new, *zf0, *zf1, *zf2_0c, *zf2_1c;
    DdNode *f2_prime;
	    
    std::pair<DdNode*, DdNode*> res;
    res = expandExactRecur(f0, fc0);
    f0_new = res.first;
    zf0 = res.second;
    res = expandExactRecur(f1, fc1);
    f1_new = res.first;
    zf1 = res.second;
    
	f2_prime = Cudd_bddXor(_ddManager, f0_new, f1_new); Cudd_Ref(f2_prime);
    
    res = expandExactRecur(f2_prime, fc0);
    f2_0c_new = res.first;
    zf2_0c = res.second;
    res = expandExactRecur(f2_prime, fc1);
    f2_1c_new = res.first;
    zf2_1c = res.second;
   
    uint32_t c0, c1, c2_0c, c2_1c;     
    c0 = Cudd_CountPathsToNonZero(zf0);
    c1 = Cudd_CountPathsToNonZero(zf1);
    c2_0c = Cudd_CountPathsToNonZero(zf2_0c);
    c2_1c = Cudd_CountPathsToNonZero(zf2_1c);

    uint32_t c_pD, c_nD, c_Sh, c_min;
    c_pD = c0 + c2_1c;
    c_nD = c1 + c2_0c;
    c_Sh = c0 + c1;
    c_min = std::min( std::min( c_pD, c_nD), c_Sh);

	if (c_min == c_nD) 
    {
        if(f1_new == f2_0c_new)
        {
            assert(zf1 == zf2_0c);
            zRes = cuddZddGetNode(_ddManager, varIdx*2, zf1, Cudd_ReadZero(_ddManager));
        }
        else 
            zRes = cuddZddGetNode(_ddManager, varIdx*2+1, zf2_0c, zf1);

        assert(zRes != nullptr);

        if(f1_new == f2_0c_new)
        {
            fRes = Cudd_bddAnd(_ddManager, f1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(fRes);
        }
        else
        {
            tem = Cudd_bddAnd(_ddManager, f2_0c_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            fRes = Cudd_bddXor(_ddManager, tem, f1_new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, tem); 
        }
    }
	else if (c_min == c_pD)
    {
        if(f0_new == f2_1c_new)
        {
            assert(zf0 == zf2_1c);
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf0, Cudd_ReadZero(_ddManager));
        }
        else
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2_1c, zf0);

        assert(zRes != nullptr);

        if(f0_new == f2_1c_new)
        {
            fRes = Cudd_bddAnd(_ddManager, f0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(fRes);
        }
        else
        { 
            tem = Cudd_bddAnd(_ddManager, f2_1c_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
            fRes = Cudd_bddXor(_ddManager, tem, f0_new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, tem); 
        }
    }
	else
    {
        if(f0_new == f1_new)
        {   
            assert(zf0 == zf1);
            zRes = zf0; 
        }
        else
        {
            // zf0, zf1, Cudd_ReadZero(_ddManager) will be deref in extraComposeCover function
            Cudd_Ref(Cudd_ReadZero(_ddManager));
            zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);
        }
        assert(zRes != nullptr);
        
        if(f0_new == f1_new)
        {
            fRes = f0_new;
        }
        else
        {
            tem = Cudd_bddAnd(_ddManager, f0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            tem2 = Cudd_bddAnd(_ddManager, f1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
            fRes = Cudd_bddXor(_ddManager, tem, tem2);
            Cudd_RecursiveDeref(_ddManager, tem);
            Cudd_RecursiveDeref(_ddManager, tem2);
        }
    }
    /*
    Cudd_RecursiveDeref(_ddManager, f0_new);
    Cudd_RecursiveDeref(_ddManager, f1_new);
    Cudd_RecursiveDeref(_ddManager, f2_0c_new);
    Cudd_RecursiveDeref(_ddManager, f2_1c_new);
    Cudd_Deref(zf0);
    Cudd_Deref(zf1);
    Cudd_Deref(zf2_0c);
    Cudd_Deref(zf2_1c);
    */
    Cudd_Ref(fRes);
    Cudd_Ref(zRes);    
	return std::make_pair(fRes, zRes);
}

std::pair<DdNode*, DdNode*> IsfExtractManager::expandHeuristicRecur(DdNode* f, DdNode* fc)
{
    assert(fc != Cudd_ReadLogicZero(_ddManager));

	// Reach constant 0/1
    if (f == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(Cudd_ReadLogicZero(_ddManager), Cudd_ReadZero(_ddManager));
	if (f == Cudd_ReadOne(_ddManager))
		return std::make_pair(Cudd_ReadOne(_ddManager), Cudd_ReadOne(_ddManager));
		
    int varIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr, *fRes = nullptr, *tem = nullptr, *tem2 = nullptr;

    DdNode *fc0 = Cudd_Cofactor(_ddManager, fc, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(fc0);
    DdNode *fc1 = Cudd_Cofactor(_ddManager, fc, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(fc1);

    assert(fc0 != Cudd_ReadLogicZero(_ddManager) || fc1 != Cudd_ReadLogicZero(_ddManager));

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
    
    if(fc0 == Cudd_ReadLogicZero(_ddManager))
    {
        return expandHeuristicRecur(f1, fc1); 
    }
    if(fc1 == Cudd_ReadLogicZero(_ddManager))
    {
        return expandHeuristicRecur(f0, fc0); 
    }

	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);

	uint32_t c0 = Cudd_CountPathsToNonZero(f0);
	uint32_t c1 = Cudd_CountPathsToNonZero(f1);
	uint32_t c2 = Cudd_CountPathsToNonZero(f2);

	// Choose the least costly expansion 
	std::uint32_t cmax = std::max(std::max(c0, c1), c2);

	if (cmax == c0) 
    {
        DdNode *zf1, *zf2, *f1new, *f2new, *f2prime;
        std::pair<DdNode*, DdNode*> res;

        res = expandHeuristicRecur(f1, fc1);
        zf1 = res.second;
        f1new = res.first;
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);

	    f2prime = Cudd_bddXor(_ddManager, f0, f1new); Cudd_Ref(f2prime);

	    res = expandHeuristicRecur(f2prime, fc0);
        zf2 = res.second;
        f2new = res.first;
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);

        if(f1new == f2new)
        {
            assert(zf1 == zf2);
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2, Cudd_ReadZero(_ddManager));
        }
        else 
            zRes = cuddZddGetNode( _ddManager, varIdx*2+1, zf2, zf1);

        assert(zRes != nullptr);
        cuddDeref(zf1);
        cuddDeref(zf2);

        if(f1new == f2new)
        {
            fRes = Cudd_bddAnd(_ddManager, f2new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(fRes);
        }
        else
        {
            tem = Cudd_bddAnd(_ddManager, f2new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            Cudd_RecursiveDeref(_ddManager, f2new);
            fRes = Cudd_bddXor(_ddManager, tem, f1new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, f1new);
            Cudd_RecursiveDeref(_ddManager, tem); 
        }
    }
	else if (cmax == c1)
    {
        DdNode *zf0, *zf2, *f0new, *f2new, *f2prime;
        std::pair<DdNode*, DdNode*> res;

		res = expandHeuristicRecur(f0, fc0);
        zf0 = res.second;
        f0new = res.first;
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);

	    f2prime = Cudd_bddXor(_ddManager, f1, f0new); Cudd_Ref(f2prime);

		res = expandHeuristicRecur(f2prime, fc1);
        zf2 = res.second;
        f2new = res.first;
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
     
        if(f0new == f2new)
        {
            assert(zf2 == zf0);
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2, Cudd_ReadZero(_ddManager));
        }
        else
            zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2, zf0);

        assert(zRes != nullptr);
        cuddDeref(zf0);
        cuddDeref(zf2);

        if(f0new == f2new)
        {
            fRes = Cudd_bddAnd(_ddManager, f0new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(fRes);
        }
        else
        { 
            tem = Cudd_bddAnd(_ddManager, f2new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
            Cudd_RecursiveDeref(_ddManager, f2new);
            fRes = Cudd_bddXor(_ddManager, tem, f0new); Cudd_Ref(fRes);
            Cudd_RecursiveDeref(_ddManager, f0new);
            Cudd_RecursiveDeref(_ddManager, tem); 
        }
    }
	else
    {
        DdNode *zf0, *zf1, *f0new, *f1new;
        std::pair<DdNode*, DdNode*> res;

		res = expandHeuristicRecur(f0, fc0);
        zf0 = res.second;
        f0new = res.first;

        assert(zf0 != nullptr);
        Cudd_Ref(zf0);

		res = expandHeuristicRecur(f1, fc1);
        zf1 = res.second;
        f1new = res.first;
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);

        if(f0new == f1new)
        {   
            assert(zf0 == zf1);
            zRes = zf0; 
            Cudd_Deref(zf1);
        }
        else
        {
            // zf0, zf1, Cudd_ReadZero(_ddManager) will be deref in extraComposeCover function
            Cudd_Ref(Cudd_ReadZero(_ddManager));
            zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);
        }
        assert(zRes != nullptr);
        
        if(f0new == f1new)
        {
            fRes = f0new;
        }
        else
        {
            tem = Cudd_bddAnd(_ddManager, f0new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
            Cudd_RecursiveDeref(_ddManager, f0new);
            tem2 = Cudd_bddAnd(_ddManager, f1new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
            Cudd_RecursiveDeref(_ddManager, f1new);
            fRes = Cudd_bddXor(_ddManager, tem, tem2);
            Cudd_RecursiveDeref(_ddManager, tem);
            Cudd_RecursiveDeref(_ddManager, tem2);
        }
    }
	
	return std::make_pair(fRes, zRes);
}

void IsfExtractManager::dumpZddDot()
{
    FILE* pFile; 
    pFile = fopen( "./out.dot", "w" );
    Cudd_zddDumpDot(_ddManager, 1, (DdNode**) &_zRoot, NULL, NULL, pFile); 
    fclose(pFile);
    Abc_ShowFile("./out.dot");
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
