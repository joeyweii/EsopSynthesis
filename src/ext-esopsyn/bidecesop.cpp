#include "bidecesop.h"
void CleanUnusedPi(Abc_Ntk_t* pNtk){
    Abc_Obj_t* pObj;
    int i;
    Abc_NtkForEachPi( pNtk, pObj, i){
        if (Abc_ObjFanoutNum(pObj) == 0){
            Abc_NtkDeleteObj(pObj);
            i--;
        }
    }
}

void BidecEsopSingleOutput(Abc_Ntk_t* pNtk){
    std::vector<Abc_Ntk_t*> Q;
    std::vector<Abc_Ntk_t*> subNtks;
    Q.push_back(pNtk);

    while(!Q.empty()){
        std::vector<enum Set> vParti;
        Abc_Ntk_t* subNtk = Q.back();
        Q.pop_back();

        int result = NtkXorBidecSingleOutput(subNtk, vParti);
  
        if(!result){
            subNtks.push_back(subNtk);
        }
        else{
            Abc_Ntk_t *fA = NULL, *fB = NULL;
            NtkXorBidecSynthesis(subNtk, vParti, fA, fB);
            CleanUnusedPi(fA);
            CleanUnusedPi(fB);
            Q.push_back(fA);
            Q.push_back(fB);
            Abc_NtkDelete(subNtk);
        }
    }

    std::cout << "#subNtks: " << subNtks.size() << std::endl;

    for(int i = 0; i < subNtks.size(); i++){
        Abc_NtkDelete(subNtks[i]);
    }
}

void BidecEsopMain(Abc_Ntk_t* pNtk){
    Abc_Obj_t* pPo;
    int iPo;

    Abc_NtkForEachPo(pNtk, pPo, iPo){
        // create cone for the current PO
        Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

        if( Abc_ObjFaninC0(pPo) )
            Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

        BidecEsopSingleOutput(pSubNtk);
    }
}