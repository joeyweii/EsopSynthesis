#include "arextract.h"

ArExtractManager::ArExtractManager(DdManager* _ddmanager, uint32_t nVars, uint32_t fLevel)
	: _ddmanager(_ddmanager), _nVars(nVars) , _values(nVars, UNUSED), _level(fLevel)
{ }

void ArExtractManager::init()
{
	_exp_cost.clear();
	_esop.clear();
	std::fill(_values.begin(), _values.end(), UNUSED);
}

void ArExtractManager::generate_psdkro(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddmanager))
		return;

	if (f == Cudd_ReadOne(_ddmanager)) {
		cube c; // mask & polarity initialized 0
		for (auto var : _vars) {
			if(_values[var] != UNUSED) c.mask.set(var);
			if(_values[var] == POSITIVE) c.polarity.set(var);
		}

		_esop.push_back(c);
		return;
	}

	// Find the best expansion by a cache lookup
	exp_type expansion = _exp_cost[f].first;

	// Determine the top-most variable
	auto idx = Cudd_NodeReadIndex(f);
	_vars.push_back(idx);

    DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
    DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));

    if (expansion == POSITIVE_DAVIO) {  
        DdNode *f2 = Cudd_bddXor(_ddmanager, f0, f1);

		_values[idx] = UNUSED;
		generate_psdkro(f0);
		_values[idx] = POSITIVE;
		generate_psdkro(f2);

        Cudd_RecursiveDeref(_ddmanager, f2);
	}
    else if (expansion == NEGATIVE_DAVIO) {
        DdNode *f2 = Cudd_bddXor(_ddmanager, f0, f1);

		_values[idx] = UNUSED;
		generate_psdkro(f1);
		_values[idx] = NEGATIVE;
		generate_psdkro(f2);

        Cudd_RecursiveDeref(_ddmanager, f2);
	}
    else{
        _values[idx] = NEGATIVE;
		generate_psdkro(f0);
		_values[idx] = POSITIVE;
		generate_psdkro(f1);
    }

    _vars.pop_back();
	_values[idx] = UNUSED;
	
}

uint32_t ArExtractManager::refine(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddmanager))
		return 0u;
	if (f == Cudd_ReadOne(_ddmanager))
		return 1u;
        
	auto iter_f = _exp_cost.find(f);

	// Calculate f0, f1, f2
	DdNode* f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode* f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode* f2 = Cudd_bddXor(_ddmanager, f0, f1); Cudd_Ref(f2); 

	if(iter_f->second.first == NEGATIVE_DAVIO)
	{
		auto cost_f1 = _exp_cost[f1].second;
		auto cost_f2 = _exp_cost[f2].second;

		uint32_t cost_f0 = starting_cover(f0);
		uint32_t cost_max = std::max(std::max(cost_f0, cost_f1), cost_f2);

		if(cost_max == cost_f0)
		{
			cost_f1 = refine(f1);
			cost_f2 = refine(f2);
			iter_f->second.second = cost_f1 + cost_f2;
			return cost_f1 + cost_f2;
		}
		else if(cost_max == cost_f1)
		{
			cost_f0 = refine(f0);
			cost_f2 = refine(f2);
			iter_f->second = std::make_pair(POSITIVE_DAVIO, cost_f0 + cost_f2);
			return cost_f0 + cost_f2;
		}
		else if(cost_max == cost_f2)
		{
			cost_f0 = refine(f0);
			cost_f1 = refine(f1);
			iter_f->second = std::make_pair(SHANNON, cost_f0 + cost_f1);
			return cost_f0 + cost_f1;
		}
	}
	else if(iter_f->second.first == POSITIVE_DAVIO)
	{
		auto cost_f0 = _exp_cost[f0].second;
		auto cost_f2 = _exp_cost[f2].second;

		uint32_t cost_f1 = starting_cover(f1);
		uint32_t cost_max = std::max(std::max(cost_f0, cost_f1), cost_f2);

		if(cost_max == cost_f1)
		{
			cost_f0 = refine(f0);
			cost_f1 = refine(f2);
			iter_f->second.second = cost_f0 + cost_f1;
			return cost_f0 + cost_f1;
		}
		else if(cost_max == cost_f0)
		{
			cost_f1 = refine(f1);
			cost_f2 = refine(f2);
			iter_f->second = std::make_pair(NEGATIVE_DAVIO, cost_f1 + cost_f2);
			return cost_f1 + cost_f2;
		}
		else if(cost_max == cost_f2)
		{
			cost_f0 = refine(f0);
			cost_f1 = refine(f1);
			iter_f->second = std::make_pair(SHANNON, cost_f0 + cost_f1);
			return cost_f0 + cost_f1;
		}
	}
	// SHANNON
	else
	{
		auto cost_f0 = _exp_cost[f0].second;
		auto cost_f1 = _exp_cost[f1].second;

		uint32_t cost_f2 = starting_cover(f2);
		uint32_t cost_max = std::max(std::max(cost_f0, cost_f1), cost_f2);

		if(cost_max == cost_f2)
		{
			cost_f0 = refine(f0);
			cost_f1 = refine(f1);
			iter_f->second.second = cost_f0 + cost_f1;
			return cost_f0 + cost_f1;
		}
		else if(cost_max == cost_f0)
		{
			cost_f1 = refine(f1);
			cost_f2 = refine(f2);
			iter_f->second = std::make_pair(NEGATIVE_DAVIO, cost_f1 + cost_f2);
			return cost_f1 + cost_f2;
		}
		else if(cost_max == cost_f1)
		{
			cost_f0 = refine(f0);
			cost_f2 = refine(f2);
			iter_f->second = std::make_pair(POSITIVE_DAVIO, cost_f0 + cost_f2);
			return cost_f0 + cost_f2;
		}
	}

	return 0;
}

