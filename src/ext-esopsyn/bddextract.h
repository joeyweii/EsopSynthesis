#include "utils.h"

class BDDExtractManager {
public:

	BDDExtractManager(DdManager*, std::uint32_t);
	void extract(DdNode *);
	void get_ordering(std::vector<uint32_t>& ordering);
	void print_esop(int);
	void write_esop_to_file(char* filename);
private:
	
	enum var_value : std::uint8_t {
		POSITIVE, // xi = 1
		NEGATIVE, // xi = 0
		UNUSED // xi don't care
	};
	enum exp_type : std::uint8_t {
		POSITIVE_DAVIO,
		NEGATIVE_DAVIO,
		SHANNON
	};

	// Second pass: generate PSDKRO 
	void generate_psdkro(DdNode *);

	// First pass: dicide the best expansion and calculate the cost 
	std::pair<exp_type, std::uint32_t> best_expansion(DdNode *);

	//  Print the esop
	

private:
	DdManager* _ddmanager; // cudd manager
	uint32_t _nVars; // the number of variables
	std::vector<std::uint32_t> _vars; // for generating psdkro 
	std::vector<var_value> _values; // for generating psdkro
	std::unordered_map<DdNode *, std::pair<exp_type, std::uint32_t>> _exp_cost; // the mapping between 1) BDD node and 2) expansion type & cost 
	std::unordered_set<cube32, cube32_hash> _esop; // storing the resulting esop
	std::vector<std::uint32_t> _ordering; // the variable ordering
};

void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);

