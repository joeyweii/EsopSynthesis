#include "bddExtract.h"

extern "C" DdNode * extraComposeCover (DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar);

BddExtractManager::BddExtractManager(DdManager* ddManager, DdNode* rootNode, uint32_t nVars, bool useZdd)
: _ddManager(ddManager), _zddManager(nullptr), _rootNode(rootNode), _nVars(nVars), _useZdd(useZdd) , _values(nVars, VarValue::DONTCARE)
{
    if(_useZdd)
    {    
        _zddManager = Cudd_Init( _nVars, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
        Cudd_ShuffleHeap(_zddManager, _ddManager->invperm);
        Cudd_zddVarsFromBddVars( _zddManager, 2 );
        // Dump permutation (ordering) of bdd manager into zdd manager
        Cudd_AutodynDisable(_zddManager);
        for(int i = 0; i < _nVars; ++i)
            std::cout << Cudd_ReadPerm(_ddManager, i) << std::endl;
        std::cout << "---------" << std::endl;
        for(int i = 0; i < 2*_nVars; ++i)
            std::cout << Cudd_ReadPermZdd(_zddManager, i) << std::endl;
        /*
        int *permList = new int(2*_nVars);
        for(int i = 0; i < _nVars; ++i)
        {
            //permList[Cudd_ReadPerm(_ddManager, i)*2] = i*2;
            //permList[Cudd_ReadPerm(_ddManager, i)*2+1] = i*2+1;
            permList[i*2] = Cudd_ReadPerm(_ddManager, i)*2;
            permList[i*2+1] = Cudd_ReadPerm(_ddManager, i)*2+1;
        }
        Cudd_ShuffleHeap(_zddManager, permList);
        delete [] permList;
        */
    }    
}

BddExtractManager::~BddExtractManager()
{ }

void BddExtractManager::extract()
{
	if (_rootNode == NULL) return;

	_exp_cost.clear();
	_esop.clear();

    Cudd_bddPrintCover(_ddManager, _rootNode, _rootNode);
	bestExpansion(_rootNode);
    if(_useZdd)
    {
        DdNode* zddRootNode = generatePSDKROZdd(_rootNode);
        Cudd_zddPrintCover(_zddManager, zddRootNode);
    }
    else
        generatePSDKRO(_rootNode);
    
}

DdNode* BddExtractManager::generatePSDKROZdd(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return Cudd_ReadZero(_zddManager);
	if (f == Cudd_ReadOne(_ddManager)) 
		return Cudd_ReadOne(_zddManager);

    //int varIdx = Cudd_ReadPerm(_ddManager, Cudd_NodeReadIndex(f)); 
    int varIdx = Cudd_NodeReadIndex(f);

	// Find the best expansion by a cache lookup
	assert(_exp_cost.find(f) != _exp_cost.end());
	ExpType expansion = _exp_cost[f].first;

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); 
	
    DdNode *zRes = nullptr;
	// Generate psdkro of the branches 
	if (expansion == ExpType::pD)
    {
        DdNode *zf0, *zf2;
		zf0 = generatePSDKROZdd(f0);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf2 = generatePSDKROZdd(f2);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _zddManager, varIdx*2, zf2, zf0);
        assert(zRes != nullptr);
        cuddDeref(zf0);
        cuddDeref(zf2);
	} 
    else if (expansion == ExpType::nD)
    {
        DdNode *zf1, *zf2;
		zf1 = generatePSDKROZdd(f1);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
		zf2 = generatePSDKROZdd(f2);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _zddManager, varIdx*2+1, zf2, zf1);
        assert(zRes != nullptr);
        cuddDeref(zf1);
        cuddDeref(zf2);
	} 
    else 
    { 
        DdNode *zf0, *zf1;
		zf0 = generatePSDKROZdd(f0);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf1 = generatePSDKROZdd(f1);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
        Cudd_Ref(Cudd_ReadZero(_zddManager));
        zRes = extraComposeCover( _zddManager, zf0, zf1, Cudd_ReadZero(_zddManager), varIdx);
        assert(zRes != nullptr);
	}

    return zRes;
}

