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
    _zRoot = expandRecur(_fRoot, _fcRoot).second;
    Cudd_Ref(_zRoot);
}

std::pair<DdNode*, DdNode*> IsfExtractManager::expandRecur(DdNode* f, DdNode* fc)
{
    assert(fc != Cudd_ReadLogicZero(_ddManager));

	// Reach constant 0/1
    if (f == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(Cudd_ReadLogicZero(_ddManager), Cudd_ReadZero(_ddManager));
	if (f == Cudd_ReadOne(_ddManager))
		return std::make_pair(Cudd_ReadOne(_ddManager), Cudd_ReadOne(_ddManager));
		
    int varIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr, *fRes = nullptr, *tem = nullptr, *tem2 = nullptr;

    DdNode *f0c = Cudd_Cofactor(_ddManager, fc, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(f0c);
    DdNode *f1c = Cudd_Cofactor(_ddManager, fc, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(f1c);

    assert(f0c != Cudd_ReadLogicZero(_ddManager) || f1c != Cudd_ReadLogicZero(_ddManager));

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
    
    if(f0c == Cudd_ReadLogicZero(_ddManager))
    {
        return expandRecur(f1, f1c); 
    }
    if(f1c == Cudd_ReadLogicZero(_ddManager))
    {
        return expandRecur(f0, f0c); 
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

        res = expandRecur(f1, f1c);
        zf1 = res.second;
        f1new = res.first;
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);

	    f2prime = Cudd_bddXor(_ddManager, f0, f1new); Cudd_Ref(f2prime);

	    res = expandRecur(f2prime, f0c);
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

		res = expandRecur(f0, f0c);
        zf0 = res.second;
        f0new = res.first;
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);

	    f2prime = Cudd_bddXor(_ddManager, f1, f0new); Cudd_Ref(f2prime);

		res = expandRecur(f2prime, f1c);
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

		res = expandRecur(f0, f0c);
        zf0 = res.second;
        f0new = res.first;

        assert(zf0 != nullptr);
        Cudd_Ref(zf0);

		res = expandRecur(f1, f1c);
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
    Cudd_zddPrintCover(_ddManager, _zRoot);
}

void IsfExtractManager::printZddNumCubes()
{
    std::cout << "#cubes: " << Cudd_CountPathsToNonZero(_zRoot) << std::endl;
}

void IsfExtractMain(Abc_Ntk_t* pNtk)
{
    assert(Abc_NtkPoNum(pNtk) == 2);

    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

    int nVars = Abc_NtkPiNum(pNtk);
    DdManager *ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);
    
    Abc_Obj_t *f = Abc_NtkPo(pNtk, 0);
    Abc_Obj_t *fc = Abc_NtkPo(pNtk, 1);
    DdNode *fRoot = (DdNode*) Abc_ObjGlobalBdd(f);
    DdNode *fcRoot = (DdNode*) Abc_ObjGlobalBdd(fc);

    fRoot = Cudd_bddRestrict(ddManager, fRoot, fcRoot);
    IsfExtractManager m(ddManager, fRoot, fcRoot, nVars);
    // BddExtractManager m(ddManager, fRoot, nVars, 0);
    m.extract();

    //std::cout << "#cubes: " << m.getNumTerms() << std::endl;
    //m.printZddCubes();
    m.printZddNumCubes();

    return;
}
