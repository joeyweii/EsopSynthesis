#include "rdExtract.h"

RdExtractManager::RdExtractManager
(
    DdManager* ddManager, 
    int level, 
    int bound,
    bool refine, 
    DdNode* FRoot, 
    int nVars
)
	: _ddManager(ddManager), _FRoot(FRoot), _level(level), _bound(bound), _nVars(nVars), _refine(refine), _values(nVars, VarValue::DONTCARE)
{ }

void RdExtractManager::extract()
{
    if(!_FRoot) return;

    partialExpand(_FRoot);
    if(_refine)
        refine(_FRoot);
    genPSDKRO(_FRoot);
}

void RdExtractManager::genPSDKRO(DdNode *F)
{
	// Reach constant 0/1
	if (F == Cudd_ReadLogicZero(_ddManager))
		return;
	if (F == Cudd_ReadOne(_ddManager))
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

	ExpType expansion = std::get<0>(_hash[F]);

	auto varIdx = Cudd_NodeReadIndex(F);
	_vars.push_back(varIdx);

    DdNode *F0, *F1, *F2;
    F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
    F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));

    if (expansion == ExpType::pD)
    {  
        F2 = Cudd_bddXor(_ddManager, F0, F1);

		_values[varIdx] = VarValue::DONTCARE;
		genPSDKRO(F0);
		_values[varIdx] = VarValue::POSITIVE;
		genPSDKRO(F2);
	}
    else if (expansion == ExpType::nD)
    {
        F2 = Cudd_bddXor(_ddManager, F0, F1);

		_values[varIdx] = VarValue::DONTCARE;
		genPSDKRO(F1);
		_values[varIdx] = VarValue::NEGATIVE;
		genPSDKRO(F2);
	}
    else
    {
        _values[varIdx] = VarValue::NEGATIVE;
		genPSDKRO(F0);
		_values[varIdx] = VarValue::POSITIVE;
		genPSDKRO(F1);
    }

    _vars.pop_back();
	_values[varIdx] = VarValue::DONTCARE;
}

int RdExtractManager::refine(DdNode *F)
{
	// Reach constant 0/1
	if (F == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (F == Cudd_ReadOne(_ddManager))
		return 1u;
        
	auto it = _hash.find(F);
    if(std::get<2>(it->second) == 1)
        return std::get<1>(it->second);

	// Calculate f0, f1, f2
    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
    F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); Cudd_Ref(F2); 

	if(std::get<0>(it->second) == ExpType::nD)
	{
        int cost1, cost2, cost0, costmax;
		cost1 = std::get<1>(_hash[F1]);
	    cost2 = std::get<1>(_hash[F2]);
		cost0 = partialExpand(F0);
		costmax = std::max(std::max(cost0, cost1), cost2);

		if(costmax == cost0)
		{
			cost1 = refine(F1);
			cost2 = refine(F2);
            std::get<1>(it->second) = cost1 + cost2;
            std::get<2>(it->second) = 1; 
			return cost1 + cost2;
		}
		else if(costmax == cost1)
		{
			cost0 = refine(F0);
			cost2 = refine(F2);
            it->second = std::make_tuple(ExpType::pD, cost0 + cost2, 1);
			return cost0 + cost2;
		}
		else if(costmax == cost2)
		{
			cost0 = refine(F0);
			cost1 = refine(F1);
			it->second = std::make_tuple(ExpType::Sh, cost0 + cost1, 1);
			return cost0 + cost1;
		}
	}
	else if(std::get<0>(it->second) == ExpType::pD)
	{
        int cost0, cost2, cost1, costmax;
		cost0 = std::get<1>(_hash[F0]);
		cost2 = std::get<1>(_hash[F2]);
		cost1 = partialExpand(F1);
		costmax = std::max(std::max(cost0, cost1), cost2);

		if(costmax == cost1)
		{
			cost0 = refine(F0);
			cost1 = refine(F2);
            std::get<1>(it->second) = cost0 + cost1;
            std::get<2>(it->second) = 1; 
			return cost0 + cost1;
		}
		else if(costmax == cost0)
		{
			cost1 = refine(F1);
			cost2 = refine(F2);
			it->second = std::make_tuple(ExpType::nD, cost1 + cost2, 1);
			return cost1 + cost2;
		}
		else if(costmax == cost2)
		{
			cost0 = refine(F0);
			cost1 = refine(F1);
			it->second = std::make_tuple(ExpType::Sh, cost0 + cost1, 1);
			return cost0 + cost1;
		}
	}
	// SHANNON
	else
	{
        int cost0, cost1, cost2, costmax;
		cost0 = std::get<1>(_hash[F0]);
		cost1 = std::get<1>(_hash[F1]);
		cost2 = partialExpand(F2);
		costmax = std::max(std::max(cost0, cost1), cost2);

		if(costmax == cost2)
		{
			cost0 = refine(F0);
			cost1 = refine(F1);
            std::get<1>(it->second) = cost0 + cost1;
            std::get<2>(it->second) = 1; 
			return cost0 + cost1;
		}
		else if(costmax == cost0)
		{
			cost1 = refine(F1);
			cost2 = refine(F2);
			it->second = std::make_tuple(ExpType::nD, cost1 + cost2, 1);
			return cost1 + cost2;
		}
		else if(costmax == cost1)
		{
			cost0 = refine(F0);
			cost2 = refine(F2);
			it->second = std::make_tuple(ExpType::pD, cost0 + cost2, 1);
			return cost0 + cost2;
		}
	}
	return 0;
}

