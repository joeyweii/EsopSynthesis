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

/* TODO: Local cache for #nodes */

int CostFunctionLevelRecur(DdNode* p, int level){
    if(p == Cudd_Not(Cudd_ReadLogicZero(dd)))
        return 1;
    else if(p == Cudd_ReadLogicZero(dd))
        return 0;
    
    if(level == 0) return BddNodeNum(p);

    int index = Cudd_NodeReadIndex(p);
    DdNode* var = Cudd_bddIthVar(dd, index); 

    DdNode* p0 = Cudd_Cofactor(dd, p, Cudd_Not(var)); Cudd_Ref(p0);
    DdNode* p1 = Cudd_Cofactor(dd, p, var); Cudd_Ref(p1);
    DdNode* p2 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p2);

    int size0 = CostFunctionLevelRecur(p0, level - 1);
    int size1 = CostFunctionLevelRecur(p1, level - 1);
    int size2 = CostFunctionLevelRecur(p2, level - 1);

    return size0 + size1 + size2 - std::max(size0, std::max(size1, size2));
    // return std::min(size0, std::min(size1, size2));
}

int CostFunctionLevel(DdNode* p){ 
    return CostFunctionLevelRecur(p, 7);
}

int CostFunction(DdNode* p){
    return CostFunctionLevel(p);
}

static void BddPSDKRO(DdNode* p, PSDKRONode* n, std::unordered_map<DdNode*, PSDKRONode*>& umap){
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
    DdNode* p2 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p2);

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

    PSDKRONode* n2;
    if(umap.find(p2) == umap.end()){
        n2 = new PSDKRONode();
        umap[p2] = n2;
        n2->Cost = -1;
    }else n2 = umap[p2];

    if(n0->Cost == -1) BddPSDKRO(p0, n0, umap);
    if(n1->Cost == -1) BddPSDKRO(p1, n1, umap);
    if(n2->Cost == -1) BddPSDKRO(p2, n2, umap);

    /*
    int size0 = BddNodeNum(p0);
    int size1 = BddNodeNum(p1);
    int size2 = BddNodeNum(p2);

    std::cout << "--------" << std::endl;
    std::cout << "p: " << p << " p0: " << p0 << " p1: " << p1 << " p2: " << p2 << std::endl;
    std::cout << "s0: " << size0 << " c0: " << n0->Cost << std::endl;
    std::cout << "s1: " << size1 << " c1: " << n1->Cost << std::endl;
    std::cout << "s2: " << size2 << " c2: " << n2->Cost << std::endl;
    */

    assert(p != p0 && p != p1 && p != p2);

    n->Cost = n0->Cost + n1->Cost + n2->Cost - std::max(n0->Cost, std::max(n1->Cost, n2->Cost));
    if(n->Cost == n0->Cost + n2->Cost){
        for(int i = 0; i < n0->Esop.size(); i++) n->Esop.push_back(n0->Esop[i]);
        for(int i = 0; i < n2->Esop.size(); i++){
            assert(n2->Esop[i][level] == '-');
            std::string tem = n2->Esop[i];
            tem[level] = '1';
            n->Esop.push_back(tem);
        }  
    }
    else if(n->Cost == n1->Cost + n2->Cost){
        for(int i = 0; i < n1->Esop.size(); i++) n->Esop.push_back(n1->Esop[i]);
        for(int i = 0; i < n2->Esop.size(); i++){
            assert(n2->Esop[i][level] == '-');
            std::string tem = n2->Esop[i];
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
    // Cudd_RecursiveDeref(dd, p2);
}

static void PrunedPSDKRO(DdNode* p, PSDKRONode* n, std::unordered_map<DdNode*, PSDKRONode*>& umap){
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
    DdNode* p2 = Cudd_bddXor(dd, p0, p1); Cudd_Ref(p2);

    int size0 = BddNodeNum(p0);
    int size1 = BddNodeNum(p1);
    int size2 = BddNodeNum(p2);

    int sizemax = std::max(size0, std::max(size1, size2));
    if(sizemax == size0 && p0 != Cudd_ReadLogicZero(dd)){
        PSDKRONode* n1;
        if(umap.find(p1) == umap.end()){
            n1 = new PSDKRONode();
            umap[p1] = n1;
            n1->Cost = -1;
        }else n1 = umap[p1];

        PSDKRONode* n2;
        if(umap.find(p2) == umap.end()){
            n2 = new PSDKRONode();
            umap[p2] = n2;
            n2->Cost = -1;
        }else n2 = umap[p2];

        if(n1->Cost == -1) PrunedPSDKRO(p1, n1, umap);
        if(n2->Cost == -1) PrunedPSDKRO(p2, n2, umap);

        for(int i = 0; i < n1->Esop.size(); i++) n->Esop.push_back(n1->Esop[i]);
        for(int i = 0; i < n2->Esop.size(); i++){
            assert(n2->Esop[i][level] == '-');
            std::string tem = n2->Esop[i];
            tem[level] = '0';
            n->Esop.push_back(tem);
        }  

        n->Cost = n1->Cost + n2->Cost;
    }
    else if(sizemax == size1){
        PSDKRONode* n0;
        if(umap.find(p0) == umap.end()){
            n0 = new PSDKRONode();
            umap[p0] = n0;
            n0->Cost = -1;
        }else n0 = umap[p0];

        PSDKRONode* n2;
        if(umap.find(p2) == umap.end()){
            n2 = new PSDKRONode();
            umap[p2] = n2;
            n2->Cost = -1;
        }else n2 = umap[p2];

        if(n0->Cost == -1) PrunedPSDKRO(p0, n0, umap);
        if(n2->Cost == -1) PrunedPSDKRO(p2, n2, umap);

        for(int i = 0; i < n0->Esop.size(); i++) n->Esop.push_back(n0->Esop[i]);
        for(int i = 0; i < n2->Esop.size(); i++){
            assert(n2->Esop[i][level] == '-');
            std::string tem = n2->Esop[i];
            tem[level] = '1';
            n->Esop.push_back(tem);
        } 
        n->Cost = n0->Cost + n2->Cost;
    }
    else{
        Cudd_RecursiveDeref(dd, p2);
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

        if(n0->Cost == -1) PrunedPSDKRO(p0, n0, umap);
        if(n1->Cost == -1) PrunedPSDKRO(p1, n1, umap);

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
    // Cudd_RecursiveDeref(dd, p2);
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
    else PrunedPSDKRO(pNode, nNode, umap);

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