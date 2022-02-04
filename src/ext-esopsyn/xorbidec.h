#include "utils.h"

int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti);
int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB);