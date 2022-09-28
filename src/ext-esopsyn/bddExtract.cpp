#include "bddExtract.h"

BddExtractManager::BddExtractManager(DdManager* ddManager, DdNode* FRoot, int nVars)
: _ddManager(ddManager), _FRoot(FRoot), _nVars(nVars), _values(nVars, VarValue::DONTCARE)
{}

BddExtractManager::~BddExtractManager()
{}

void BddExtractManager::extract()
{
	if (_FRoot == NULL) return;

	_hash.clear();
	_esop.clear();

	fullExpand(_FRoot);
    genPSDKRO(_FRoot);
}


void BddExtractManager::genPSDKRO(DdNode *F)
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

	// Find the best expansion by a cache lookup
    auto it = _hash.find(F);
	assert(it != _hash.end());
	ExpType expansion = it->second.first;

	// Determine the top-most variable
	auto varIdx = Cudd_NodeReadIndex(F);
	_vars.push_back(varIdx);

	// Calculate f0, f1, f2
    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); 
	
	// Generate psdkro of the branches 
	if (expansion == ExpType::pD)
    {
		_values[varIdx] = VarValue::DONTCARE;
		genPSDKRO(F0);
		_values[varIdx] = VarValue::POSITIVE;
		genPSDKRO(F2);
	} 
    else if (expansion == ExpType::nD)
    {
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

	Cudd_RecursiveDeref(_ddManager, F2);
	_vars.pop_back();
	_values[varIdx] = VarValue::DONTCARE;
}

int BddExtractManager::fullExpand(DdNode *F)
{
	if (F == Cudd_ReadLogicZero(_ddManager))
		return 0u;
	if (F == Cudd_ReadOne(_ddManager))
		return 1u;
		
	auto it = _hash.find(F);
	if (it != _hash.end())
		return it->second.second;

    DdNode *F0, *F1, *F2;
	F0 = Cudd_NotCond(Cudd_E(F), Cudd_IsComplement(F));
	F1 = Cudd_NotCond(Cudd_T(F), Cudd_IsComplement(F));
	F2 = Cudd_bddXor(_ddManager, F0, F1); Cudd_Ref(F2);

    int cost0, cost1, cost2;
	cost0 = fullExpand(F0);
	cost1 = fullExpand(F1);
	cost2 = fullExpand(F2);

	int costmax = std::max(std::max(cost0, cost1), cost2);

	std::pair<ExpType, int> ret;
	if (costmax == cost0) 
		ret = std::make_pair(ExpType::nD, cost1 + cost2);
	else if (costmax == cost1)
		ret = std::make_pair(ExpType::pD, cost0 + cost2);
	else
		ret = std::make_pair(ExpType::Sh, cost0 + cost1);
	
	_hash[F] = ret;
	return ret.second;
}

void BddExtractManager::getESOP(std::vector<std::string>& ret) const
{
    for(auto &cube : _esop) 
        ret.push_back(cube.str(_nVars));
}

int BddExtractManager::getNumCubes() const
{
    return _esop.size();
}

/* ZDD implicit representation
DdNode* dummyGenPSDKROZdd(DdManager *m, DdNode *f){ return NULL;};
DdNode* BddExtractManager::genPSDKROZdd(DdNode *f)
{
	// Reach constant 0/1
	if (f == Cudd_ReadLogicZero(_ddManager))
		return Cudd_ReadZero(_ddManager);
	if (f == Cudd_ReadOne(_ddManager)) 
		return Cudd_ReadOne(_ddManager);

    int varvarIdx = Cudd_NodeReadIndex(f);
    DdNode *zRes = nullptr;
    zRes = cuddCacheLookup1Zdd(_ddManager, dummyGenPSDKROZdd, f);
    if(zRes)
		return zRes;

	// Find the best expansion by a cache lookup
	assert(_hash.find(f) != _hash.end());
	ExpType expansion = _hash[f].first;

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
        
        zRes = cuddZddGetNode( _ddManager, varvarIdx*2, zf2, zf0);
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
        
        zRes = cuddZddGetNode( _ddManager, varvarIdx*2+1, zf2, zf1);
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
        zRes = extraComposeCover( _ddManager, zf0, zf1, Cudd_ReadZero(_ddManager), varvarIdx);
        assert(zRes != nullptr);
	}

    cuddCacheInsert1( _ddManager, dummyGenPSDKROZdd, f, zRes );
    return zRes;
}
*/

// extract ESOP using BddExtract algorithm and store the resulting ESOP into ret 
void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ret)
{   
    int fReorder = 1;               // Use reordering or not
    int fBddMaxSize = ABC_INFINITY; // The maximum #node in BDD

    int nVars = Abc_NtkPiNum(pNtk);

    DdManager* ddManager = (DdManager*) Abc_NtkBuildGlobalBdds(pNtk, fBddMaxSize, 1, fReorder, 0, 0);

    Abc_Obj_t* pPo = Abc_NtkPo(pNtk, 0);
    DdNode* ddroot = (DdNode*) Abc_ObjGlobalBdd(pPo);
   
	// Check the numPI is smaller than the bitwidth of a cube in psdkro
	if (nVars > psdkro::bitwidth)
    {
		std::cout << "Cannot support nVars > " << psdkro::bitwidth << " cases. Please modify the bitwidth." << std::endl;
		return;
	}

    BddExtractManager m(ddManager, ddroot, nVars); 

    m.extract();	

    m.getESOP(ret);

    Abc_NtkFreeGlobalBdds( pNtk, 0);
    Extra_StopManager(ddManager);
}


void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    BddExtractSingleOutput(pNtk, ESOP);

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
