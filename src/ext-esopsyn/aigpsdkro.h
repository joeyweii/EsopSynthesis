#include "utils.h"

void AigPSDKROMain(Abc_Ntk_t* pNtk);
Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2);
void PSDKRO(Abc_Ntk_t* pNtk, int level, int& Cost, std::vector<std::string>& Esop);
