#include"scalpsdkro.h"

static DdManager* dd = NULL;
static int numPI = -1;
static std::unordered_map<DdNode*, PSDKRONode*> umap;
static std::vector<bool> CofList;

static void BddPSDKRO(DdNode* p, PSDKRONode* n){
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

    if(n0->Cost == -1) BddPSDKRO(p0, n0);
    if(n1->Cost == -1) BddPSDKRO(p1, n1);
    if(n01->Cost == -1) BddPSDKRO(p01, n01);

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

static void ScalPSDKRO(Abc_Ntk_t* pNtk, int numCof, int& Cost, std::vector<std::string>& Esop){
    if(numCof == 0){
        Abc_Ntk_t* pNtkBdd = Abc_NtkCollapse( pNtk, 1000000000, 0, 0, 0, 0, 0);

        dd = (DdManager*) pNtkBdd->pManFunc;

        Abc_Obj_t* pObj = Abc_ObjFanin0(Abc_NtkPo(pNtkBdd, 0));

        DdNode* pNode = (DdNode *) pObj->pData;
        PSDKRONode* nNode = new PSDKRONode();
        nNode->Cost = -1;
        BddPSDKRO(pNode, nNode);

        assert(Cudd_ReadReorderings(dd) == 0);

        Cost = nNode->Cost;
        Esop = nNode->Esop;

        for(auto& n : umap){
            delete n.second; 
        }
        umap.clear();

        Abc_NtkDelete(pNtkBdd);
        return;
    }
    else{
        int level = -1;
        int minSize = INT_MAX;
        for(int i = 0; i < numPI; i++){
            if(CofList[i]) continue;
            Abc_Ntk_t* p0 = AIGCOF(pNtk, i, 0);
            Abc_Ntk_t* p1 = AIGCOF(pNtk, i, 1);
            int size0 = Abc_NtkNodeNum(p0);
            int size1 = Abc_NtkNodeNum(p1);
            if(size0 + size1 < minSize){
                minSize = size0 + size1;
                level = i;
            }               
            Abc_NtkDelete(p0);
            Abc_NtkDelete(p1);
        }

        assert(level != -1);

        int Cost0 = -1, Cost1 = -1;
        std::vector<std::string> Esop0, Esop1;

        CofList[level] = true;


        Abc_Ntk_t* p0 = AIGCOF(pNtk, level, 0);
        ScalPSDKRO(p0, numCof -1, Cost0, Esop0);
        
        Abc_Ntk_t* p1 = AIGCOF(pNtk, level, 1);
        ScalPSDKRO(p1, numCof -1, Cost1, Esop1);


        Abc_NtkDelete(p0);
        Abc_NtkDelete(p1);


        CofList[level] = false;

        Cost = Cost0 + Cost1 ;
        
        /* TODO: Synthesis Esop */
        /*
        for(int i = 0; i < Esop0.size(); i++){
            assert(Esop0[i][level] == '-');
            std::string tem = Esop0[i];
            tem[level] = 0;
            Esop.push_back(tem);
        }
        for(int i = 0; i < Esop1.size(); i++){
            assert(Esop1[i][level] == '-');
            std::string tem = Esop1[i];
            tem[level] = 1;
            Esop.push_back(tem);
        }
        */
        return;
    }
}


void ScalablePSDKROMain(Abc_Ntk_t* pNtk, int numCof){
    assert(Abc_NtkIsStrash(pNtk));
    numPI = Abc_NtkPiNum(pNtk);

    CofList.clear();
    CofList.resize(numPI);
    for(int i = 0; i < numPI; i++)
        CofList[i] = false;

    int Cost;
    std::vector<std::string> Esop;

    ScalPSDKRO(pNtk, numCof, Cost, Esop);
    std::cout << "#Terms: " << Cost << std::endl;
}
