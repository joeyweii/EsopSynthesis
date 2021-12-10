#include "utils.h"

int my_sat_solver_add_buffer_enable( sat_solver * pSat, int iVarA, int iVarB, int iVarEn, int fCompl );
int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, int fPrintParti, int& nSat);
int NtkXorBidec(Abc_Ntk_t* pNtk, int fPrintParti, int fPrintSatNum);