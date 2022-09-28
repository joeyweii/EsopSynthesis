#ifndef _BDDEXTRACT_H_
#define _BDDEXTRACT_H_

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

class BddExtractManager 
{

public:

    // Constructor and Destructor 
	BddExtractManager(DdManager* ddManager, DdNode* FRoot, std::uint32_t nVars);
	~BddExtractManager();
    
    // extract algorithm
	void extract();

    // get the final ESOP and number of terms
    void getESOP(std::vector<std::string>& ret) const;
    uint32_t getNumCubes() const;

private:

	// First pass: dicide the best expansion and calculate the cost 
	std::uint32_t fullExpand(DdNode* F);

	// Second pass: generate PSDKRO 
	void genPSDKRO(DdNode* F);

    //DdNode* genPSDKROZdd(DdNode* f);

private:
	DdManager* _ddManager;              // cudd manager
    DdNode*    _FRoot;                  // root node of function to be extracted
	uint32_t _nVars;                    // the number of variables
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
	std::vector<VarValue> _values;      // for generating psdkro
	std::unordered_map<DdNode*, std::pair<ExpType, std::uint32_t>> _hash; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::vector<cube> _esop;            // storing the resulting esop
};

#endif
