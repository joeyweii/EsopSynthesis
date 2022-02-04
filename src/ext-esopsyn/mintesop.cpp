#include "utils.h"

void MintEsopMain(Abc_Obj_t* pNode, std::ofstream& OutFile){
    DdManager * dd = (DdManager *)pNode->pNtk->pManFunc;
    DdNode * OnSet = (DdNode *)pNode->pData;
    int nVars = Abc_ObjFaninNum(pNode);
    assert(nVars > 0 && nVars <= 31);
    char** pVarNames = (char **)(Abc_NodeGetFaninNames(pNode))->pArray;

    std::vector<int> map;
    for(int i = 0; i < nVars; i++){
        for(int j = 0; j < nVars; j++){
            if(!strcmp(pVarNames[j], Abc_ObjName(Abc_NtkPi(pNode->pNtk, i)))){
                map.push_back(j);
                break;
            }
        }
    }

    assert(map.size() == nVars);

    DdNode * bCube, * bPart;
    std::vector<std::vector<bool> > onsets;
    for(int i = 0; i < nVars; i++){
        std::vector<bool> tem;
        onsets.push_back(tem);
    }

    for(unsigned i = 0; i < (1 << nVars); i++){
        bCube = Extra_bddBitsToCube( dd, i, nVars, dd->vars, 0 );   Cudd_Ref( bCube );
        bPart = Cudd_Cofactor( dd, OnSet, bCube );                  Cudd_Ref( bPart );
        bool value = (bPart == b1);
        Cudd_RecursiveDeref( dd, bPart );
        Cudd_RecursiveDeref( dd, bCube );
        if(value){
            int bit = 1;
            for(int j = 0; j < nVars; j++){
                onsets[j].push_back((i & bit) > 0);
                bit = bit << 1;
            }
        }
    }

    OutFile << ".i " << nVars << std::endl;
    OutFile << ".o " << 1 << std::endl;
    OutFile << ".type esop" << std::endl;

    for(int i = 0; i < onsets[0].size(); i++){
        for(int j = 0; j < nVars; j++){
            OutFile << onsets[map[j]][i];
        }

        OutFile << " 1" << std::endl;
    }

    OutFile << ".e" << std::endl;

}