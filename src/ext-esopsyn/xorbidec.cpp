#include "xorbidec.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"

#include <iostream>

#define DEBUG 0

extern "C" Aig_Man_t *  Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
extern "C" int Abc_NtkDarCec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nConfLimit, int fPartition, int fVerbose );

/**Function*************************************************************

  Synopsis    [Xor two Network]

  Description [Only single output is allowed. The name of PO is set the same as pNtk1 and pNtk2.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static void Abc_NtkMiterFinalize( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter, int fComb, int nPartSize, int fImplic, int fMulti )
{
    Vec_Ptr_t * vPairs;
    Abc_Obj_t * pMiter, * pNode;
    int i;
    assert( nPartSize == 0 || fMulti == 0 );
    // collect the PO pairs from both networks
    vPairs = Vec_PtrAlloc( 100 );
    if ( fComb )
    {
        // collect the CO nodes for the miter
        Abc_NtkForEachCo( pNtk1, pNode, i )
        {
            if ( fMulti )
            {
                pMiter = Abc_AigXor( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild0Copy(Abc_NtkCo(pNtk2, i)) );
                Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,i), pMiter );
            }
            else
            {
                Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
                pNode = Abc_NtkCo( pNtk2, i );
                Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
            }
        }
    }
    else
    {
        // collect the PO nodes for the miter
        Abc_NtkForEachPo( pNtk1, pNode, i )
        {
            if ( fMulti )
            {
                pMiter = Abc_AigXor( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild0Copy(Abc_NtkCo(pNtk2, i)) );
                Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,i), pMiter );
            }
            else
            {
                Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
                pNode = Abc_NtkPo( pNtk2, i );
                Vec_PtrPush( vPairs, Abc_ObjChild0Copy(pNode) );
            }
        }
        // connect new latches
        Abc_NtkForEachLatch( pNtk1, pNode, i )
            Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
        Abc_NtkForEachLatch( pNtk2, pNode, i )
            Abc_ObjAddFanin( Abc_ObjFanin0(pNode)->pCopy, Abc_ObjChild0Copy(Abc_ObjFanin0(pNode)) );
    }
    // add the miter
    if ( nPartSize <= 0 )
    {
        if ( !fMulti )
        {
            pMiter = Abc_AigMiter( (Abc_Aig_t *)pNtkMiter->pManFunc, vPairs, fImplic );
            Abc_ObjAddFanin( Abc_NtkPo(pNtkMiter,0), pMiter );
        }
    }
    else
    {
        char Buffer[1024];
        Vec_Ptr_t * vPairsPart;
        int nParts, i, k, iCur;
        assert( Vec_PtrSize(vPairs) == 2 * Abc_NtkCoNum(pNtk1) );
        // create partitions
        nParts = Abc_NtkCoNum(pNtk1) / nPartSize + (int)((Abc_NtkCoNum(pNtk1) % nPartSize) > 0);
        vPairsPart = Vec_PtrAlloc( nPartSize );
        for ( i = 0; i < nParts; i++ )
        {
            Vec_PtrClear( vPairsPart );
            for ( k = 0; k < nPartSize; k++ )
            {
                iCur = i * nPartSize + k;
                if ( iCur >= Abc_NtkCoNum(pNtk1) )
                    break;
                Vec_PtrPush( vPairsPart, Vec_PtrEntry(vPairs, 2*iCur  ) );
                Vec_PtrPush( vPairsPart, Vec_PtrEntry(vPairs, 2*iCur+1) );
            }
            pMiter = Abc_AigMiter( (Abc_Aig_t *)pNtkMiter->pManFunc, vPairsPart, fImplic );
            pNode = Abc_NtkCreatePo( pNtkMiter );
            Abc_ObjAddFanin( pNode, pMiter );
            // assign the name to the node
            if ( nPartSize == 1 )
                sprintf( Buffer, "%s", Abc_ObjName(Abc_NtkCo(pNtk1,i)) );
            else
                sprintf( Buffer, "%d", i );
            Abc_ObjAssignName( pNode, "miter_", Buffer );
        }
        Vec_PtrFree( vPairsPart );
    }
    Vec_PtrFree( vPairs );
}

static void Abc_NtkMiterAddOne( Abc_Ntk_t * pNtk, Abc_Ntk_t * pNtkMiter )
{
    Abc_Obj_t * pNode;
    int i;
    assert( Abc_NtkIsDfsOrdered(pNtk) );
    Abc_AigForEachAnd( pNtk, pNode, i )
        pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode), Abc_ObjChild1Copy(pNode) );
}


static void My_NtkMiterPrepare( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter, int fComb, int nPartSize, int fMulti )
{
    Abc_Obj_t * pObj, * pObjNew;
    int i;
    // clean the copy field in all objects
//    Abc_NtkCleanCopy( pNtk1 );
//    Abc_NtkCleanCopy( pNtk2 );
    Abc_AigConst1(pNtk1)->pCopy = Abc_AigConst1(pNtkMiter);
    Abc_AigConst1(pNtk2)->pCopy = Abc_AigConst1(pNtkMiter);

    if ( fComb )
    {
        // create new PIs and remember them in the old PIs
        Abc_NtkForEachCi( pNtk1, pObj, i )
        {
            pObjNew = Abc_NtkCreatePi( pNtkMiter );
            // remember this PI in the old PIs
            pObj->pCopy = pObjNew;
            pObj = Abc_NtkCi(pNtk2, i);  
            pObj->pCopy = pObjNew;
            // add name
            Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
        }
        if ( nPartSize <= 0 )
        {
            // create POs
            if ( fMulti )
            {
                Abc_NtkForEachCo( pNtk1, pObj, i )
                {
                    pObjNew = Abc_NtkCreatePo( pNtkMiter );
                    Abc_ObjAssignName( pObjNew, "miter", Abc_ObjName(pObjNew) );
                }

            }
            else
            {
                pObjNew = Abc_NtkCreatePo( pNtkMiter );
                Abc_ObjAssignName( pObjNew, Abc_ObjName(Abc_NtkPo(pNtk1, 0)), NULL );
            }
        }
    }
    else
    {
        // create new PIs and remember them in the old PIs
        Abc_NtkForEachPi( pNtk1, pObj, i )
        {
            pObjNew = Abc_NtkCreatePi( pNtkMiter );
            // remember this PI in the old PIs
            pObj->pCopy = pObjNew;
            pObj = Abc_NtkPi(pNtk2, i);  
            pObj->pCopy = pObjNew;
            // add name
            Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), NULL );
        }
        if ( nPartSize <= 0 )
        {
            // create POs
            if ( fMulti )
            {
                Abc_NtkForEachPo( pNtk1, pObj, i )
                {
                    pObjNew = Abc_NtkCreatePo( pNtkMiter );
                    Abc_ObjAssignName( pObjNew, "miter", Abc_ObjName(pObjNew) );
                }

            }
            else
            {
                pObjNew = Abc_NtkCreatePo( pNtkMiter );
                Abc_ObjAssignName( pObjNew, Abc_ObjName(Abc_NtkPo(pNtk1, 0)), NULL );
            }
        }
        // create the latches
        Abc_NtkForEachLatch( pNtk1, pObj, i )
        {
            pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
            // add names
            Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_1" );
            Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_1" );
            Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_1" );
        }
        Abc_NtkForEachLatch( pNtk2, pObj, i )
        {
            pObjNew = Abc_NtkDupBox( pNtkMiter, pObj, 0 );
            // add name
            Abc_ObjAssignName( pObjNew, Abc_ObjName(pObj), "_2" );
            Abc_ObjAssignName( Abc_ObjFanin0(pObjNew),  Abc_ObjName(Abc_ObjFanin0(pObj)), "_2" );
            Abc_ObjAssignName( Abc_ObjFanout0(pObjNew), Abc_ObjName(Abc_ObjFanout0(pObj)), "_2" );
        }
    }
}

Abc_Ntk_t * My_NtkMiterInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int fComb, int nPartSize, int fImplic, int fMulti )
{
    char Buffer[1000];
    Abc_Ntk_t * pNtkMiter;

    assert( Abc_NtkIsStrash(pNtk1) );
    assert( Abc_NtkIsStrash(pNtk2) );

    // start the new network
    pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH, ABC_FUNC_AIG, 1 );
    sprintf( Buffer, "%s_%s_miter", pNtk1->pName, pNtk2->pName );
    pNtkMiter->pName = Extra_UtilStrsav(Buffer);

    // perform strashing
    My_NtkMiterPrepare( pNtk1, pNtk2, pNtkMiter, fComb, nPartSize, fMulti );
    Abc_NtkMiterAddOne( pNtk1, pNtkMiter );
    Abc_NtkMiterAddOne( pNtk2, pNtkMiter );
    Abc_NtkMiterFinalize( pNtk1, pNtk2, pNtkMiter, fComb, nPartSize, fImplic, fMulti );
    Abc_AigCleanup((Abc_Aig_t *)pNtkMiter->pManFunc);

    // make sure that everything is okay
    if ( !Abc_NtkCheck( pNtkMiter ) )
    {
        printf( "Abc_NtkMiter: The network check has failed.\n" );
        Abc_NtkDelete( pNtkMiter );
        return NULL;
    }
    return pNtkMiter;
}

static Abc_Ntk_t * My_NtkXor( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int fComb, int nPartSize, int fImplic, int fMulti )
{
    Abc_Ntk_t * pTemp = NULL;
    int fRemove1, fRemove2;
    assert( Abc_NtkHasOnlyLatchBoxes(pNtk1) );
    assert( Abc_NtkHasOnlyLatchBoxes(pNtk2) );
    // check that the networks have the same PIs/POs/latches
    if ( !Abc_NtkCompareSignals( pNtk1, pNtk2, fImplic, fComb ) )
        return NULL;
    // make sure the circuits are strashed 
    fRemove1 = (!Abc_NtkIsStrash(pNtk1) || Abc_NtkGetChoiceNum(pNtk1)) && (pNtk1 = Abc_NtkStrash(pNtk1, 0, 0, 0));
    fRemove2 = (!Abc_NtkIsStrash(pNtk2) || Abc_NtkGetChoiceNum(pNtk2)) && (pNtk2 = Abc_NtkStrash(pNtk2, 0, 0, 0));
    if ( pNtk1 && pNtk2 )
        pTemp = My_NtkMiterInt( pNtk1, pNtk2, fComb, nPartSize, fImplic, fMulti );
    if ( fRemove1 )  Abc_NtkDelete( pNtk1 );
    if ( fRemove2 )  Abc_NtkDelete( pNtk2 );
    return pTemp;
}

/**Function*************************************************************

  Synopsis    [Print Aig Network]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void PrintAig(Abc_Ntk_t* pNtk){
    assert(Abc_NtkIsStrash(pNtk));

    Abc_Obj_t* pPi;
    int i;
    std::cout << "** PI **" << std::endl;
    Abc_NtkForEachPi( pNtk, pPi, i ){
        std::cout << "Object Id = " << Abc_ObjId(pPi) << " name = " << Abc_ObjName(pPi) << std::endl;
    }

    Abc_Obj_t* pPo;
    std::cout << "** PO **" << std::endl;
    Abc_NtkForEachPo( pNtk, pPo, i ){
        std::cout << "Object Id = " << Abc_ObjId(pPo) << " name = " << Abc_ObjName(pPo) << std::endl;
    
        assert(Abc_ObjFaninNum(pPo) == 1);
        std::cout << "   Fanin0 Id = " << Abc_ObjId(Abc_ObjFanin0(pPo))<< " cp = " <<Abc_ObjFaninC0(pPo) << std::endl;
    
    }

    std::cout << "** Internal Node **" << std::endl;
    Abc_Obj_t* pObj;
    Abc_NtkForEachObj( pNtk, pObj, i ){
        if(!Abc_ObjIsPi(pObj) && !Abc_ObjIsPo(pObj)){
            if(Abc_ObjFaninNum(pObj) == 0)  std::cout << "Object Id = " << Abc_ObjId(pObj) << " name = " << Abc_ObjName(pObj) << std::endl;
            else{
                std::cout << "Object Id = " << Abc_ObjId(pObj) << " name = " << Abc_ObjName(pObj) << std::endl;
                assert(Abc_ObjFaninNum(pObj) == 2);
                std::cout << "   Fanin0 Id = " << Abc_ObjId(Abc_ObjFanin0(pObj)) << " cp = " <<Abc_ObjFaninC0(pObj) << std::endl;
                std::cout << "   Fanin1 Id = " << Abc_ObjId(Abc_ObjFanin1(pObj)) << " cp = " <<Abc_ObjFaninC1(pObj) << std::endl;
            }
        }
    }
    std::cout << "---------" << std::endl;
}

/**Function*************************************************************

  Synopsis    [Add buffer enable to solver.]

  Description [Add (iVarA == fCompl(iVarB)) relation constrain to solver with control varaible iVarEn]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
static int my_sat_solver_add_buffer_enable( sat_solver * pSat, int iVarA, int iVarB, int iVarEn, int fCompl )
{
    lit Lits[3];
    int Cid;
    assert( iVarA >= 0 && iVarB >= 0 && iVarEn >= 0 );

    Lits[0] = toLitCond( iVarA, 0 );
    Lits[1] = toLitCond( iVarB, !fCompl );
    Lits[2] = toLitCond( iVarEn, 0 );
    Cid = sat_solver_addclause( pSat, Lits, Lits + 3 );
    assert( Cid );

    Lits[0] = toLitCond( iVarA, 1 );
    Lits[1] = toLitCond( iVarB, fCompl );
    Lits[2] = toLitCond( iVarEn, 0 );
    Cid = sat_solver_addclause( pSat, Lits, Lits + 3 );
    assert( Cid );
    return 0;
}

/**Function*************************************************************

  Synopsis    [Xor bidecompose a single output aig network.]

  Description [Return the partition of variables]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& bestParti){

    Aig_Obj_t* pObj = NULL;
    int iObj = -1;

    // derive CNF
    Aig_Man_t* pAigMan = Abc_NtkToDar(pNtk, 0, 0);
    Cnf_Dat_t* pCnf = Cnf_Derive(pAigMan, 1);

    // collect PIs' variable ids
    Vec_Int_t* vPiIds = Cnf_DataCollectPiSatNums(pCnf, pAigMan);
    int nPis = vPiIds->nSize;
    int nVars = pCnf->nVars;

    if(DEBUG){
      // 
      std::cout << "#Pi: " << nPis << std::endl;
 
      // Print Cnf
      printf("----CNF----\n");
      printf("#Clauses: %d \n", pCnf->nClauses);
      
      for(int i = 0; i < pCnf->nClauses; ++i){
        for(int* pLit = pCnf->pClauses[i]; pLit !=  pCnf->pClauses[i+1]; pLit++){
          printf("%s%d ", Abc_LitIsCompl(*pLit) ? "-":"", Abc_Lit2Var(*pLit) );
        }
        printf("\n");
      }
    }

    sat_solver* solver = sat_solver_new();

    // print added clauses
    if(DEBUG){
      solver->fPrintClause = 1;
      printf("----Added Clauses----\n");
    }

    sat_solver_setnvars(solver, 4*nVars);

    // write circuit X cnf into solver
    for(int i = 0; i < pCnf->nClauses; ++i)
      sat_solver_addclause(solver, pCnf->pClauses[i], pCnf->pClauses[i+1]);

    // write circuit X' cnf into solver
    Cnf_DataLift(pCnf, nVars);
    for(int i = 0; i < pCnf->nClauses; ++i)
      sat_solver_addclause(solver, pCnf->pClauses[i], pCnf->pClauses[i+1]);

    // write circuit X'' cnf into solver
    Cnf_DataLift(pCnf, nVars);
    for(int i = 0; i < pCnf->nClauses; ++i)
      sat_solver_addclause(solver, pCnf->pClauses[i], pCnf->pClauses[i+1]);

    // write circuit X''' cnf into solver
    Cnf_DataLift(pCnf, nVars);
    for(int i = 0; i < pCnf->nClauses; ++i)
      sat_solver_addclause(solver, pCnf->pClauses[i], pCnf->pClauses[i+1]);

    Cnf_DataLift(pCnf, (-3) * pCnf->nVars);

    // write f(X) == f(X') and f(X'') != f(X''') into solver
    assert(Aig_ManCoNum(pAigMan) == 1);
    Aig_ManForEachCo(pAigMan, pObj, iObj ){
      int var = pCnf->pVarNums[Aig_ObjId(pObj)];

      // f(X) == f(X')
      lit lits[2];
      lits[0] = var*2;
      lits[1] = (var + nVars)*2 + 1;
      sat_solver_addclause(solver, lits, lits+2);

      lits[0] = var*2 + 1;
      lits[1] = (var + nVars)*2;
      sat_solver_addclause(solver, lits, lits+2);

      // f(X'') != f(X''')
      lits[0] = (var + 2*nVars)*2;
      lits[1] = (var + 3*nVars)*2;
      sat_solver_addclause(solver, lits, lits+2);

      lits[0] = (var + 2*nVars)*2 + 1;
      lits[1] = (var + 3*nVars)*2 + 1;
      sat_solver_addclause(solver, lits, lits+2);
    }

    // write control variable constrains into solver
    std::vector<int> ctrAVars;
    std::vector<int> ctrBVars;

    // new variables in solver
    for(int i = 0; i < nPis; i++){
      ctrAVars.push_back(sat_solver_addvar(solver));
    }
    for(int i = 0; i < nPis; i++){
      ctrBVars.push_back(sat_solver_addvar(solver));
    }
    
    for(int i = 0; i < nPis; i++){
      int var = vPiIds->pArray[i];

      // control alpha constrains
      my_sat_solver_add_buffer_enable(solver, var, var + 2*nVars, ctrAVars[i], 0);
      my_sat_solver_add_buffer_enable(solver, var + nVars, var + 3*nVars, ctrAVars[i], 0);
   
      // control beta constrains
      my_sat_solver_add_buffer_enable(solver, var, var + nVars, ctrBVars[i], 0);
      my_sat_solver_add_buffer_enable(solver, var + 2*nVars, var + 3*nVars, ctrBVars[i], 0);
    }

    int bestXC = INT_MAX, bestXA = INT_MAX, bestXB = INT_MAX, bestXAB = INT_MAX;
    lit assump[2*nPis];

    // enumerate partition seed
    for(int ctrA = 0; ctrA < nPis -1; ctrA++){
      for(int ctrB = ctrA+1 ; ctrB < nPis; ctrB++){

        // unit assumption
        for(int i = 0; i < nPis; i++){
          assump[2*i] = (i == ctrA)? 2*ctrAVars[i] : 2*ctrAVars[i]+1;
          assump[2*i+1] = (i == ctrB)? 2*ctrBVars[i] : 2*ctrBVars[i]+1;
        }
      
        // solve
        int result = sat_solver_solve(solver, assump, assump + 2*nPis, 0, 0, 0, 0);
        if(result == l_True) continue;
        
        assert(result == l_False);
        
        // get conflict clause

        int conf_size = solver->conf_final.size;
        int conf_cls[conf_size];
        for(int i = 0; i < conf_size; i++)
            conf_cls[i] = solver->conf_final.ptr[i];

        // minimize the conflice clause
        for(int i = 0; i < conf_size; ++i)
            conf_cls[i]  += 1;

        for(int i = 0; i < conf_size; ++i){
            ABC_SWAP(int, conf_cls[i], conf_cls[conf_size-1]);
            result = sat_solver_solve(solver, conf_cls, conf_cls + conf_size - 1, 0, 0, 0, 0);
            if(result != l_False){  
                assert(result == l_True);
                ABC_SWAP(int, conf_cls[i], conf_cls[conf_size-1]);
                continue;
            }
            else{
                i--;
                conf_size--;
            } 
        }

        assert(sat_solver_solve(solver, conf_cls, conf_cls + conf_size, 0, 0, 0, 0) == l_False);

        for(int i = 0; i < conf_size; ++i)
            conf_cls[i]  -= 1;

        // find partition
        bool partition[2][nPis];
        for(int i = 0; i < nPis; i++){
          partition[0][i] = partition[1][i] = 1;
        }

        for(int i = 0; i < conf_size; i++){
          int var = conf_cls[i]/2 - 4*nVars;
          if(var >= nPis) partition[1][var-nPis] = 0;
          else partition[0][var] = 0;
        }
        
        std::vector<enum Set> vParti;
        int numXA = 0, numXB = 0, numXC = 0, numXAB = 0;
        for(int i = 0; i < nPis; i++){
          if(partition[0][i] == 0 && partition[1][i] == 0){
            vParti.push_back(XC);
            numXC++;
          }
          else if(partition[0][i] == 1 && partition[1][i] == 0){
            vParti.push_back(XA);
            numXA++;
          }
          else if(partition[0][i] == 0 && partition[1][i] == 1){
            vParti.push_back(XB);
            numXB++;
          }
          else{
            if(numXA > numXB){
              vParti.push_back(XB);
              numXB++;
            }
            else{
              vParti.push_back(XA);
              numXA++;
            }

            /*
            vParti.push_back(XAB);
            numXAB++;
            */
          }
        }

        if(DEBUG){
          printf("----Partition----\n");
          for(int i = 0; i < nPis; i++){
            if(vParti[i] == XC) printf("%d: XC\n", i);
            else if(vParti[i] == XA) printf("%d: XA\n", i);
            else if(vParti[i] == XB) printf("%d: XB\n", i);
            else printf("%d: XAB\n", i);
          }
        }

        // update best partition
        // if find minimal, break
        if(numXC == bestXC){
          ctrA = nPis;
          ctrB = nPis;
        }
        else if(numXC < bestXC){
          bestXC = numXC;
          bestXA = numXA;
          bestXB = numXB;
          bestXAB = numXAB;
          bestParti = vParti;
          if(bestXC == 0){
            ctrA = nPis;
            ctrB = nPis;
          }
        }
      }
    }

    if(bestXC != INT_MAX){
        if(DEBUG){
            printf("----Best Partition----\n");
                for(int i = 0; i < nPis; i++){
                if(bestParti[i] == XC) printf("%d: XC\n", i);
                else if(bestParti[i] == XA) printf("%d: XA\n", i);
                else if(bestParti[i] == XB) printf("%d: XB\n", i);
                else printf("%d: XAB\n", i);
            }
            printf("----Partition Set Size----\n");
            std::cout << "|XA|: " << bestXA << std::endl;
            std::cout << "|XB|: " << bestXB << std::endl;
            std::cout << "|XC|: " << bestXC << std::endl;
            std::cout << "|XAB|: " << bestXAB << std::endl;
        }
        return 1;
    }
    else return 0;
}

