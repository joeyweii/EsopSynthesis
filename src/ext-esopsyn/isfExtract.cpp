#include "isfExtract.h"

#include <fstream>

IsfExtractManager::IsfExtractManager(DdManager* ddManager, DdNode* FRoot, DdNode* CRoot, int nVars, int naive)
: 
    _ddManager(ddManager), 
    _FRoot(FRoot), 
    _CRoot(CRoot), 
    _nVars(nVars), 
    _naive(naive),
    _values(nVars, VarValue::DONTCARE)
{
}

void IsfExtractManager::extract()
{
    if(_naive == 1)
    {
        firstPassNaive(_FRoot, _CRoot);
        secondPassNaive(_FRoot, _CRoot);
    }
    else
    {
        firstPass(_FRoot, _CRoot);
        secondPass(_FRoot, _CRoot);
    }
}

std::pair<DdNode*, int> IsfExtractManager::firstPassNaive(DdNode* F, DdNode* C)
{
    if (F == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(Cudd_ReadLogicZero(_ddManager), 0u);
	if (F == Cudd_ReadOne(_ddManager))
		return std::make_pair(Cudd_ReadOne(_ddManager), 1u);
		
    std::pair<DdNode*, DdNode*> F_C = std::make_pair(F, C);
    auto it = _hash.find(F_C);
    if(it != _hash.end())
       return std::make_pair(std::get<0>(it->second), std::get<3>(it->second)); 

    int varIdx = Cudd_NodeReadIndex(F);

    DdNode *C0, *C1, *F0, *F1;

    C0 = Cudd_Cofactor(_ddManager, C, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(C0);
    C1 = Cudd_Cofactor(_ddManager, C, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(C1);

	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
    
    DdNode *F0_new, *F1_new;
    int cost0, cost1;
    if(C0 == Cudd_ReadLogicZero(_ddManager))
    {
        std::tie(F1_new, cost1) = firstPass(F1, C1);
        _hash[F_C] = std::make_tuple(F1_new, nullptr, ExpType::C0, cost1);
        return std::make_pair(F1_new, cost1); 
    }
    if(C1 == Cudd_ReadLogicZero(_ddManager))
    {
        std::tie(F0_new, cost0) = firstPass(F0, C0);
        _hash[F_C] = std::make_tuple(F0_new, nullptr, ExpType::C1, cost0);
        return std::make_pair(F0_new, cost0); 
    }

    DdNode *FRet, *tem, *tem2, *F2, *C2, *F2_new;
    int cost2, cost_pD, cost_nD, cost_Sh, costmin, costRet;
	    
    std::tie(F0_new, cost0) = firstPassNaive(F0, C0);
    std::tie(F1_new, cost1) = firstPassNaive(F1, C1);
    
    if(cost0 == 0 && cost1 == 0) 
    {
        _hash[F_C] = std::make_tuple(Cudd_ReadLogicZero(_ddManager), nullptr, ExpType::F0, 0); 
        return std::make_pair(Cudd_ReadLogicZero(_ddManager), 0);
    }

	F2 = Cudd_bddXor(_ddManager, F0_new, F1_new); Cudd_Ref(F2);
	C2 = Cudd_bddOr(_ddManager, C0, C1); Cudd_Ref(C2);
    
    std::tie(F2_new, cost2) = firstPassNaive(F2, C2);

    cost_pD = cost0 + cost2;
    cost_nD = cost1 + cost2;
    cost_Sh = cost0 + cost1;
    costmin = std::min( std::min( cost_pD, cost_nD), cost_Sh);

    ExpType typeRet;

	if (costmin == cost_nD) 
    {
        assert(F1_new != F2_new);

        tem = Cudd_bddAnd(_ddManager, F2_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        FRet = Cudd_bddXor(_ddManager, tem, F1_new); Cudd_Ref(FRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        typeRet =  ExpType::nD;

        costRet = cost2 + cost1;
    }
	else if (costmin == cost_pD)
    {
        assert(F0_new != F2_new);

        tem = Cudd_bddAnd(_ddManager, F2_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
        FRet = Cudd_bddXor(_ddManager, tem, F0_new); Cudd_Ref(FRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        typeRet = ExpType::pD;

        costRet = cost2 + cost0;
    }
	else
    {
        assert(F0_new != F1_new);

        tem = Cudd_bddAnd(_ddManager, F0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        tem2 = Cudd_bddAnd(_ddManager, F1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
        FRet = Cudd_bddXor(_ddManager, tem, tem2);
        Cudd_RecursiveDeref(_ddManager, tem);
        Cudd_RecursiveDeref(_ddManager, tem2);

        typeRet = ExpType::Sh;

        costRet = cost0 + cost1;
    }

    Cudd_Ref(FRet);
    _hash[F_C] = std::make_tuple(FRet, F2, typeRet, costRet);
	return std::make_pair(FRet, costRet);
}

void IsfExtractManager::secondPassNaive(DdNode *F, DdNode* C)
{
	if (F == Cudd_ReadLogicZero(_ddManager))
		return;
	if (F == Cudd_ReadOne(_ddManager)) 
    {
		cube c; 
		for (auto var : _vars) {
			if(_values[var] != VarValue::DONTCARE) c._iscare.set(var);
			if(_values[var] == VarValue::POSITIVE) c._polarity.set(var);
		}

		_esop.push_back(c);
		return;
	}

    std::pair<DdNode*, DdNode*> F_C = std::make_pair(F, C);

    auto it = _hash.find(F_C);
	assert(it != _hash.end());

	auto varIdx = Cudd_NodeReadIndex(F);

	ExpType expansion = std::get<2>(it->second);

    if(expansion == ExpType::F0) return;

    DdNode *F0, *F1, *C0, *C1;

    C0 = Cudd_Cofactor(_ddManager, C, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(C0);
    C1 = Cudd_Cofactor(_ddManager, C, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(C1);

	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));

    if(expansion == ExpType::C0)
    {
        secondPassNaive(F1, C1);
        return;
    }
    if(expansion == ExpType::C1)
    {
        secondPassNaive(F0, C0);
        return;
    }

	_vars.push_back(varIdx);

	DdNode *F2, *C2;
    F2 = std::get<1>(it->second); 
	C2 = Cudd_bddOr(_ddManager, C0, C1); Cudd_Ref(C2);

	if (expansion == ExpType::nD)
    {
		_values[varIdx] = VarValue::DONTCARE;
        secondPassNaive(F1, C1);
		_values[varIdx] = VarValue::NEGATIVE;
        secondPassNaive(F2, C2);
	} 
    else if (expansion == ExpType::pD)
    {
		_values[varIdx] = VarValue::DONTCARE;
        secondPassNaive(F0, C0);
		_values[varIdx] = VarValue::POSITIVE;
        secondPassNaive(F2, C2);
	} 
    else 
    { 
        assert(expansion == ExpType::Sh);
		_values[varIdx] = VarValue::NEGATIVE;
        secondPassNaive(F0, C0);
		_values[varIdx] = VarValue::POSITIVE;
        secondPassNaive(F1, C1);
	}

	_vars.pop_back();
	_values[varIdx] = VarValue::DONTCARE;
}
std::pair<DdNode*, int> IsfExtractManager::firstPass(DdNode* F, DdNode* C)
{
    if (F == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(Cudd_ReadLogicZero(_ddManager), 0u);
	if (F == Cudd_ReadOne(_ddManager))
		return std::make_pair(Cudd_ReadOne(_ddManager), 1u);
		
    std::pair<DdNode*, DdNode*> F_C = std::make_pair(F, C);
    auto it = _hash.find(F_C);
    if(it != _hash.end())
       return std::make_pair(std::get<0>(it->second), std::get<3>(it->second)); 

    int varIdx = Cudd_NodeReadIndex(F);

    DdNode *C0, *C1, *F0, *F1;

    C0 = Cudd_Cofactor(_ddManager, C, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(C0);
    C1 = Cudd_Cofactor(_ddManager, C, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(C1);

	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
    
    DdNode *F0_new, *F1_new;
    int cost0, cost1;
    if(C0 == Cudd_ReadLogicZero(_ddManager))
    {
        std::tie(F1_new, cost1) = firstPass(F1, C1);
        _hash[F_C] = std::make_tuple(F1_new, nullptr, ExpType::C0, cost1);
        return std::make_pair(F1_new, cost1); 
    }
    if(C1 == Cudd_ReadLogicZero(_ddManager))
    {
        std::tie(F0_new, cost0) = firstPass(F0, C0);
        _hash[F_C] = std::make_tuple(F0_new, nullptr, ExpType::C1, cost0);
        return std::make_pair(F0_new, cost0); 
    }

    DdNode *FRet, *tem, *tem2, *F2, *F20_new, *F21_new;
    int cost20, cost21, cost_pD, cost_nD, cost_Sh, costmin, costRet;
	    
    std::tie(F0_new, cost0) = firstPass(F0, C0);
    std::tie(F1_new, cost1) = firstPass(F1, C1);
    
    if(cost0 == 0 && cost1 == 0) 
    {
        _hash[F_C] = std::make_tuple(Cudd_ReadLogicZero(_ddManager), nullptr, ExpType::F0, 0); 
        return std::make_pair(Cudd_ReadLogicZero(_ddManager), 0);
    }

	F2 = Cudd_bddXor(_ddManager, F0_new, F1_new); Cudd_Ref(F2);
    
    std::tie(F20_new, cost20) = firstPass(F2, C0);
    std::tie(F21_new, cost21) = firstPass(F2, C1);

    cost_pD = cost0 + cost21;
    cost_nD = cost1 + cost20;
    cost_Sh = cost0 + cost1;
    costmin = std::min( std::min( cost_pD, cost_nD), cost_Sh);

    ExpType typeRet;

	if (costmin == cost_nD) 
    {
        assert(F1_new != F20_new);

        tem = Cudd_bddAnd(_ddManager, F20_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        FRet = Cudd_bddXor(_ddManager, tem, F1_new); Cudd_Ref(FRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        typeRet =  ExpType::nD;

        costRet = cost20 + cost1;
    }
	else if (costmin == cost_pD)
    {
        assert(F0_new != F21_new);

        tem = Cudd_bddAnd(_ddManager, F21_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
        FRet = Cudd_bddXor(_ddManager, tem, F0_new); Cudd_Ref(FRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        typeRet = ExpType::pD;

        costRet = cost21 + cost0;
    }
	else
    {
        assert(F0_new != F1_new);

        tem = Cudd_bddAnd(_ddManager, F0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        tem2 = Cudd_bddAnd(_ddManager, F1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
        FRet = Cudd_bddXor(_ddManager, tem, tem2);
        Cudd_RecursiveDeref(_ddManager, tem);
        Cudd_RecursiveDeref(_ddManager, tem2);

        typeRet = ExpType::Sh;

        costRet = cost0 + cost1;
    }

    Cudd_Ref(FRet);
    _hash[F_C] = std::make_tuple(FRet, F2, typeRet, costRet);
	return std::make_pair(FRet, costRet);
}

void IsfExtractManager::secondPass(DdNode *F, DdNode* C)
{
	if (F == Cudd_ReadLogicZero(_ddManager))
		return;
	if (F == Cudd_ReadOne(_ddManager)) 
    {
		cube c; 
		for (auto var : _vars) {
			if(_values[var] != VarValue::DONTCARE) c._iscare.set(var);
			if(_values[var] == VarValue::POSITIVE) c._polarity.set(var);
		}

		_esop.push_back(c);
		return;
	}

    std::pair<DdNode*, DdNode*> F_C = std::make_pair(F, C);

    auto it = _hash.find(F_C);
	assert(it != _hash.end());

	auto varIdx = Cudd_NodeReadIndex(F);

	ExpType expansion = std::get<2>(it->second);

    if(expansion == ExpType::F0) return;

    DdNode *F0, *F1, *C0, *C1;

    C0 = Cudd_Cofactor(_ddManager, C, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(C0);
    C1 = Cudd_Cofactor(_ddManager, C, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(C1);

	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));

    if(expansion == ExpType::C0)
    {
        secondPass(F1, C1);
        return;
    }
    if(expansion == ExpType::C1)
    {
        secondPass(F0, C0);
        return;
    }

	_vars.push_back(varIdx);

	DdNode *F2 = std::get<1>(it->second); 
	
	if (expansion == ExpType::nD)
    {
		_values[varIdx] = VarValue::DONTCARE;
        secondPass(F1, C1);
		_values[varIdx] = VarValue::NEGATIVE;
        secondPass(F2, C0);
	} 
    else if (expansion == ExpType::pD)
    {
		_values[varIdx] = VarValue::DONTCARE;
        secondPass(F0, C0);
		_values[varIdx] = VarValue::POSITIVE;
        secondPass(F2, C1);
	} 
    else 
    { 
        assert(expansion == ExpType::Sh);
		_values[varIdx] = VarValue::NEGATIVE;
        secondPass(F0, C0);
		_values[varIdx] = VarValue::POSITIVE;
        secondPass(F1, C1);
	}

	_vars.pop_back();
	_values[varIdx] = VarValue::DONTCARE;
}

int IsfExtractManager::getNumCubes() const
{
    return _esop.size();
}

void IsfExtractManager::printESOP() const
{
    std::cout << "ESOP:" << '\n';
    for(auto &c : _esop)
        std::cout << c.str(_nVars) << '\n';
}

void IsfExtractManager::writeESOPIntoPla(char* filename)
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

    for(auto &c : _esop)
        outFile << c.str(_nVars) << " 1\n";

    outFile << ".e\n";
    outFile.close();
}

/*
std::tuple<DdNode*, DdNode*, int> IsfExtractManager::expandExactRecur(DdNode* f, DdNode* fc)
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

    DdNode *zRes, *fRet, *tem, *tem2, *f2, *f0_new, *f1_new, *f20_new, *f21_new, *zf0, *zf1, *zf20, *zf21;
    int c0, c1, c20, c21, c_pD, c_nD, c_Sh, c_min, costRet;
	    
    std::tie(f0_new, zf0, c0) = expandExactRecur(f0, fc0);
    std::tie(f1_new, zf1, c1) = expandExactRecur(f1, fc1);

    if(c0 == 0 && c1 == 0) 
        return std::make_tuple(Cudd_ReadLogicZero(_ddManager), Cudd_ReadZero(_ddManager), 0); 

	f2 = Cudd_bddXor(_ddManager, f0_new, f1_new); Cudd_Ref(f2);
    
    std::tie(f20_new, zf20, c20) = expandExactRecur(f2, fc0);
    std::tie(f21_new, zf21, c21) = expandExactRecur(f2, fc1);

    c_pD = c0 + c21;
    c_nD = c1 + c20;
    c_Sh = c0 + c1;
    c_min = std::min( std::min( c_pD, c_nD), c_Sh);

	if (c_min == c_nD) 
    {
        assert(f1_new != f20_new);
        assert(zf1 != zf20);

        zRes = cuddZddGetNode(_ddManager, varIdx*2+1, zf20, zf1);

        tem = Cudd_bddAnd(_ddManager, f20_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        fRet = Cudd_bddXor(_ddManager, tem, f1_new); Cudd_Ref(fRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        costRet = c20 + c1;
    }
	else if (c_min == c_pD)
    {
        assert(f0_new != f21_new);
        assert(zf0 != zf21);

        zRes = cuddZddGetNode( _ddManager, varIdx*2, zf21, zf0);

        tem = Cudd_bddAnd(_ddManager, f21_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem);
        fRet = Cudd_bddXor(_ddManager, tem, f0_new); Cudd_Ref(fRet);
        Cudd_RecursiveDeref(_ddManager, tem); 

        costRet = c21 + c0;
    }
	else
    {
        assert(f0_new != f1_new);
        assert(zf0 != zf1);

        Cudd_Ref(Cudd_ReadZero(_ddManager));
        zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);

        tem = Cudd_bddAnd(_ddManager, f0_new, Cudd_Not(Cudd_bddIthVar(_ddManager, varIdx))); Cudd_Ref(tem);
        tem2 = Cudd_bddAnd(_ddManager, f1_new, Cudd_bddIthVar(_ddManager, varIdx)); Cudd_Ref(tem2);
        fRet = Cudd_bddXor(_ddManager, tem, tem2);
        Cudd_RecursiveDeref(_ddManager, tem);
        Cudd_RecursiveDeref(_ddManager, tem2);

        costRet = c0 + c1;
    }

    Cudd_Ref(fRet);
    Cudd_Ref(zRes);    
	return std::make_tuple(fRet, zRes, costRet);
}

void IsfExtractManager::printZddNumCubes()
{
    std::cout << "Number of terms: \t\t" << Cudd_CountPathsToNonZero(_zRoot) << std::endl;
}

void IsfExtractManager::printZddCubes()
{
    std::cout << "ESOP:" << '\n';
    Cudd_zddPrintCover(_ddManager, _zRoot);
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
*/

void IsfExtractMain(Abc_Ntk_t* pNtk, int fVerbose, int fNaive, char* filename)
{
    assert(Abc_NtkPoNum(pNtk) == 2);

    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

    int nVars = Abc_NtkPiNum(pNtk);

    abctime clk = Abc_Clock();

    DdManager *ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);
    
    Abc_Obj_t *f = Abc_NtkPo(pNtk, 0);
    Abc_Obj_t *fc = Abc_NtkPo(pNtk, 1);
    DdNode *FRoot = (DdNode*) Abc_ObjGlobalBdd(f);
    DdNode *CRoot = (DdNode*) Abc_ObjGlobalBdd(fc);

    IsfExtractManager m(ddManager, FRoot, CRoot, nVars, fNaive);
    m.extract();

	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);

    if(fVerbose)
    {
        m.printESOP();
    }

	std::cout << "Time used: \t\t" << runtime << " sec" << std::endl;
	std::cout << "Memory used: \t\t" << memory << " GB" << std::endl;
    std::cout << "Number of cubes: \t\t" << m.getNumCubes() << std::endl;

    if(filename)
        m.writeESOPIntoPla(filename);

    return;
}