int RdExtractManager::partialExpand(DdNode *F)
{
	// Reach constant 0/1
	if (F == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (F == Cudd_ReadOne(_ddManager))
		return 1u;
        
	auto it = _hash.find(F);
	if (it != _hash.end())
		return std::get<1>(it->second);

    if(Cudd_DagSize(F) <= _bound) return fullExpand(F);

	// Calculate f0, f1, f2
    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); Cudd_Ref(F2);

    // Calculate cost
    int cost0, cost1, cost2, costmax;
    cost0 = CostEstimate(F0); 
    cost1 = CostEstimate(F1); 
    cost2 = CostEstimate(F2);
    costmax = std::max(std::max(cost0, cost1), cost2);

    // Recursive calls
    std::tuple<ExpType, int, bool> ret;
    if(cost0 == costmax)
    {
        int n1 = partialExpand(F1);
	    int n2 = partialExpand(F2);
        ret = std::make_tuple(ExpType::nD, n1 + n2, 0);
    }
    else if(cost1 == costmax)
    {
        int n0 = partialExpand(F0);
        int n2 = partialExpand(F2);
        ret = std::make_tuple(ExpType::pD, n0 + n2, 0);
    }
    else
    {
        int n0 = partialExpand(F0);
	    int n1 = partialExpand(F1);
        ret = std::make_tuple(ExpType::Sh, n0 + n1, 0);
    }

	_hash[F] = ret;
	return std::get<1>(ret);
}

int RdExtractManager::fullExpand(DdNode *F)
{
	if (F == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (F == Cudd_ReadOne(_ddManager))
		return 1u;
		
	auto it = _hash.find(F);
	if (it != _hash.end())
		return std::get<1>(it->second);

    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); Cudd_Ref(F2);

    int cost0, cost1, cost2;
	cost0 = fullExpand(F0);
	cost1 = fullExpand(F1);
	cost2 = fullExpand(F2);

	int costmax = std::max(std::max(cost0, cost1), cost2);

	std::tuple<ExpType, int, bool> ret;
	if (costmax == cost0) 
		ret = std::make_tuple(ExpType::nD, cost1 + cost2, 1);
	else if (costmax == cost1)
		ret = std::make_tuple(ExpType::pD, cost0 + cost2, 1);
	else
		ret = std::make_tuple(ExpType::Sh, cost0 + cost1, 1);
	
	_hash[F] = ret;
	return std::get<1>(ret);
}

int RdExtractManager::CostEstimate(DdNode* F)
{ 
    return CostLookAhead(F, _level);
}

int RdExtractManager::CostLookAhead(DdNode* F, int level)
{ 
	if (F == Cudd_ReadLogicZero(_ddManager))
		return 0;
	if (F == Cudd_ReadOne(_ddManager))
		return 1;

    if(level == 1)
	    return Cudd_DagSize(F);

    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); Cudd_Ref(F2);

    int cost0, cost1, cost2;
    cost0 = CostLookAhead(F0, level-1); 
    cost1 = CostLookAhead(F1, level-1); 
    cost2 = CostLookAhead(F2, level-1);

	return cost0 + cost1 + cost2 - std::max(cost0, std::max(cost1, cost2));
}

void RdExtractManager::getESOP(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
}

int RdExtractManager::getNumTerms() const
{
    return _esop.size();
}

// extract ESOP using RdExtract algorithm and store the resulting ESOP into ret 
void RdExtractSingleOutput(Abc_Ntk_t* pNtk, int fLevel, int fBound, int fRefine, std::vector<std::string> &ESOP)
{   
    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

    int nVars = Abc_NtkPiNum(pNtk);

    DdManager* ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);

    Abc_Obj_t* pPo = Abc_NtkPo(pNtk, 0);
    DdNode* rootNode = (DdNode*) Abc_ObjGlobalBdd(pPo);
   
	// Check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > psdkro::bitwidth)
    {
		std::cout << "Cannot support nVars > " << psdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

    RdExtractManager m(ddManager, fLevel, fBound, fRefine, rootNode, nVars); 

    m.extract();	

    m.getESOP(ESOP);

    Abc_NtkFreeGlobalBdds( pNtk, 0);
    Cudd_Quit(ddManager);
}

void RdExtractMain(Abc_Ntk_t* pNtk, char* filename, int fLevel, int fBound, int fRefine, int fVerbose)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    RdExtractSingleOutput(pNtk, fLevel, fBound, fRefine, ESOP);

	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);

    if(fVerbose)
    {
        std::cout << "ESOP:" << '\n';
        for(auto& cube: ESOP)
            std::cout << cube << '\n';
    } 

	std::cout << "Time used: \t\t" << runtime << " sec" << std::endl;
	std::cout << "Memory used: \t\t" << memory << " GB" << std::endl;
    std::cout << "Number of cubes: \t\t" << ESOP.size() << std::endl;

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
