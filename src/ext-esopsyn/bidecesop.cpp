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

    abctime clk = Abc_Clock();
    Q.push_back(pNtk);
    std::cout << "#PI: " << Abc_NtkPiNum(pNtk) << std::endl;

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
    Abc_PrintTime( 1, "Decompose Time", Abc_Clock() - clk );

    for(int i = 0; i < subNtks.size(); i++){
        std::cout << "subNtk[" << i << "] #PI: " << Abc_NtkPiNum(subNtks[i]) << std::endl;
    }

    
    clk = Abc_Clock();

    int nCubes = 0;
    for(int i = 0; i < subNtks.size(); i++){
        std::cout << "i: " << i << std::endl;
        Abc_Ntk_t* subNtk = subNtks[i];
        Abc_Ntk_t* pStrash = Abc_NtkStrash( subNtk, 0, 1, 0 );
        Aig_Man_t* pAig = Abc_NtkToDar( pStrash, 0, 0 );
        Abc_NtkDelete( pStrash );
        Gia_Man_t* pGia = Gia_ManFromAig( pAig );

        Vec_Wec_t* vEsop;
        Eso_ManCompute(pGia, 0, &vEsop);
        nCubes += vEsop->nSize;
    }
    std::cout << "nCubes: " << nCubes << std::endl;
    Abc_PrintTime( 1, "Starting Cover Compute Time", Abc_Clock() - clk );
    
    for(int i = 0; i < subNtks.size(); i++){
        Abc_NtkDelete(subNtks[i]);
    }
}

void BidecEsopMain(Abc_Ntk_t* pNtk, int fOutput){
    Abc_Obj_t* pPo;
    int iPo;

    Abc_NtkForEachPo(pNtk, pPo, iPo){
        if(fOutput != -1 && iPo != fOutput) continue;
        // create cone for the current PO
        Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

        if( Abc_ObjFaninC0(pPo) )
            Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

        BidecEsopSingleOutput(pSubNtk);
    }
}