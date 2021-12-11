#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "aig/aig/aig.h"

extern "C" Aig_Man_t *  Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
extern "C" int Abc_ExorcismMain( Vec_Wec_t * vEsop, int nIns, int nOuts, char * pFileNameOut, int Quality, int Verbosity, int nCubesMax, int fUseQCost );
extern "C" Vec_Wec_t * Abc_ExorcismNtk2Esop( Abc_Ntk_t * pNtk );
extern "C" Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
extern "C" int Abc_NtkDarCec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nConfLimit, int fPartition, int fVerbose );