/**Function*************************************************************

  Synopsis    [Synthesis fA and fB according to the partition]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB){
  
  // ensure that there is no XAB variables
  for(int k = 0; k < vParti.size(); k++){
    assert(vParti[k] != XAB);
  }
  
  Abc_Ntk_t* trashNtk = NULL;

  fA = Abc_NtkDup(pNtk);
  fA = Abc_NtkToLogic(trashNtk = fA);
  Abc_NtkDelete(trashNtk);
  Abc_Obj_t* const0 = Abc_NtkCreateNodeConst0(fA);
	assert(Abc_ObjIsNode(const0));

  // replace XB's variables with 0 in fA
  Abc_Obj_t* pObj;
  int i;

  Abc_NtkForEachPi(fA, pObj, i){
      if(vParti[i] == XB){
          Abc_ObjTransferFanout(pObj, const0);
      }
  }

  // Remove dangling node in ntk
  fA = Abc_NtkStrash(trashNtk = fA, 0, 0, 0);
  Abc_NtkDelete(trashNtk);

  Abc_Ntk_t *fB1 = NULL, *fB2 = NULL;
  fB1 = Abc_NtkDup(pNtk);
  fB2 = Abc_NtkDup(pNtk);

  // replace XA's variables with 0 in fB to create fB1
  fB1 = Abc_NtkToLogic(trashNtk = fB1);
  Abc_NtkDelete(trashNtk);

  const0 = Abc_NtkCreateNodeConst0(fB1);
  assert(Abc_ObjIsNode(const0));

  Abc_NtkForEachPi(fB1, pObj, i){
      if(vParti[i] == XA){
          Abc_ObjTransferFanout(pObj, const0);
      }
  }

  // Remove dangling node in ntk
  fB1 = Abc_NtkStrash(trashNtk = fB1, 0, 0, 0);
  Abc_NtkDelete(trashNtk);

  // replace XC and XB's variables with 0 in fB to create fB2
  fB2 = Abc_NtkToLogic(trashNtk = fB2);
  Abc_NtkDelete(trashNtk);

  const0 = Abc_NtkCreateNodeConst0(fB2);
  assert(Abc_ObjIsNode(const0));

  Abc_NtkForEachPi(fB2, pObj, i){
      if(vParti[i] == XB || vParti[i] == XA){
          Abc_ObjTransferFanout(pObj, const0);
      }
  }

  // Remove dangling node in ntk
  fB2 = Abc_NtkStrash(trashNtk = fB2, 0, 0, 0);
  Abc_NtkDelete(trashNtk);
  
  // Xor fB1 and fB2 to get fB
  fB = My_NtkXor( fB1, fB2, 1, 0, 0, 0 );
  
  // Remove dangling node in ntk
  fB = Abc_NtkStrash(trashNtk = fB, 0, 0, 0);
  Abc_NtkDelete(trashNtk);

  // ensure theat fA xor fB == f
  if(0){
    Abc_Ntk_t* f = My_NtkXor( fA, fB, 1, 0, 0, 0 );
    Abc_NtkDarCec( f, pNtk, 0, 0, 0 );
  }
  return 0; 
}

/**Function*************************************************************

  Synopsis    [XorBidec main function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int NtkXorBidecMain(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis, int fOutput){
  Abc_Obj_t* pPo = NULL;
  int iPo = -1;

  abctime clk = Abc_Clock();

  int nSuccess = 0;
  std::vector<enum Set> vParti;
  Abc_NtkForEachPo(pNtk, pPo, iPo){
      if(fOutput != -1 && iPo != fOutput) continue;

      // create cone for the current PO
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

      if( Abc_ObjFaninC0(pPo) )
        Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

      int result = NtkXorBidecSingleOutput(pSubNtk, vParti);
      if(result){
        nSuccess++;
        if(fPrintParti){
          printf("----%s----\n", Abc_ObjName(pPo));
              for(int i = 0; i < Abc_NtkPiNum(pSubNtk); i++){
              if(vParti[i] == XC) printf("PI %d: XC\n", i);
              else if(vParti[i] == XA) printf("PI %d: XA\n", i);
              else if(vParti[i] == XB) printf("PI %d: XB\n", i);
              else printf("PI %d: XAB\n", i);
          }
        }

        if(fSynthesis){
          Abc_Ntk_t *fA = NULL, *fB = NULL;
          NtkXorBidecSynthesis(pSubNtk, vParti, fA, fB);
          std::cout << "----fA----" << std::endl;
          PrintAig(fA);
          std::cout << "----fB----" << std::endl;
          PrintAig(fB);
        }
      }
      else if(fPrintParti){
        printf("----%s----\n", Abc_ObjName(pPo));
        std::cout << "Fail" << std::endl;
      }

      
  }
  

  if(fOutput != -1){
    std::cout << "PO[" << fOutput << "] " << Abc_NtkPo(pNtk, fOutput) << " - Bidecomposable: ";
    if(nSuccess) std::cout << "Yes";
    else std::cout << "No";
    std::cout << std::endl;
  }
  else std::cout << "Number of bidecomposable POs: " << nSuccess << std::endl;
  Abc_PrintTime( 1, "Time used:", Abc_Clock() - clk );
  return 1;
    
}