void BddExtractManager::generatePSDKRO(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return;
	if (f == Cudd_ReadOne(_ddManager)) 
    {
		cube c; 
		for (auto var : _vars) {
			if(_values[var] != VarValue::DONTCARE) c._iscare.set(var);
			if(_values[var] == VarValue::POSITIVE) c._polarity.set(var);
		}

		_esop.push_back(c);
		return;
	}

	// Find the best expansion by a cache lookup
	assert(_exp_cost.find(f) != _exp_cost.end());
	ExpType expansion = _exp_cost[f].first;

	// Determine the top-most variable
	auto idx = Cudd_NodeReadIndex(f);
	_vars.push_back(idx);

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); 
	
	// Generate psdkro of the branches 
	if (expansion == ExpType::pD)
    {
		_values[idx] = VarValue::DONTCARE;
		generatePSDKRO(f0);
		_values[idx] = VarValue::POSITIVE;
		generatePSDKRO(f2);
	} 
    else if (expansion == ExpType::nD)
    {
		_values[idx] = VarValue::DONTCARE;
		generatePSDKRO(f1);
		_values[idx] = VarValue::NEGATIVE;
		generatePSDKRO(f2);
	} 
    else 
    { 
		_values[idx] = VarValue::NEGATIVE;
		generatePSDKRO(f0);
		_values[idx] = VarValue::POSITIVE;
		generatePSDKRO(f1);
	}

	Cudd_RecursiveDeref(_ddManager, f2);
	_vars.pop_back();
	_values[idx] = VarValue::DONTCARE;
}

std::pair<ExpType, std::uint32_t> BddExtractManager::bestExpansion(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return std::make_pair(ExpType::pD, 0u);
	if (f == Cudd_ReadOne(_ddManager))
		return std::make_pair(ExpType::pD, 1u);
		
	auto it = _exp_cost.find(f);
	if (it != _exp_cost.end())
		return it->second;

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); Cudd_Ref(f2);

	// Recusive calls on f0, f1, f2
	std::uint32_t c0 = bestExpansion(f0).second;
	std::uint32_t c1 = bestExpansion(f1).second;
	std::uint32_t c2 = bestExpansion(f2).second;

	// Choose the least costly expansion 
	std::uint32_t cmax = std::max(std::max(c0, c1), c2);

	std::pair<ExpType, std::uint32_t> ret;
	if (cmax == c0) 
		ret = std::make_pair(ExpType::nD, c1 + c2);
	else if (cmax == c1)
		ret = std::make_pair(ExpType::pD, c0 + c2);
	else
		ret = std::make_pair(ExpType::S, c0 + c1);
	
	_exp_cost[f] = ret;
	return ret;
}

void BddExtractManager::getESOP(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
}

uint32_t BddExtractManager::getNumTerms() const
{
    return _esop.size();
}

// extract ESOP using BddExtract algorithm and store the resulting ESOP into ret 
void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ret, bool fUseZdd)
{   
    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

	// Number of variables(Pis) in pNtk
    int nVars = Abc_NtkPiNum(pNtk);

    DdManager* ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);

    Abc_Obj_t* pPo = Abc_NtkPo(pNtk, 0);
    DdNode* ddroot = (DdNode*) Abc_ObjGlobalBdd(pPo);
   
	// Check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > psdkro::bitwidth) {
		std::cout << "Cannot support nVars > " << psdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

    BddExtractManager m(ddManager, ddroot, nVars, fUseZdd); 

    // Extract
    m.extract();	

    m.getESOP(ret);

    // Delete global BDD
    Abc_NtkFreeGlobalBdds( pNtk, 0);
    Extra_StopManager(ddManager);
}


void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose, bool fUseZdd)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    BddExtractSingleOutput(pNtk, ESOP, fUseZdd);

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
