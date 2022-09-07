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

class IsfExtractManager
{
public:
    IsfExtractManager(DdManager* ddManager, DdNode* fRoot, DdNode* fcRoot, std::uint32_t nVars);
    void extract();
    void dumpZddDot();
    void printZddCubes();
    void printZddNumCubes();
    void writePlaFile(char* filename);
private:
    DdManager*  _ddManager;
    DdNode*     _fRoot;
    DdNode*     _fcRoot;
    DdNode*     _zRoot;
    uint32_t    _nVars;

    std::pair<DdNode*, DdNode*> expandHeuristicRecur(DdNode* f, DdNode* fc);
    std::pair<DdNode*, DdNode*> expandExactRecur(DdNode* f, DdNode* fc);
};

#endif
