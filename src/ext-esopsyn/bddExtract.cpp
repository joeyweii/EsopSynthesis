#include "bddExtract.h"

extern "C" DdNode * extraComposeCover (DdManager* dd, DdNode * zC0, DdNode * zC1, DdNode * zC2, int TopVar);

BddExtractManager::BddExtractManager(DdManager* ddManager, DdNode* rootNode, uint32_t nVars, bool useZdd)
: _ddManager(ddManager), _rootNode(rootNode), _zddNode(nullptr), _nVars(nVars), _useZdd(useZdd) , _values(nVars, VarValue::DONTCARE)
{
    if(_useZdd)
        Cudd_zddVarsFromBddVars( _ddManager, 2 );
}

BddExtractManager::~BddExtractManager()
{ }

void BddExtractManager::extract()
{
	if (_rootNode == NULL) return;

	_exp_cost.clear();
	_esop.clear();

	bestExpansion(_rootNode);
    if(_useZdd)
        _zddNode = genPSDKROZdd(_rootNode);
    else
        genPSDKROBitStr(_rootNode);
}

DdNode* dummyGenPSDKROZdd(DdManager *m, DdNode *f){ return NULL;};
DdNode* BddExtractManager::genPSDKROZdd(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return Cudd_ReadZero(_ddManager);
	if (f == Cudd_ReadOne(_ddManager)) 
		return Cudd_ReadOne(_ddManager);

    int varIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr;
    zRes = cuddCacheLookup1Zdd(_ddManager, dummyGenPSDKROZdd, f);
    if(zRes)
		return zRes;

	// Find the best expansion by a cache lookup
	assert(_exp_cost.find(f) != _exp_cost.end());
	ExpType expansion = _exp_cost[f].first;

	// Calculate f0, f1, f2
	DdNode *f0 = Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
	DdNode *f1 = Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
	DdNode *f2 = Cudd_bddXor(_ddManager, f0, f1); 
	
	// Generate psdkro of the branches 
	if (expansion == ExpType::pD)
    {
        DdNode *zf0, *zf2;
		zf0 = genPSDKROZdd(f0);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf2 = genPSDKROZdd(f2);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _ddManager, varIdx*2, zf2, zf0);
        assert(zRes != nullptr);
        cuddDeref(zf0);
        cuddDeref(zf2);
	} 
    else if (expansion == ExpType::nD)
    {
        DdNode *zf1, *zf2;
		zf1 = genPSDKROZdd(f1);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
		zf2 = genPSDKROZdd(f2);
        assert(zf2 != nullptr);
        Cudd_Ref(zf2);
        
        zRes = cuddZddGetNode( _ddManager, varIdx*2+1, zf2, zf1);
        assert(zRes != nullptr);
        cuddDeref(zf1);
        cuddDeref(zf2);
	} 
    else 
    { 
        DdNode *zf0, *zf1;
		zf0 = genPSDKROZdd(f0);
        assert(zf0 != nullptr);
        Cudd_Ref(zf0);
		zf1 = genPSDKROZdd(f1);
        assert(zf1 != nullptr);
        Cudd_Ref(zf1);
        Cudd_Ref(Cudd_ReadZero(_ddManager));
        // zf0, zf1, Cudd_ReadZero(_ddManager) will be deref in extraComposeCover function
        zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varIdx);
        assert(zRes != nullptr);
	}

    cuddCacheInsert1( _ddManager, dummyGenPSDKROZdd, f, zRes );
    return zRes;
}

void BddExtractManager::genPSDKROBitStr(DdNode *f)
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
		genPSDKROBitStr(f0);
		_values[idx] = VarValue::POSITIVE;
		genPSDKROBitStr(f2);
	} 
    else if (expansion == ExpType::nD)
    {
		_values[idx] = VarValue::DONTCARE;
		genPSDKROBitStr(f1);
		_values[idx] = VarValue::NEGATIVE;
		genPSDKROBitStr(f2);
	} 
    else 
    { 
		_values[idx] = VarValue::NEGATIVE;
		genPSDKROBitStr(f0);
		_values[idx] = VarValue::POSITIVE;
		genPSDKROBitStr(f1);
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

void BddExtractManager::getBitStr(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
}

uint32_t BddExtractManager::getNumTerms() const
{
    if(_useZdd)
        return Cudd_CountPathsToNonZero(_zddNode); 
    else
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

    if(!fUseZdd)
        m.getBitStr(ret);

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
    if(!fUseZdd) 
        std::cout << "Number of terms: \t\t" << ESOP.size() << std::endl;

    if(fVerbose && !fUseZdd)
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
