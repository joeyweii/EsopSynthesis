#ifndef _ISFEXTRACT_H_
#define _ISFEXTRACT_H_

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

struct pair_hash
{
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
 
        if (hash1 != hash2)
            return hash1 ^ hash2;             
         
        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};

class IsfExtractManager
{
public:
    IsfExtractManager(DdManager* ddManager, DdNode* FRoot, DdNode* CRoot, int nVars, int naive);
    void extract();
    int getNumCubes() const;
    void printESOP() const;
    void writeESOPIntoPla(char* filename);

private:
    DdManager*  _ddManager;
    DdNode*     _FRoot;
    DdNode*     _CRoot;
    int    _nVars;
    int    _naive;
	std::vector<int> _vars;   // for generating psdkro 
	std::vector<VarValue> _values;      // for generating psdkro
	std::vector<cube> _esop;            // storing the resulting esop
    std::unordered_map
    <
        std::pair<DdNode*, DdNode*>,                        // F, C
        std::tuple<DdNode*, DdNode*, ExpType, int>,         // F', F2, exp, cost 
        pair_hash                                           // hash for pair
    > _hash;

    std::pair<DdNode*, int> firstPass(DdNode* F, DdNode* C);
    void secondPass(DdNode* F, DdNode* C);
    std::pair<DdNode*, int> firstPassNaive(DdNode* F, DdNode* C);
    void secondPassNaive(DdNode* F, DdNode* C);
};

#endif
