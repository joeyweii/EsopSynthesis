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
	BDDExtractManager(DdManager*, std::uint32_t);
	void extract(DdNode *);
	void printResult() const;
    void printESOPwithOrder(int nPi, std::vector<int>& ordering) const;
    void writePLAwithOrder(int nPi, std::vector<int>& ordering, char* filename) const;
    uint32_t getNumTerms() const;

private:

	// First pass: dicide the best expansion and calculate the cost 
	std::pair<exp_type, std::uint32_t> best_expansion(DdNode *);

	// Second pass: generate PSDKRO 
	void generate_psdkro(DdNode *);

private:
	DdManager* _ddmanager;              // cudd manager
	uint32_t _nVars;                    // the number of variables
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
	std::vector<var_value> _values;     // for generating psdkro
	std::unordered_map<DdNode *, std::pair<exp_type, std::uint32_t>> _exp_cost; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::vector<cube> _esop;            // storing the resulting esop
};
