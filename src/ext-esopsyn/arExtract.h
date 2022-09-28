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
        int level, 
        int bound,
        bool refine, 
        DdNode* FRoot, 
        int nVars
    );
    ~ArExtractManager() {}

    void extract();

    void getESOP(std::vector<std::string> &ret) const;
    int getNumTerms() const;
private:
	// Find the starting cover
	int partialExpand(DdNode *F);
	int fullExpand(DdNode *F);

	// Refinement
	int refine(DdNode *F);

	// Generate the psdkro
	void genPSDKRO(DdNode *F);

	//  Cost Functions
    int CostEstimate(DdNode* F);
	int CostLookAhead(DdNode* F, int level);

private:
	DdManager* _ddManager;              // cudd manager
    DdNode* _FRoot;                     // root node of function to be extracted 
	int _level;                    // k level cost look ahead
    int _bound;                    // the decision bound (BDD size) of full/partial expansion
	int _nVars;                    // the number of variables
    bool     _refine;                   // conduct refinement or not
	std::vector<VarValue> _values;      // for generating psdkro
	std::vector<int> _vars;   // for generating psdkro 
	std::unordered_map
    <
        DdNode *,                                   // function
        std::tuple<ExpType, int, bool>    // expansion, cost, refined
    > _hash; 
	std::vector<cube> _esop;            // storing the resulting esop
};

#endif
