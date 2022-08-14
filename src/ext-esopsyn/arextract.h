#ifndef _AREXTRACT_H_
#define _AREXTRACT_H_

#include "base/main/main.h"
#ifdef ABC_USE_CUDD
#include "bdd/extrab/extraBdd.h"
#endif
#include "utils.h"
#include "memory_measure.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace psdkro;

class ArExtractManager {
public:

    // Constructor and Destructor
	ArExtractManager(DdManager *ddManager, std::uint32_t level, bool refine, DdNode* _rootNode, std::uint32_t nVars);
    ~ArExtractManager() {}

    void extract();

    void getESOP(std::vector<std::string> &ret) const;
    uint32_t getNumTerms() const;
private:
	// Find the starting cover
	std::uint32_t startingCover(DdNode *f);

	// Refinement
	std::uint32_t refine(DdNode *f);

	// Generate the psdkro
	void generatePSDKRO(DdNode *f);

	//  Cost Functions
    // uint32_t BddNodeNum(DdNode* p, std::unordered_set<DdNode*>& visited);
    // uint32_t Const1Path(DdNode* p, std::unordered_map<DdNode *, uint32_t>&);
    uint32_t CostFunction(DdNode* f);
	uint32_t CostFunctionLevel(DdNode* f, int level);

private:
	DdManager* _ddManager;              // cudd manager
    DdNode* _rootNode;                  // root node of function to be extracted 
	uint32_t _level;                    // k level cost look ahead
	uint32_t _nVars;                    // the number of variables
    bool     _refine;                   // conduct refinement or not
	std::vector<VarValue> _values;     // for generating psdkro
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
	std::unordered_map<DdNode *, std::pair<ExpType, std::uint32_t>> _exp_cost; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::vector<cube> _esop;            // storing the resulting esop
    
};

#endif
