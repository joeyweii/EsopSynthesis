#include "isfExtract.h"

extern "C" DdNode * extraComposeCover (DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar);

IsfExtractManager::IsfExtractManager(DdManager* ddManager, DdNode* fRoot, DdNode* fcRoot, std::uint32_t nVars)
: _ddManager(ddManager), _fRoot(fRoot), _fcRoot(fcRoot), _nVars(nVars), _values(nVars, VarValue::DONTCARE)

{
    Cudd_zddVarsFromBddVars( _ddManager, 2 );
}

void IsfExtractManager::extract()
{
    DdNode* zRoot = bestExpansion(_fRoot, _fcRoot);
    Cudd_zddPrintCover(_ddManager, zRoot);
}

DdNode* IsfExtractManager::bestExpansion(DdNode* f, DdNode* fc)
{
    assert(fc != Cudd_ReadLogicZero(_ddManager));

	// Reach constant 0/1
    if (f == Cudd_ReadLogicZero(_ddManager))
		return Cudd_ReadZero(_ddManager);
	if (f == Cudd_ReadOne(_ddManager))
		return Cudd_ReadOne(_ddManager);
		
    int varIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr;

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);

	uint32_t c0 = Cudd_DagSize(f);
	uint32_t c1 = Cudd_DagSize(f);
	uint32_t c2 = Cudd_DagSize(f);

	// Choose the least costly expansion 
	std::uint32_t cmax = std::max(std::max(c0, c1), c2);

	std::pair<ExpType, std::uint32_t> ret;
	if (cmax == c0) 
    {
        DdNode *zf1, *zf2;
        zf1 = bestExpansion(f1, fc);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
	    zf2 = bestExpansion(f2, fc);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _ddManager, varIdx*2+1, zf2, zf1);
        assert(zRes != nullptr);
        cuddDeref(zf1);
        cuddDeref(zf2);
    }
	else if (cmax == c1)
    {
        DdNode *zf0, *zf2;
		zf0 = bestExpansion(f0, fc);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf2 = bestExpansion(f2, fc);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2, zf0);
        assert(zRes != nullptr);
        cuddDeref(zf0);
        cuddDeref(zf2);
    }
	else
    {
        DdNode *zf0, *zf1;
		zf0 = bestExpansion(f0, fc);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf1 = bestExpansion(f1, fc);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
        Cudd_Ref(Cudd_ReadZero(_ddManager));
        // zf0, zf1, Cudd_ReadZero(_ddManager) will be deref in extraComposeCover function
        zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);
        assert(zRes != nullptr);
    }
	
	return zRes;
}

void IsfExtractManager::generatePSDKRO(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return;
	if (f == Cudd_ReadOne(_ddManager))
    {
		cube c;
		for (auto var : _vars) 
        {
			if(_values[var] != VarValue::DONTCARE) c._iscare.set(var);
			if(_values[var] == VarValue::POSITIVE) c._polarity.set(var);
		}
		_esop.push_back(c);
		return;
	}

	// Find the best expansion by a cache lookup
	ExpType expansion = _exp_cost[f].first;

	// Determine the top-most variable
	auto idx = Cudd_NodeReadIndex(f);
	_vars.push_back(idx);

    DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
    DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));

    if (expansion == ExpType::pD)
    {  
        DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1);

		_values[idx] = VarValue::DONTCARE;
		generatePSDKRO(f0);
		_values[idx] = VarValue::POSITIVE;
		generatePSDKRO(f2);

        Cudd_RecursiveDeref(_ddManager, f2);
	}
    else if (expansion == ExpType::nD)
    {
        DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1);

		_values[idx] = VarValue::DONTCARE;
		generatePSDKRO(f1);
		_values[idx] = VarValue::NEGATIVE;
		generatePSDKRO(f2);

        Cudd_RecursiveDeref(_ddManager, f2);
	}
    else
    {
        _values[idx] = VarValue::NEGATIVE;
		generatePSDKRO(f0);
		_values[idx] = VarValue::POSITIVE;
		generatePSDKRO(f1);
    }

    _vars.pop_back();
	_values[idx] = VarValue::DONTCARE;
}

void IsfExtractManager::getESOP(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
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

    IsfExtractManager m(ddManager, fRoot, fcRoot, nVars);
    
    m.extract();

    std::vector<std::string> ESOP;
    m.getESOP(ESOP);

    for(auto& cube: ESOP)
        std::cout << cube << std::endl;

    return;
}
