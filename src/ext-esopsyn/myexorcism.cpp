#include "myexorcism.h"

void ExorcismSingleOutput(Abc_Ntk_t* pNtk, Vec_Wec_t*& vEsop){
    Abc_Ntk_t* pStrash = Abc_NtkStrash( pNtk, 0, 1, 0 );
    Aig_Man_t* pAig = Abc_NtkToDar( pStrash, 0, 0 );
    Abc_NtkDelete( pStrash );
    Gia_Man_t* pGia = Gia_ManFromAig( pAig );

    Eso_ManCompute(pGia, 0, &vEsop);
    if(0){
        for(int i = 0; i < vEsop->nSize; i++){
            for(int j = 0; j < vEsop->pArray[i].nSize; j++){
                std::cout << vEsop->pArray[i].pArray[j] << " ";
            }
            std::cout << std::endl;
        }
    } 
}

void My_Exorcism(Abc_Ntk_t * pNtk, int nCubeMax, int fOutput){
    Abc_Obj_t* pPo = NULL;
    int iPo = -1;

    Abc_NtkForEachPo(pNtk, pPo, iPo){
        if(fOutput != -1 && iPo != fOutput) continue;
        // create cone for the current PO
        Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

        if( Abc_ObjFaninC0(pPo) )
            Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );


        abctime clk = Abc_Clock();

        Vec_Wec_t* vEsop;
        ExorcismSingleOutput(pSubNtk, vEsop);

        assert(Abc_NtkPoNum(pSubNtk) == 1);
        std::cout << "PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << " - nCubes:" << vEsop->nSize << std::endl;
        
        Abc_PrintTime( 1, "Starting Cover Compute Time", Abc_Clock() - clk );
    }
}