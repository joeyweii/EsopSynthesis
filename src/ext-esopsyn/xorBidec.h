#ifndef _XORBIDEC_H_
#define _XORBIDEC_H_

#include <vector>

#include "base/main/main.h"

enum Set
{
  XC,
  XB,
  XA,
  XAB
};


int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& bestParti);
int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB);
#endif 
