#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <climits>
#include <iostream>
#include <unordered_map>
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "aig/aig/aig.h"

enum Set{
  XC,
  XB,
  XA,
  XAB
};

typedef struct PSDKRONode PSDKRONode;
struct PSDKRONode{
    std::vector<std::string> Esop;
    int Cost;
};

extern "C" Aig_Man_t *  Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
extern "C" int Abc_ExorcismMain( Vec_Wec_t * vEsop, int nIns, int nOuts, char * pFileNameOut, int Quality, int Verbosity, int nCubesMax, int fUseQCost );
extern "C" Vec_Wec_t * Abc_ExorcismNtk2Esop( Abc_Ntk_t * pNtk );
extern "C" Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
extern "C" int Abc_NtkDarCec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nConfLimit, int fPartition, int fVerbose );
extern "C" Gia_Man_t * Gia_ManFromAig( Aig_Man_t * p );
extern "C" void Abc_NodeShowBdd( Abc_Obj_t * pNode, int fCompl );

extern int NtkXorBidecMain(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis, int fOutput);
extern void MintEsopMain(Abc_Obj_t* pNode, std::ofstream& OutFile);
extern void MyExorcismMain(Abc_Ntk_t * pNtk, int nCubeMax, int fOutput);
extern void AigPSDKROMain(Abc_Ntk_t* pNtk);
extern void BddPSDKROMain(Abc_Ntk_t* pNtk, char* pFileNameOut);
extern void BidecEsopMain(Abc_Ntk_t* pNtk, int fOutput);
extern void ScalablePSDKROMain(Abc_Ntk_t* pNtk, int numCof);

extern int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti);
extern int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB);
extern void PrintAig(Abc_Ntk_t* pNtk);
extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2);
