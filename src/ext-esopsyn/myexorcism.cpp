#include "myexorcism.h"

void ExorcismSingleOutput(Abc_Ntk_t* pNtk){
    Abc_Ntk_t* pStrash = Abc_NtkStrash( pNtk, 0, 1, 0 );
    Aig_Man_t* pAig = Abc_NtkToDar( pStrash, 0, 0 );
    Abc_NtkDelete( pStrash );
    Gia_Man_t* pGia = Gia_ManFromAig( pAig );

    Vec_Wec_t * vEsop = NULL;
    Eso_ManCompute(pGia, 0, &vEsop);
    if(0){
        for(int i = 0; i < vEsop->nSize; i++){
            for(int j = 0; j < vEsop->pArray[i].nSize; j++){
                std::cout << vEsop->pArray[i].pArray[j] << " ";
            }
            std::cout << std::endl;
        }
    }
    
    assert(Abc_NtkPoNum(pNtk) == 1);
    std::cout << "PO " << Abc_ObjName(Abc_NtkPo(pNtk, 0)) << ": " << vEsop->nSize << std::endl;
}

void My_Exorcism(Abc_Ntk_t * pNtk, int nCubeMax){
    Abc_Obj_t* pPo = NULL;
    int iPo = -1;

    Abc_NtkForEachPo(pNtk, pPo, iPo){
        // create cone for the current PO
        Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

        if( Abc_ObjFaninC0(pPo) )
            Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

        ExorcismSingleOutput(pSubNtk);
    }
}