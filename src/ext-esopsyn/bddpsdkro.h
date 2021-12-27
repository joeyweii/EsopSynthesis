#include "utils.h"
typedef struct PSDKRONode PSDKRONode;
struct PSDKRONode{
    std::vector<std::string> Esop;
    int Cost;
};


void BddPSDKROMain(Abc_Ntk_t* pNtk);