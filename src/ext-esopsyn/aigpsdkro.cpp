#include "utils.h"

static int numPI;

/**Function*************************************************************

  Synopsis    [Do cofactor on AIG]

  Description [
                var: specify which PI to be cofactored
                phase: specify postive cofactor or negative cofactor
               ]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase){
    char Buffer[1000];
    Abc_Obj_t* constNode = NULL;
    Abc_Ntk_t* pNtkNew = NULL;
    Abc_Obj_t* pObjNew = NULL;

    Abc_Obj_t* pObj;
    int i = -1;

    assert(Abc_NtkPoNum(pNtk) == 1);

    pNtkNew = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    sprintf( Buffer, "new");
    pNtkNew->pName = Extra_UtilStrsav(Buffer);

    if(phase) constNode = Abc_AigConst1(pNtkNew);
    else constNode = Abc_ObjNot(Abc_AigConst1(pNtkNew));

    Abc_AigConst1(pNtk)->pCopy = Abc_AigConst1(pNtkNew);

    Abc_NtkForEachCi( pNtk, pObj, i ){
        pObjNew = Abc_NtkCreatePi(pNtkNew);
        
        if(i == var) pObj->pCopy = constNode;
        else pObj->pCopy = pObjNew;
        Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    }

    assert( Abc_NtkIsDfsOrdered(pNtk) );
    Abc_AigForEachAnd( pNtk, pObj, i ){
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );
    }

    Abc_Obj_t* pPo = Abc_NtkCreatePo( pNtkNew );
    Abc_ObjAssignName( pPo, "o", NULL );
    
    if(Abc_ObjFaninC0(Abc_NtkPo(pNtk, 0)))
        Abc_ObjAddFanin(pPo, Abc_ObjNot(Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))->pCopy));
    else
        Abc_ObjAddFanin(pPo, Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))->pCopy);;

    return pNtkNew;
}

/**Function*************************************************************

  Synopsis    [xor two AIG]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2){
    char Buffer[1000];
    Abc_Ntk_t* pNtkNew = NULL;
    Abc_Obj_t* pObjNew = NULL;
    Abc_Obj_t* pPo = NULL;

    Abc_Obj_t* pObj;
    int i = -1;
    assert(Abc_NtkPiNum(pNtk1) == Abc_NtkPiNum(pNtk2));
    assert(Abc_NtkPoNum(pNtk1) == 1);
    assert(Abc_NtkPoNum(pNtk2) == 1);
    
    Abc_NtkForEachCi( pNtk1, pObj, i )
        assert(strcmp(Abc_ObjName(pObj), Abc_ObjName(Abc_NtkPi(pNtk2, i))) == 0);
    
    pNtkNew = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    sprintf( Buffer, "new");
    pNtkNew->pName = Extra_UtilStrsav(Buffer);

    Abc_AigConst1(pNtk1)->pCopy = Abc_AigConst1(pNtkNew);
    Abc_AigConst1(pNtk2)->pCopy = Abc_AigConst1(pNtkNew);


    Abc_NtkForEachCi( pNtk1, pObj, i ){
        pObjNew = Abc_NtkCreatePi(pNtkNew);
    
        pObj->pCopy = pObjNew;
        pObj = Abc_NtkPi(pNtk2, i);
        pObj->pCopy = pObjNew;
        Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    }

    assert( Abc_NtkIsDfsOrdered(pNtk1) );
    Abc_AigForEachAnd( pNtk1, pObj, i ){
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );
    }

    assert( Abc_NtkIsDfsOrdered(pNtk2) );
    Abc_AigForEachAnd( pNtk2, pObj, i ){
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );
    }

    pPo = Abc_NtkCreatePo( pNtkNew );
    Abc_ObjAssignName( pPo, "o", NULL );

    pObjNew = Abc_AigXor( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(Abc_NtkCo(pNtk1, 0)), Abc_ObjChild0Copy(Abc_NtkCo(pNtk2, 0)));
    Abc_ObjAddFanin( pPo, pObjNew);

    // ************* TODO: optimize pObjNew ***************
    return pNtkNew;

}

/**Function*************************************************************

  Synopsis    [AIG PSDKRO recursive function]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

static void AigPSDKRO(Abc_Ntk_t* pNtk, int level, int& Cost, std::vector<std::string>& Esop){
    Abc_Obj_t* f = Abc_ObjFanin0(Abc_NtkPo(pNtk, 0));
    if(f == Abc_AigConst1(pNtk)){
        // f == 0
        if(Abc_ObjFaninC0(Abc_NtkPo(pNtk, 0))){
            Cost = 0;
        }
        // f == 1
        else{
            Cost = 1;
            std::string alldash;
            for(int i = 0; i < numPI; i++) alldash += "-";
            Esop.push_back(alldash);
        }
        return;
    }

    assert(level != numPI);

    Abc_Ntk_t* p0 = AIGCOF(pNtk, level, 0);
    Abc_Ntk_t* p1 = AIGCOF(pNtk, level, 1);
    Abc_Ntk_t* p01 = AIGXOR(p0, p1);

    std::vector<std::string> Esop0;
    std::vector<std::string> Esop1;
    std::vector<std::string> Esop01;
    int Cost0;
    int Cost1;
    int Cost01;

    AigPSDKRO(p0, level+1, Cost0, Esop0);
    Abc_NtkDelete(p0);
    AigPSDKRO(p1, level+1, Cost1, Esop1);
    Abc_NtkDelete(p1);
    AigPSDKRO(p01, level+1, Cost01, Esop01);
    Abc_NtkDelete(p01);

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

/**Function*************************************************************

  Synopsis    [AIG PSDKRO extract]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void AigPSDKROMain(Abc_Ntk_t* pNtk){
    assert(Abc_NtkPoNum(pNtk) == 1);
    numPI = Abc_NtkPiNum(pNtk);

    std::vector<std::string> Esop;
    int Cost;
    AigPSDKRO(pNtk, 0, Cost, Esop); 
    printf("Cost: %d\n", Cost);
    printf("Esop:\n");
    for(int i = 0; i < Esop.size(); i++){
        std::cout << Esop[i] << std::endl;
    }
}