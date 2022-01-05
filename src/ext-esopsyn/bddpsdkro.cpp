#include "bddpsdkro.h"

static DdManager* dd = NULL;
static int numPI = -1;
static std::unordered_map<DdNode*, PSDKRONode*> umap;

void BddPSDKRO(DdNode* pNode){
    assert(umap.find(pNode) != umap.end());
    PSDKRONode* n = umap[pNode];
    if(pNode == Cudd_Not(Cudd_ReadLogicZero(dd))){
        n->Cost = 1;
        std::string c;
        for(int i = 0; i < numPI; i++) c += "-";
        n->Esop.push_back(c);
        return;
    }
    else if(pNode == Cudd_ReadLogicZero(dd)){
        n->Cost = 0;
        return;
    }

    

    int level = Cudd_NodeReadIndex(pNode);
    DdNode* var = Cudd_bddIthVar(dd, level); 
   
    DdNode* p0 = Cudd_Cofactor(dd, pNode, Cudd_Not(var)); Cudd_Ref(p0);
    DdNode* p1 = Cudd_Cofactor(dd, pNode, var); Cudd_Ref(p1);
    DdNode* p01 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p01);

    PSDKRONode* n0;
    if(umap.find(p0) == umap.end()){
        n0 = new PSDKRONode();
        umap[p0] = n0;
        n0->Cost = -1;
    }else n0 = umap[p0];

    PSDKRONode* n1;
    if(umap.find(p1) == umap.end()){
        n1 = new PSDKRONode();
        umap[p1] = n1;
        n1->Cost = -1;
    }else n1 = umap[p1];

    PSDKRONode* n01;
    if(umap.find(p01) == umap.end()){
        n01 = new PSDKRONode();
        umap[p01] = n01;
        n01->Cost = -1;
    }else n01 = umap[p01];

    if(n0->Cost == -1) BddPSDKRO(p0);
    if(n1->Cost == -1) BddPSDKRO(p1);
    if(n01->Cost == -1) BddPSDKRO(p01);

    assert(pNode != p0 && pNode != p1 && pNode != p01);

    n->Cost = n0->Cost + n1->Cost + n01->Cost - std::max(n0->Cost, std::max(n1->Cost, n01->Cost));
    if(n->Cost == n0->Cost + n01->Cost){
        for(int i = 0; i < n0->Esop.size(); i++) n->Esop.push_back(n0->Esop[i]);
        for(int i = 0; i < n01->Esop.size(); i++){
            assert(n01->Esop[i][level] == '-');
            std::string tem = n01->Esop[i];
            tem[level] = '1';
            n->Esop.push_back(tem);
        }  
    }
    else if(n->Cost == n1->Cost + n01->Cost){
        for(int i = 0; i < n1->Esop.size(); i++) n->Esop.push_back(n1->Esop[i]);
        for(int i = 0; i < n01->Esop.size(); i++){
            assert(n01->Esop[i][level] == '-');
            std::string tem = n01->Esop[i];
            tem[level] = '0';
            n->Esop.push_back(tem);
        }  
    }
    else{
        for(int i = 0; i < n0->Esop.size(); i++){
            assert(n0->Esop[i][level] == '-');
            std::string tem = n0->Esop[i];
            tem[level] = '0';
            n->Esop.push_back(tem);
        } 
        for(int i = 0; i < n1->Esop.size(); i++){
            assert(n1->Esop[i][level] == '-');
            std::string tem = n1->Esop[i];
            tem[level] = '1';
            n->Esop.push_back(tem);
        }   
    }

    // Cudd_RecursiveDeref(dd, p0);
    // Cudd_RecursiveDeref(dd, p1);
    // Cudd_RecursiveDeref(dd, p01);
}

// TODO: add reordering
void BddPSDKROMain(Abc_Ntk_t* pNtk){
    dd = (DdManager*) pNtk->pManFunc;
    numPI = Abc_NtkPiNum(pNtk);    

    assert(Abc_NtkPoNum(pNtk) == 1);
    Abc_Obj_t* pObj = Abc_ObjFanin0(Abc_NtkPo(pNtk, 0));

    DdNode* pNode = (DdNode *) pObj->pData;
    PSDKRONode* nNode = new PSDKRONode();
    umap[pNode] = nNode;
    nNode->Cost = -1;

    BddPSDKRO(pNode);
    std::cout << "Cost: " << nNode->Cost << std::endl;
    std::cout << "Esop:" << std::endl;
    for(int i = 0; i < nNode->Esop.size(); i++)
        std::cout << nNode->Esop[i] << std::endl;

    assert(Cudd_ReadReorderings(dd) == 0);

    for(auto& n : umap){
        delete n.second;
    }

    umap.clear();
}