std::uint32_t ArExtractManager::starting_cover(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddmanager))
		return 0u;
	if (f == Cudd_ReadOne(_ddmanager))
		return 1u;
        
	auto it = _exp_cost.find(f);
	if (it != _exp_cost.end()) {
		return it->second.second;
	}

	// Calculate f0, f1, f2
	DdNode* f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode* f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode* f2 = Cudd_bddXor(_ddmanager, f0, f1); Cudd_Ref(f2);

    // Calculate cost
    uint32_t s0 = CostFunction(f0); 
    uint32_t s1 = CostFunction(f1); 
    uint32_t s2 = CostFunction(f2);

    // Recursive calls
    uint32_t smax = std::max(std::max(s0, s1), s2);

    std::pair<exp_type, std::uint32_t> ret;
    if(s0 == smax){
        std::uint32_t n1 = starting_cover(f1);
	    std::uint32_t n2 = starting_cover(f2);
        ret = std::make_pair(NEGATIVE_DAVIO, n1 + n2);
    }
    else if(s1 == smax){
        std::uint32_t n0 = starting_cover(f0);
        std::uint32_t n2 = starting_cover(f2);
        ret = std::make_pair(POSITIVE_DAVIO, n0 + n2);
    }
    else{
        // Cudd_RecursiveDeref(_ddmanager, f2);
        std::uint32_t n0 = starting_cover(f0);
	    std::uint32_t n1 = starting_cover(f1);
        ret = std::make_pair(SHANNON, n0 + n1);
    }

	_exp_cost[f] = ret;
	return ret.second;
}

uint32_t ArExtractManager::CostFunction(DdNode* p){ 
    return CostFunctionLevel(p, _level-1);
}

uint32_t ArExtractManager::CostFunctionLevel(DdNode* f, int level){ 
	if (f == Cudd_ReadLogicZero(_ddmanager))
		return 0;
	if (f == Cudd_ReadOne(_ddmanager))
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
	DdNode* f2 = Cudd_bddXor(_ddmanager, f0, f1); Cudd_Ref(f2);

    uint32_t s0 = CostFunctionLevel(f0, level-1); 
    uint32_t s1 = CostFunctionLevel(f1, level-1); 
    uint32_t s2 = CostFunctionLevel(f2, level-1);

	return s0 + s1 + s2 - std::max(s0, std::max(s1, s2));
}

void ArExtractManager::printResult() const
{
	std::cout << "Resulting PSDKRO:" << std::endl;
	for (auto &cube : _esop)
    {
		std::cout << cube.str(_nVars) << std::endl;
	}
}

void ArExtractManager::printESOPwithOrder( int nPi, std::vector<int>& ordering) const
{
    assert(ordering.size() == _nVars);
	std::cout << "Resulting PSDKRO (with order):" << std::endl;
    for(auto &cube : _esop) 
    {
        std::string e(nPi, '-');
        for(uint32_t i = 0; i < _nVars; ++i)
          e[ordering[i]] = cube.lit(i);

        std::cout << e << std::endl; 
    }   
}

