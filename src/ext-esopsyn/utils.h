#include <iostream>
#include <vector>
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "aig/aig/aig.h"

extern "C" Aig_Man_t *  Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );