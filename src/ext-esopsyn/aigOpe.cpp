#include <iostream>

#include "base/main/main.h"

/**Function*************************************************************

  Synopsis    [Do cofactor on AIG]

  Description [
                var: specify which PI to be cofactored
                phase: specify postive cofactor or negative cofactor
               ]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase)
{
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

    Abc_NtkForEachCi( pNtk, pObj, i )
    {
        pObjNew = Abc_NtkCreatePi(pNtkNew);
        
        if(i == var) pObj->pCopy = constNode;
        else pObj->pCopy = pObjNew;
        Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    }

    assert( Abc_NtkIsDfsOrdered(pNtk) );
    Abc_AigForEachAnd( pNtk, pObj, i )
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );

    Abc_Obj_t* pPo = Abc_NtkCreatePo( pNtkNew );
    Abc_ObjAssignName( pPo, "o", NULL );
    
    if(Abc_ObjFaninC0(Abc_NtkPo(pNtk, 0)))
        Abc_ObjAddFanin(pPo, Abc_ObjNot(Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))->pCopy));
    else
        Abc_ObjAddFanin(pPo, Abc_ObjFanin0(Abc_NtkPo(pNtk, 0))->pCopy);;

    return pNtkNew;
}

/**Function*************************************************************

  Synopsis    [XOR two AIG]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2)
{
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


    Abc_NtkForEachCi( pNtk1, pObj, i )
    {
        pObjNew = Abc_NtkCreatePi(pNtkNew);
    
        pObj->pCopy = pObjNew;
        pObj = Abc_NtkPi(pNtk2, i);
        pObj->pCopy = pObjNew;
        Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
    }

    assert( Abc_NtkIsDfsOrdered(pNtk1) );
    Abc_AigForEachAnd( pNtk1, pObj, i )
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );

    assert( Abc_NtkIsDfsOrdered(pNtk2) );
    Abc_AigForEachAnd( pNtk2, pObj, i )
       pObj->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj) );

    pPo = Abc_NtkCreatePo( pNtkNew );
    Abc_ObjAssignName( pPo, "o", NULL );

    pObjNew = Abc_AigXor( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(Abc_NtkCo(pNtk1, 0)), Abc_ObjChild0Copy(Abc_NtkCo(pNtk2, 0)));
    Abc_ObjAddFanin( pPo, pObjNew);

    return pNtkNew;
}
