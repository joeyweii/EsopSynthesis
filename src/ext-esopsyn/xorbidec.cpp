#include "xorbidec.h"

#define DEBUG 0

enum Set{
  XC,
  XB,
  XA,
  XAB
};

/**Function*************************************************************

  Synopsis    [Add buffer enable to solver.]

  Description [Add (iVarA == fCompl(iVarB)) relation constrain to solver with control varaible iVarEn]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int my_sat_solver_add_buffer_enable( sat_solver * pSat, int iVarA, int iVarB, int iVarEn, int fCompl )
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

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, int fPrintParti, int& nSat){

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

    std::vector<enum Set> bestParti;
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
        nSat++;
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
            vParti.push_back(XAB);
            numXAB++;
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
        if(fPrintParti){
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

  Synopsis    [XorBidec main function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int NtkXorBidec(Abc_Ntk_t* pNtk, int fPrintParti, int fPrintSatNum){
  Abc_Obj_t* pPo = NULL;
  int iPo = -1;

  abctime clk = Abc_Clock();

  int nSuccess = 0;
  int nSat = 0;
  Abc_NtkForEachPo(pNtk, pPo, iPo){
      // create cone for the current PO
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

      if( Abc_ObjFaninC0(pPo) )
        Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

      if(NtkXorBidecSingleOutput(pSubNtk, fPrintParti, nSat)) nSuccess++;
  }

  std::cout << "Number of bidecomposable POs: " << nSuccess << std::endl;
  Abc_PrintTime( 1, "Time used:", Abc_Clock() - clk );
  if(fPrintSatNum) std::cout << "Number of SAT solving called: " << nSat << std::endl;
  return 1;
    
}