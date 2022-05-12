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

class ARExtractManager {
public:

	ARExtractManager(DdManager*, std::uint32_t, std::uint32_t);

	void get_ordering(std::vector<uint32_t>& ordering);
	void print_esop(int);
	void write_esop_to_file(char* filename);

	// initialize data structure 
	void init();
		
	// find the starting cover
	std::uint32_t starting_cover(DdNode *);

	// refinement
	std::uint32_t refine(DdNode *);

	// generate the psdkro
	void generate_psdkro(DdNode *);
	
private:

	//  Cost Functions
    // uint32_t BddNodeNum(DdNode* p, std::unordered_set<DdNode*>& visited);
    // uint32_t Const1Path(DdNode* p, std::unordered_map<DdNode *, uint32_t>&);
    uint32_t CostFunction(DdNode*);
	uint32_t CostFunctionLevel(DdNode*, int);

private:
	DdManager* _ddmanager; // cudd manager
	uint32_t _nVars; // the number of variables
	std::vector<std::uint32_t> _vars; // for generating psdkro 
	std::vector<var_value> _values; // for generating psdkro
	std::unordered_map<DdNode *, std::pair<exp_type, std::uint32_t>> _exp_cost; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::vector<cube> _esop; // storing the resulting esop
	std::vector<std::uint32_t> _ordering; // the variable ordering
	uint32_t _level; // k level look ahead
    
};

void ARExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);

