#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "misc/vec/vec.h"

static Vec_Wec_t* readPlaIntoESOP(char* pFileName)
{
    std::fstream inFile;
    inFile.open(pFileName, std::ios::in);    

    std::string inStr;
    int nVars = -1;
    Vec_Wec_t* vEsop = Vec_WecAlloc(0);
    
    while(inFile >> inStr)
    {
        if(inStr == ".i")
        {
            inFile >> nVars;
        }
        else if(inStr == ".o")
        {
            int nOutputs;
            inFile >> nOutputs;
            assert(nOutputs == 1);
        }
        else if(inStr == ".type")
        {
            inFile >> inStr;
            assert(inStr == "esop");
        }
        else if(inStr == ".e")
        {
            break;
        }
        else
        {
            assert(vEsop != NULL);
            
            Vec_Int_t *vCube = Vec_WecPushLevel(vEsop);
            Vec_IntGrow(vCube, nVars + 2);

            for(int i = 0, end_i = inStr.size(); i < end_i; ++i)
            {
                if(inStr[i] == '0')
                    Vec_IntPush(vCube, 2*i+1);
                else if(inStr[i] == '1')
                    Vec_IntPush(vCube, 2*i); 
            }

            Vec_IntPush(vCube, -1);
            inFile >> inStr;
        }
    }

    return vEsop;
}

void DcMinimizeMain(char* pFileNameIn, char* pFileNameOut)
{
    Vec_Wec_t *vEsop = readPlaIntoESOP(pFileNameIn);
    
    Vec_Int_t * vCube;
    int Lit, c, k;
    Vec_WecForEachLevel( vEsop, vCube, c )
    {
        Vec_IntForEachEntry( vCube, Lit, k )
        {
            printf("%d", Lit);
        }
        printf("\n");
    }

    return;
}
