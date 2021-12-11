#include "utils.h"
enum Set{
  XC,
  XB,
  XA,
  XAB
};

void PrintAig(Abc_Ntk_t* pNtk);
int my_sat_solver_add_buffer_enable( sat_solver * pSat, int iVarA, int iVarB, int iVarEn, int fCompl );
int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti);
int NtkXorBidec(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis);
int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB);
Abc_Ntk_t * My_NtkMiterInt( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int fComb, int nPartSize, int fImplic, int fMulti );
void My_NtkMiterPrepare( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, Abc_Ntk_t * pNtkMiter, int fComb, int nPartSize, int fMulti );