#ifndef _AREXTRACT_H_
#define _AREXTRACT_H_

#include "base/main/main.h"
#ifdef ABC_USE_CUDD
#include "bdd/extrab/extraBdd.h"
#endif
#include "utils.h"
#include "memMeasure.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace psdkro;

class ArExtractManager
{
public:

    // Constructor and Destructor
	ArExtractManager
    (
        DdManager *ddManager, 
        std::uint32_t level, 
        std::uint32_t bound,
        bool refine, 
        DdNode* FRoot, 
        std::uint32_t nVars
    );
    ~ArExtractManager() {}

    void extract();

    void getESOP(std::vector<std::string> &ret) const;
    uint32_t getNumTerms() const;
private:
	// Find the starting cover
	std::uint32_t partialExpand(DdNode *F);
	std::uint32_t fullExpand(DdNode *F);

	// Refinement
	std::uint32_t refine(DdNode *F);

	// Generate the psdkro
	void genPSDKRO(DdNode *F);

	//  Cost Functions
    uint32_t CostEstimate(DdNode* F);
	uint32_t CostLookAhead(DdNode* F, int level);

private:
	DdManager* _ddManager;              // cudd manager
    DdNode* _FRoot;                     // root node of function to be extracted 
	uint32_t _level;                    // k level cost look ahead
    uint32_t _bound;                    // the decision bound (BDD size) of full/partial expansion
	uint32_t _nVars;                    // the number of variables
    bool     _refine;                   // conduct refinement or not
	std::vector<VarValue> _values;      // for generating psdkro
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
	std::unordered_map
    <
        DdNode *,                                   // function
        std::tuple<ExpType, std::uint32_t, bool>    // expansion, cost, refined
    > _hash; 
	std::vector<cube> _esop;            // storing the resulting esop
};

#endif
