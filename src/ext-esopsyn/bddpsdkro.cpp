#include "bddpsdkro.h"

static DdManager* dd = NULL;
static int numPI = -1;


/* TODO: Local cache for #nodes */
void BddNodeNumRecur(DdNode* p, int& cnt, std::unordered_set<DdNode*>& visited){
    if(visited.find(p) != visited.end()) return;
    if(p == Cudd_Not(Cudd_ReadLogicZero(dd)) || p == Cudd_ReadLogicZero(dd)){
        cnt += 1;
        visited.emplace(p);
        return;
    }

    DdNode* t = Cudd_T(p);
    BddNodeNumRecur(t, cnt, visited);
    DdNode* e = Cudd_E(p);
    BddNodeNumRecur(e, cnt, visited);
    cnt += 1;
    visited.emplace(p);
    return;
}

int BddNodeNum(DdNode* p){
    std::unordered_set<DdNode*> visited;
    int cnt = 0;

    BddNodeNumRecur(p, cnt, visited);
    return cnt;
}

static void BddPSDKRO(DdNode* p, PSDKRONode* n, std::unordered_map<DdNode*, PSDKRONode*> umap){
    if(p == Cudd_Not(Cudd_ReadLogicZero(dd))){
        n->Cost = 1;
        std::string c;
        for(int i = 0; i < numPI; i++) c += "-";
        n->Esop.push_back(c);
        return;
    }
    else if(p == Cudd_ReadLogicZero(dd)){
        n->Cost = 0;
        return;
    }

    int level = Cudd_NodeReadIndex(p);
    DdNode* var = Cudd_bddIthVar(dd, level); 
   
    DdNode* p0 = Cudd_Cofactor(dd, p, Cudd_Not(var)); Cudd_Ref(p0);
    DdNode* p1 = Cudd_Cofactor(dd, p, var); Cudd_Ref(p1);
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

    if(n0->Cost == -1) BddPSDKRO(p0, n0, umap);
    if(n1->Cost == -1) BddPSDKRO(p1, n1, umap);
    if(n01->Cost == -1) BddPSDKRO(p01, n01, umap);

    assert(p != p0 && p != p1 && p != p01);

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

static void BddPSDKROSelect(DdNode* p, PSDKRONode* n, std::unordered_map<DdNode*, PSDKRONode*>& umap){
    if(p == Cudd_Not(Cudd_ReadLogicZero(dd))){
        n->Cost = 1;
        std::string c;
        for(int i = 0; i < numPI; i++) c += "-";
        n->Esop.push_back(c);
        return;
    }
    else if(p == Cudd_ReadLogicZero(dd)){
        n->Cost = 0;
        return;
    }

    int level = Cudd_NodeReadIndex(p);
    DdNode* var = Cudd_bddIthVar(dd, level); 
   
    DdNode* p0 = Cudd_Cofactor(dd, p, Cudd_Not(var)); Cudd_Ref(p0);
    DdNode* p1 = Cudd_Cofactor(dd, p, var); Cudd_Ref(p1);
    DdNode* p01 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p01);

    int size0 = BddNodeNum(p0);
    int size1 = BddNodeNum(p1);
    int size01 = BddNodeNum(p01);

    int sizemax = std::max(size0, std::max(size1, size01));
    if(sizemax == size0 && p0 != Cudd_ReadLogicZero(dd)){
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

        if(n1->Cost == -1) BddPSDKROSelect(p1, n1, umap);
        if(n01->Cost == -1) BddPSDKROSelect(p01, n01, umap);

        for(int i = 0; i < n1->Esop.size(); i++) n->Esop.push_back(n1->Esop[i]);
        for(int i = 0; i < n01->Esop.size(); i++){
            assert(n01->Esop[i][level] == '-');
            std::string tem = n01->Esop[i];
            tem[level] = '0';
            n->Esop.push_back(tem);
        }  

        n->Cost = n1->Cost + n01->Cost;
    }
    else if(sizemax == size1){
        PSDKRONode* n0;
        if(umap.find(p0) == umap.end()){
            n0 = new PSDKRONode();
            umap[p0] = n0;
            n0->Cost = -1;
        }else n0 = umap[p0];

        PSDKRONode* n01;
        if(umap.find(p01) == umap.end()){
            n01 = new PSDKRONode();
            umap[p01] = n01;
            n01->Cost = -1;
        }else n01 = umap[p01];

        if(n0->Cost == -1) BddPSDKROSelect(p0, n0, umap);
        if(n01->Cost == -1) BddPSDKROSelect(p01, n01, umap);

        for(int i = 0; i < n0->Esop.size(); i++) n->Esop.push_back(n0->Esop[i]);
        for(int i = 0; i < n01->Esop.size(); i++){
            assert(n01->Esop[i][level] == '-');
            std::string tem = n01->Esop[i];
            tem[level] = '1';
            n->Esop.push_back(tem);
        } 
        n->Cost = n0->Cost + n01->Cost;
    }
    else{
        Cudd_RecursiveDeref(dd, p01);
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

        if(n0->Cost == -1) BddPSDKROSelect(p0, n0, umap);
        if(n1->Cost == -1) BddPSDKROSelect(p1, n1, umap);

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
        n->Cost = n1->Cost + n0->Cost;
    }

    // Cudd_RecursiveDeref(dd, p0);
    // Cudd_RecursiveDeref(dd, p1);
    // Cudd_RecursiveDeref(dd, p01);
}

// TODO: add reordering
void BddPSDKROMain(Abc_Ntk_t* pNtk, char* pFileNameOut, int type){
    dd = (DdManager*) pNtk->pManFunc;
    numPI = Abc_NtkPiNum(pNtk);    

    assert(Abc_NtkPoNum(pNtk) == 1);
    Abc_Obj_t* pObj = Abc_ObjFanin0(Abc_NtkPo(pNtk, 0));

    std::unordered_map<DdNode*, PSDKRONode*> umap;

    

    DdNode* pNode = (DdNode *) pObj->pData;
    PSDKRONode* nNode = new PSDKRONode();
    umap[pNode] = nNode;
    nNode->Cost = -1;

    if(type == 0) BddPSDKRO(pNode, nNode, umap);
    else BddPSDKROSelect(pNode, nNode, umap);

    assert(Cudd_ReadReorderings(dd) == 0);

    std::cout << "#Terms: " << nNode->Cost << std::endl;
    
    /*
    std::cout << "Esop:" << std::endl;
    for(int i = 0; i < nNode->Esop.size(); i++)
        std::cout << nNode->Esop[i] << std::endl;
    */

    if(pFileNameOut != NULL){
        std::ofstream File;
        File.open(pFileNameOut, std::ios::out);

        File << ".i " << numPI << std::endl;
        File << ".o 1" << std::endl;
        File << ".type esop" << std::endl;
        for(int i = 0; i < nNode->Esop.size(); i++)
            File << nNode->Esop[i] << " 1" << std::endl;
        File << ".e" << std::endl;
    }

    for(auto& n : umap){
        delete n.second;
    }

    umap.clear();
}