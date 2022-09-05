#include "base/main/main.h"
#ifdef ABC_USE_CUDD
#include "bdd/extrab/extraBdd.h"
#endif
#include "utils.h"
#include "memMeasure.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace psdkro;

class IsfExtractManager
{
public:
    IsfExtractManager(DdManager* ddManager, DdNode* fRoot, DdNode* fcRoot, std::uint32_t nVars);
    void extract();
    void getESOP(std::vector<std::string>& ret) const;
private:
    DdManager*  _ddManager;
    DdNode*     _fRoot;
    DdNode*     _fcRoot;
    uint32_t    _nVars;
	std::vector<VarValue> _values;     // for generating psdkro
	std::vector<std::uint32_t> _vars;   // for generating psdkro 
    std::unordered_map<DdNode*, std::pair<ExpType, std::uint32_t>> _exp_cost;
	std::vector<cube> _esop;            // storing the resulting esop

    std::pair<DdNode*, DdNode*> bestExpansion(DdNode* f, DdNode* fc);
    void generatePSDKRO(DdNode* f);
};
