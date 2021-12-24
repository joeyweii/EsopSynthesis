#include "bddpsdkro.h"

#define DEBUG 1

static DdManager* dd = NULL;
int numPI = -1;

void BddPSDKRO(DdNode* pNode, int level, int& Cost, std::vector<std::string>& Esop){
    if(pNode == Cudd_ReadOne(dd)){
        Cost = 1;
        std::string c;
        for(int i = 0; i < numPI; i++) c += "-";
        Esop.push_back(c);
        return;
    }
    else if(pNode == Cudd_ReadLogicZero(dd)){
        Cost = 0;
        return;
    }

    assert(level != numPI);

    DdNode* var = Cudd_bddIthVar(dd, level); // No need to ref

    DdNode* p0 = Cudd_Cofactor(dd, pNode, Cudd_Not(var)); Cudd_Ref(p0);
    DdNode* p1 = Cudd_Cofactor(dd, pNode, var); Cudd_Ref(p1);
    DdNode* p01 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p01);

    std::vector<std::string> Esop0;
    std::vector<std::string> Esop1;
    std::vector<std::string> Esop01;
    int Cost0;
    int Cost1;
    int Cost01;

    BddPSDKRO(p0, level+1, Cost0, Esop0);
    Cudd_RecursiveDeref(dd, p0);
    BddPSDKRO(p1, level+1, Cost1, Esop1);
    Cudd_RecursiveDeref(dd, p1);
    BddPSDKRO(p01, level+1, Cost01, Esop01);
    Cudd_RecursiveDeref(dd, p01);

    Cost = Cost0 + Cost1 + Cost01 - std::max(Cost0, std::max(Cost1, Cost01));
    if(Cost == Cost0 + Cost01){
        for(int i = 0; i < Esop0.size(); i++) Esop.push_back(Esop0[i]);
        for(int i = 0; i < Esop01.size(); i++){
            assert(Esop01[i][level] == '-');
            Esop01[i][level] = '1';
            Esop.push_back(Esop01[i]);
        }  
    }
    else if(Cost == Cost1 + Cost01){
        for(int i = 0; i < Esop1.size(); i++) Esop.push_back(Esop1[i]);
        for(int i = 0; i < Esop01.size(); i++){
            assert(Esop01[i][level] == '-');
            Esop01[i][level] = '0';
            Esop.push_back(Esop01[i]);
        }  
    }
    else{
        for(int i = 0; i < Esop0.size(); i++){
            assert(Esop0[i][level] == '-');
            Esop0[i][level] = '0';
           
            Esop.push_back(Esop0[i]);
        } 
        for(int i = 0; i < Esop1.size(); i++){
            assert(Esop1[i][level] == '-');
            Esop1[i][level] = '1';
            Esop.push_back(Esop1[i]);
        }   
    }
}

// TODO: add reordering
void BddPSDKROMain(Abc_Ntk_t* pNtk){
    dd = (DdManager*) pNtk->pManFunc;
    numPI = Abc_NtkPiNum(pNtk);

    assert(Abc_NtkPoNum(pNtk) == 1);
    Abc_Obj_t* pObj = Abc_ObjFanin0(Abc_NtkPo(pNtk, 0));
    DdNode* pNode = (DdNode *) pObj->pData;
    std::vector<std::string> Esop;
    int Cost;

    BddPSDKRO(pNode, 0, Cost, Esop);
    std::cout << "Cost: " << Cost << std::endl;
    std::cout << "Esop:" << std::endl;
    for(int i = 0; i < Esop.size(); i++)
        std::cout << Esop[i] << std::endl;
}