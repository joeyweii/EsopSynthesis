#include "bddExtract.h"

BddExtractManager::BddExtractManager(DdManager* ddManager, DdNode* rootNode, uint32_t nVars)
: _ddManager(ddManager), _rootNode(rootNode), _nVars(nVars) , _values(nVars, VarValue::DONTCARE)
{ }

BddExtractManager::~BddExtractManager()
{ }

void BddExtractManager::extract()
{
	if (_rootNode == NULL) return;

	_exp_cost.clear();
	_esop.clear();

	bestExpansion(_rootNode);
    generatePSDKRO(_rootNode);
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
void BddExtractSingleOutput(Abc_Ntk_t* pNtk, std::vector<std::string>& ret)
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

    BddExtractManager m(ddManager, ddroot, nVars); 

    // Extract
    m.extract();	

    m.getESOP(ret);

    // Delete global BDD
    Abc_NtkFreeGlobalBdds( pNtk, 0);
    Cudd_Quit(ddManager);
}


void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose)
{
    std::vector<std::string> ESOP;

    abctime clk = Abc_Clock();

    BddExtractSingleOutput(pNtk, ESOP);

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
