#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "utils.h"

#define DEBUG 0

enum Set{
  XC,
  XB,
  XA,
  XAB
};

//  add (iVarA == fCompl(iVarB)) relation constrain to solver with control varaible iVarEn
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

// xorbidec single output
int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk){

    Aig_Obj_t* pObj = NULL;
    int iObj = -1;

    // derive CNF
    Aig_Man_t* pAigMan = Abc_NtkToDar(pNtk, 0, 0);
    Cnf_Dat_t* pCnf = Cnf_Derive(pAigMan, 0);

    // collect PIs' variable ids
    Vec_Int_t* vPiIds = Cnf_DataCollectPiSatNums(pCnf, pAigMan);
    int nPis = vPiIds->nSize;
    int nVars = pCnf->nVars ;

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

    // write f(X) == f(X') and f(X'') != f(X''')
    assert(Aig_ManCoNum(pAigMan) == 1);
    Aig_ManForEachCo(pAigMan , pObj, iObj ){
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

    // add control variable constrains
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
    int bestXC = INT_MAX;
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
        if(result == 1) continue;
        
        assert(result == -1);
        
        // find partition
        int* conf_cls = NULL;
        int conf_cls_num = sat_solver_final(solver, &conf_cls);

        //********* TODO: minimize conflict clause *********

        bool partition[2][nPis];
        for(int i = 0; i < nPis; i++){
          partition[0][i] = partition[1][i] = 1;
        }
        for(int i = 0; i < conf_cls_num; i++){
          int var = conf_cls[i]/2 - 4*pCnf->nVars;
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
          bestParti = vParti;
          if(bestXC == 0){
            ctrA = nPis;
            ctrB = nPis;
          }
        }
      }
    }

    printf("----Best Partition----\n");
        for(int i = 0; i < nPis; i++){
        if(bestParti[i] == XC) printf("%d: XC\n", i);
        else if(bestParti[i] == XA) printf("%d: XA\n", i);
        else if(bestParti[i] == XB) printf("%d: XB\n", i);
        else printf("%d: XAB\n", i);
    }
    return 0;
}

// xorbidec main function
int NtkXorBidec(Abc_Ntk_t* pNtk){
  Abc_Obj_t* pPo = NULL;
  int iPo = -1;

  Abc_NtkForEachPo(pNtk, pPo, iPo){
      // create cone for the current PO
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 1);

      if( Abc_ObjFaninC0(pPo) )
        Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

      NtkXorBidecSingleOutput(pSubNtk);
  }

  return 0;
    
}


// xorbidec command parser
int EsopSyn_CommandXorBidec(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  if(!Abc_NtkIsStrash(pNtk)){
    Abc_Print(-1, "Current network is not an AIG.\n");
    return 1;
  }

  NtkXorBidec(pNtk);

  return 0;

usage:
  Abc_Print(-2, "usage: xorbidec [-h]\n");
  Abc_Print(-2, "\t        for each PO, print the xor bidecomposition result\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}

// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "esopsyn", "xorbidec", EsopSyn_CommandXorBidec, 0);
}

// called during ABC termination
void destroy(Abc_Frame_t* pAbc)
{
}

// this object should not be modified after the call to Abc_FrameAddInitializer
Abc_FrameInitializer_t frame_initializer = { init, destroy };

// register the initializer a constructor of a global object
// called before main (and ABC startup)
struct registrar
{
    registrar() 
    {
        Abc_FrameAddInitializer(&frame_initializer);
    }
} esopsyn_registrar;