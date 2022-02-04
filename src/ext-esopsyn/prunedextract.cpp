#include "prunedextract.h"

PrunedExtractManager::PrunedExtractManager(DdManager* _ddmanager, uint32_t nVars)
	: _ddmanager(_ddmanager), _nVars(nVars) , _values(nVars, UNUSED)
{ }

void PrunedExtractManager::extract(DdNode *f)
{
	if (f == NULL) return;

	_exp_cost.clear();
	_esop.clear();
	std::fill(_values.begin(), _values.end(), UNUSED);

	best_expansion(f);
    generate_psdkro(f);
}

void PrunedExtractManager::generate_psdkro(DdNode *f)
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
	auto idx = _ordering[Cudd_NodeReadIndex(f)];
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

std::pair<exp_type, std::uint32_t> PrunedExtractManager::best_expansion(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddmanager))
		return std::make_pair(POSITIVE_DAVIO, 0u);
	if (f == Cudd_ReadOne(_ddmanager))
		return std::make_pair(POSITIVE_DAVIO, 1u);
        
	auto it = _exp_cost.find(f);
	if (it != _exp_cost.end()) {
		return it->second;
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
        std::uint32_t n1 = best_expansion(f1).second;
	    std::uint32_t n2 = best_expansion(f2).second;
        ret = std::make_pair(NEGATIVE_DAVIO, n1 + n2);
    }
    else if(s1 == smax){
        std::uint32_t n0 = best_expansion(f0).second;
        std::uint32_t n2 = best_expansion(f2).second;
        ret = std::make_pair(POSITIVE_DAVIO, n0 + n2);
    }
    else{
        Cudd_RecursiveDeref(_ddmanager, f2);
        std::uint32_t n0 = best_expansion(f0).second;
	    std::uint32_t n1 = best_expansion(f1).second;
        ret = std::make_pair(SHANNON, n0 + n1);
    }

	_exp_cost[f] = ret;
	return ret;
}

void PrunedExtractManager::get_ordering(std::vector<uint32_t>& ordering){
	_ordering = ordering;	
}

void PrunedExtractManager::print_esop(int verbose){
	if(verbose){
		std::cout << "Resulting PSDKRO:" << std::endl;
		for (auto &cube : _esop) {
			std::cout << cube.str(_nVars) << std::endl;
		}
	}
	std::cout << "nTerms: " << _esop.size() << std::endl;
}

void PrunedExtractManager::write_esop_to_file(char* filename){
	std::ofstream file;
	file.open(filename, std::ios::out);

	if(!file.is_open()) std::cout << "Output file cannot be opened!" << std::endl;
	else{
		file << ".i " << _nVars << "\n";
		file << ".o 1\n";
		file << ".type esop\n";

		for (auto &cube : _esop) {
			file << cube.str(_nVars) << " 1\n";
		}
		file << ".e\n";
	}
}

uint32_t PrunedExtractManager::CostFunction(DdNode* p){ 
    return Cudd_CountPathsToNonZero(p);
}

/*

uint32_t PrunedExtractManager::Const1Path(DdNode* p, std::unordered_map<DdNode *, uint32_t>& nPaths){
    if(nPaths.find(p) != nPaths.end())
        return nPaths[p];

    if(p == Cudd_ReadOne(_ddmanager)){
        nPaths[p] = 1;
        return 1;
    }

    if(p == Cudd_ReadLogicZero(_ddmanager)){
        nPaths[p] = 0;
        return 0;
    }

    DdNode* t = Cudd_NotCond(Cudd_T(p), Cudd_IsComplement(p));
    uint32_t l = Const1Path(t, nPaths);
    DdNode* e = Cudd_NotCond(Cudd_E(p), Cudd_IsComplement(p));
    uint32_t r = Const1Path(e, nPaths);
    nPaths[p] = l + r;
    return l + r;
}

uint32_t PrunedExtractManager::CostFunction(DdNode* p){ 
    std::unordered_map<DdNode*, uint32_t> nPaths;
    return Const1Path(p, nPaths);
}


uint32_t PrunedExtractManager::BddNodeNum(DdNode* p, std::unordered_set<DdNode*>& visited){
    if(visited.find(p) != visited.end()) return 0;

    if(p == Cudd_ReadOne(_ddmanager) || p == Cudd_ReadLogicZero(_ddmanager)){
        visited.emplace(p);
        return 1;
    }

    DdNode* t = Cudd_NotCond(Cudd_T(p), Cudd_IsComplement(p));
    uint32_t l = BddNodeNum(t, visited);
    DdNode* e = Cudd_NotCond(Cudd_E(p), Cudd_IsComplement(p));
    uint32_t r = BddNodeNum(e, visited);
    visited.emplace(p);
    return l + r + 1;
}


uint32_t PrunedExtractManager::CostFunction(DdNode* p){ 
    std::unordered_set<DdNode*> visited;
    return BddNodeNum(p, visited);
}
*/

void PrunedExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose){
    Abc_Ntk_t* pNtkBdd = NULL;
    int fReorder = 1; // use reordering or not
    int fBddMaxSize = ABC_INFINITY; // the max size of BDD

    // get the number of variables
	int nVars = Abc_NtkPiNum(pNtk);

	// check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > bddpsdkro::bitwidth) {
		std::cout << "Cannot support nVars > " << bddpsdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

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

	// make sure that every PI is used in BDD
    assert(nVars == Cudd_SupportSize(ddmanager, ddnode)); 

	// get the ordering
	char** pVarNames = (char **)(Abc_NodeGetFaninNames(pObj))->pArray;
	std::vector<uint32_t> ordering;
    for(int i = 0; i < nVars; i++){
        for(int j = 0; j < nVars; j++){
            if(!strcmp(pVarNames[i], Abc_ObjName(Abc_NtkPi(pNtkBdd, j)))){
                ordering.push_back(j);
                break;
            }
        }
    }

    PrunedExtractManager m(ddmanager, nVars);   

	m.get_ordering(ordering);

	clk = Abc_Clock();
    m.extract(ddnode);

	double currentSize = getCurrentRSS( );
	std::cout << "PSDKRO time used: \t\t" <<  static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC << " sec" << std::endl;
	std::cout << "PSDKRO memory used: \t\t" << currentSize / (1024.0 * 1024.0) << " MB" << std::endl;
	
	m.print_esop(fVerbose);
    
	if(filename)
		m.write_esop_to_file(filename);

    Abc_NtkDelete(pNtkBdd);
}