void ArExtractManager::writePLAwithOrder( int nPi, std::vector<int>& ordering, char* filename) const
{
    assert(ordering.size() == _nVars);
    assert(filename);
    std::ofstream outFile;
    outFile.open(filename, std::ios::out);
    
    outFile << ".i " << nPi << '\n';
    outFile << ".o 1\n";
    outFile << ".type esop\n";

    for(auto &cube : _esop) 
    {
        std::string e(nPi, '-');
        for(uint32_t i = 0; i < _nVars; ++i)
          e[ordering[i]] = cube.lit(i);

        outFile << e << " 1" << '\n';
    }   
    
    outFile << ".e\n" << std::endl;

    outFile.close();
}

uint32_t ArExtractManager::getNumTerms() const
{
    return _esop.size();
}

void ArExtractMain(Abc_Ntk_t* pNtk, char* filename, int fLevel, int fRefine, int fVerbose){
    Abc_Ntk_t* pNtkBdd = NULL;
    int fReorder = 1; // use reordering or not
    int fBddMaxSize = ABC_INFINITY; // the max size of BDD

	// number of variables in Ntk
	int nPi = Abc_NtkPiNum(pNtk);

	abctime clk = Abc_Clock();

	// Build BDD
    if ( Abc_NtkIsStrash(pNtk) )
        pNtkBdd = Abc_NtkCollapse( pNtk, fBddMaxSize, 0, fReorder, 0, 1, 0);
    else{
        Abc_Ntk_t* pStrNtk;
        pStrNtk = Abc_NtkStrash( pNtk, 0, 0, 0 );
        pNtkBdd = Abc_NtkCollapse( pStrNtk, fBddMaxSize, 0, fReorder, 0, 1, 0);
        Abc_NtkDelete( pStrNtk );
    }

	std::cout << "BDD construction time used: \t" <<  static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC << " sec" << std::endl;

	// get CUDD manager
    DdManager* ddmanager = (DdManager*) pNtkBdd->pManFunc;

	// get the root node of the BDD
    Abc_Obj_t* pObj = Abc_ObjFanin0(Abc_NtkPo(pNtkBdd, 0));
    DdNode* ddnode = (DdNode *) pObj->pData; 

    // number of used variables in BDD
    int nVars = Cudd_SupportSize(ddmanager, ddnode); 

    char** pVarNames = (char **)(Abc_NodeGetFaninNames(pObj))->pArray;
   
	// check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > psdkro::bitwidth) {
		std::cout << "Cannot support nVars > " << psdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

    ArExtractManager m(ddmanager, nVars, fLevel);   

    // extract algorithm
	m.init();

	clk = Abc_Clock();
    uint32_t nTerms_start = m.starting_cover(ddnode);

	double runtime_start = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
	double memory_start = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);

	std::cout << "---- Staring Cover ----" << std::endl;
	std::cout << "Time used: \t\t" <<  runtime_start << " sec" << std::endl;
	std::cout << "Memory used: \t" << memory_start << " GB" << std::endl;
	std::cout << "nTerms: \t\t" << nTerms_start << std::endl;

    if(fRefine)
    {   
        clk = Abc_Clock();
        uint32_t nTerms_refine = m.refine(ddnode);

        double runtime_refine = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
        double memory_refine = getPeakRSS( ) / (1024.0 * 1024.0 * 1024.0);

        std::cout << "---- Refinement ----" << std::endl;
        std::cout << "Time used: \t\t" <<  runtime_refine << " sec" << std::endl;
        std::cout << "Memory used: \t" << memory_refine << " GB" << std::endl;
        std::cout << "nTerms: \t\t" << nTerms_refine << std::endl;
    }   

	m.generate_psdkro(ddnode);

    // dump ordering. ordering[i] indicates the order of variable i in PIs.
    std::vector<int> ordering;
    ordering.resize(nPi);
    for(int i = 0; i < nVars; ++i)
    {
        for(int j = 0; j < nPi; ++j)
        {
            if(!strcmp(pVarNames[i], Abc_ObjName(Abc_NtkPi(pNtkBdd, j))))
            {
                ordering[i] = j;
                break;
            }
        }
    }
  
    // print ESOP and write PLA file
    if(fVerbose)
    { 
        m.printESOPwithOrder(nPi, ordering); 
    }
    if(filename)
    {
        m.writePLAwithOrder(nPi, ordering, filename);
    }

    Abc_NtkDelete(pNtkBdd);
}
