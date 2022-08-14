#include "arextract.h"

ArExtractManager::ArExtractManager(DdManager* ddManager, std::uint32_t level, bool refine, DdNode* rootNode, std::uint32_t nVars)
	: _ddManager(ddManager), _rootNode(rootNode), _level(level), _nVars(nVars), _refine(refine), _values(nVars, VarValue::DONTCARE)
{ }

void ArExtractManager::extract()
{
    startingCover(_rootNode);
    if(_refine)
        refine(_rootNode);
    generatePSDKRO(_rootNode);
}

void ArExtractManager::generatePSDKRO(DdNode *f)
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

uint32_t ArExtractManager::refine(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (f == Cudd_ReadOne(_ddManager))
		return 1u;
        
	auto iter_f = _exp_cost.find(f);

	// Calculate f0, f1, f2
	DdNode* f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode* f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode* f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2); 

	if(iter_f->second.first == ExpType::nD)
	{
		auto c1 = _exp_cost[f1].second;
		auto c2 = _exp_cost[f2].second;

		uint32_t c0 = startingCover(f0);
		uint32_t cmax = std::max(std::max(c0, c1), c2);

		if(cmax == c0)
		{
			c1 = refine(f1);
			c2 = refine(f2);
			iter_f->second.second = c1 + c2;
			return c1 + c2;
		}
		else if(cmax == c1)
		{
			c0 = refine(f0);
			c2 = refine(f2);
			iter_f->second = std::make_pair(ExpType::pD, c0 + c2);
			return c0 + c2;
		}
		else if(cmax == c2)
		{
			c0 = refine(f0);
			c1 = refine(f1);
			iter_f->second = std::make_pair(ExpType::S, c0 + c1);
			return c0 + c1;
		}
	}
	else if(iter_f->second.first == ExpType::pD)
	{
		auto c0 = _exp_cost[f0].second;
		auto c2 = _exp_cost[f2].second;

		uint32_t c1 = startingCover(f1);
		uint32_t cmax = std::max(std::max(c0, c1), c2);

		if(cmax == c1)
		{
			c0 = refine(f0);
			c1 = refine(f2);
			iter_f->second.second = c0 + c1;
			return c0 + c1;
		}
		else if(cmax == c0)
		{
			c1 = refine(f1);
			c2 = refine(f2);
			iter_f->second = std::make_pair(ExpType::nD, c1 + c2);
			return c1 + c2;
		}
		else if(cmax == c2)
		{
			c0 = refine(f0);
			c1 = refine(f1);
			iter_f->second = std::make_pair(ExpType::S, c0 + c1);
			return c0 + c1;
		}
	}
	// SHANNON
	else
	{
		auto c0 = _exp_cost[f0].second;
		auto c1 = _exp_cost[f1].second;

		uint32_t c2 = startingCover(f2);
		uint32_t cmax = std::max(std::max(c0, c1), c2);

		if(cmax == c2)
		{
			c0 = refine(f0);
			c1 = refine(f1);
			iter_f->second.second = c0 + c1;
			return c0 + c1;
		}
		else if(cmax == c0)
		{
			c1 = refine(f1);
			c2 = refine(f2);
			iter_f->second = std::make_pair(ExpType::nD, c1 + c2);
			return c1 + c2;
		}
		else if(cmax == c1)
		{
			c0 = refine(f0);
			c2 = refine(f2);
			iter_f->second = std::make_pair(ExpType::pD, c0 + c2);
			return c0 + c2;
		}
	}
	return 0;
}

std::uint32_t ArExtractManager::startingCover(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (f == Cudd_ReadOne(_ddManager))
		return 1u;
        
	auto it = _exp_cost.find(f);
	if (it != _exp_cost.end())
		return it->second.second;

	// Calculate f0, f1, f2
	DdNode* f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode* f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode* f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);

    // Calculate cost
    uint32_t c0 = CostFunction(f0); 
    uint32_t c1 = CostFunction(f1); 
    uint32_t c2 = CostFunction(f2);

    // Recursive calls
    uint32_t cmax = std::max(std::max(c0, c1), c2);

    std::pair<ExpType, std::uint32_t> ret;
    if(c0 == cmax)
    {
        std::uint32_t n1 = startingCover(f1);
	    std::uint32_t n2 = startingCover(f2);
        ret = std::make_pair(ExpType::nD, n1 + n2);
    }
    else if(c1 == cmax)
    {
        std::uint32_t n0 = startingCover(f0);
        std::uint32_t n2 = startingCover(f2);
        ret = std::make_pair(ExpType::pD, n0 + n2);
    }
    else
    {
        // Cudd_RecursiveDeref(_ddManager, f2);
        std::uint32_t n0 = startingCover(f0);
	    std::uint32_t n1 = startingCover(f1);
        ret = std::make_pair(ExpType::S, n0 + n1);
    }

	_exp_cost[f] = ret;
	return ret.second;
}

uint32_t ArExtractManager::CostFunction(DdNode* f)
{ 
    return CostFunctionLevel(f, _level-1);
}

uint32_t ArExtractManager::CostFunctionLevel(DdNode* f, int level)
{ 
	if (f == Cudd_ReadLogicZero(_ddManager))
		return 0;
	if (f == Cudd_ReadOne(_ddManager))
		return 1;

	// Path Approximation
	// if(level == 0) return  Cudd_CountPathsToNonZero(f);
	// Node Approximation
	// if(level == 0) return Cudd_DagSize(f);
	// Hybrid Approximation
	if(level == 0) 
		return (Cudd_NodeReadIndex(f) >= _nVars - level - 3)? Cudd_CountPathsToNonZero(f) :  Cudd_DagSize(f);

	// Calculate f0, f1, f2
	DdNode* f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode* f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode* f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);

    uint32_t c0 = CostFunctionLevel(f0, level-1); 
    uint32_t c1 = CostFunctionLevel(f1, level-1); 
    uint32_t c2 = CostFunctionLevel(f2, level-1);

	return c0 + c1 + c2 - std::max(c0, std::max(c1, c2));
}

void ArExtractManager::getESOP(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
}

uint32_t ArExtractManager::getNumTerms() const
{
    return _esop.size();
}

// extract ESOP using ArExtract algorithm and store the resulting ESOP into ret 
void ArExtractSingleOutput(Abc_Ntk_t* pNtk, uint32_t level, int fRefine, std::vector<std::string>& ret)
{   
    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

	// Number of variables(Pis) in pNtk
    int nVars = Abc_NtkPiNum(pNtk);

    DdManager* ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);

    Abc_Obj_t* pPo = Abc_NtkPo(pNtk, 0);
    DdNode* rootNode = (DdNode*) Abc_ObjGlobalBdd(pPo);
   
	// Check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > psdkro::bitwidth) {
		std::cout << "Cannot support nVars > " << psdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

    ArExtractManager m(ddManager, level, fRefine, rootNode, nVars); 

    // Extract
    m.extract();	

    m.getESOP(ret);

    // Delete global BDD
    Abc_NtkFreeGlobalBdds( pNtk, 0);
    Cudd_Quit(ddManager);
}

void ArExtractMain(Abc_Ntk_t* pNtk, char* filename, int fLevel, int fRefine, int fVerbose)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    ArExtractSingleOutput(pNtk, fLevel, fRefine, ESOP);

	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);
	std::cout << "Time used: \t\t" << runtime << " sec" << std::endl;
	std::cout << "Memory used: \t\t" << memory << " GB" << std::endl;
    std::cout << "Number of terms: \t\t" << ESOP.size() << std::endl;

    if(fVerbose)
    {
        std::cout << "ESOP:" << '\n';
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
