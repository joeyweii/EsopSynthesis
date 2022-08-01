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

class BDDExtractManager {

public:
    // initialize
	BDDExtractManager(DdManager*, DdNode*, std::uint32_t);
    
    // extract algorithm
	void extract();

    // get the final ESOP and number of terms
    void getESOP(int nPi, std::vector<int>& ordering, std::vector<std::string>& ret) const;
    uint32_t getNumTerms() const;

private:

	// First pass: dicide the best expansion and calculate the cost 
	std::pair<exp_type, std::uint32_t> best_expansion(DdNode *);

	// Second pass: generate PSDKRO 
	void generate_psdkro(DdNode *);

private:
	DdManager* _ddmanager;              // cudd manager
    DdNode*    _ddroot;                 // dd root node
	uint32_t _nVars;                    // the number of variables
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
	std::vector<var_value> _values;     // for generating psdkro
	std::unordered_map<DdNode *, std::pair<exp_type, std::uint32_t>> _exp_cost; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::vector<cube> _esop;            // storing the resulting esop
};
