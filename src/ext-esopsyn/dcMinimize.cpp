#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include "misc/vec/vec.h"

extern "C" Vec_Wec_t* MyExorcism(Vec_Wec_t* vEsop, int nIns);

static void freeESOPList(std::vector<Vec_Wec_t*>& vEsopList)
{
    for(Vec_Wec_t* vEsop: vEsopList)
    {
        Vec_WecFree(vEsop); 
    }
}

static void splitESOPList(std::vector<Vec_Wec_t*>& vEsopList, std::vector<Vec_Wec_t*>& ret, int nVars, int groupSize)
{

    Vec_Int_t * vCube;
    int c, k, Lit;

    int count = 0;
    Vec_Wec_t* esop = Vec_WecAlloc(groupSize);

    for(Vec_Wec_t* vEsop : vEsopList)
    {
        Vec_WecForEachLevel( vEsop, vCube, c )
        {
            if(count == groupSize)
            {
                count = 0;
                ret.push_back(esop);
                esop = Vec_WecAlloc(groupSize);
            }
            
            Vec_Int_t *cube = Vec_WecPushLevel(esop);
            Vec_IntGrow(cube, nVars + 2);

            Vec_IntForEachEntry( vCube, Lit, k )
            {
                Vec_IntPush(cube, Lit);
            }
            ++count;
        }
    }
    ret.push_back(esop);
}

void printWecESOP(Vec_Wec_t* vEsop)
{
    std::cout << "--------" << std::endl;
    Vec_Int_t * vCube;
    int c, k, Lit;
    
    Vec_WecForEachLevel( vEsop, vCube, c )
    {
        Vec_IntForEachEntry( vCube, Lit, k )
        {
            std::cout << Lit << ' ';
        }
        std::cout << '\n';
    }
}

static std::pair<Vec_Wec_t*, int> readPlaIntoESOP(char* pFileName)
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
            std::string inStr2;
            inFile >> inStr2;
            assert(inStr2.size() == 1);
            if(inStr2 == "0") continue;

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
        }
    }

    return std::make_pair(vEsop, nVars);
}

static void writeESOPListIntoPla(std::vector<Vec_Wec_t*>& vEsopList, char* pFileName, int nVars)
{
    std::fstream outFile;
    outFile.open(pFileName, std::ios::out);

    if(!outFile.is_open())
        std::cerr << "Output file cannot be opened." << std::endl;
    else
    {
        Vec_Int_t * vCube;
        int c, k, Lit;

        outFile << ".i " << nVars << '\n';
        outFile << ".o 1\n";
        outFile << ".type esop\n";
        
        for(Vec_Wec_t* esop: vEsopList)
        {
            Vec_WecForEachLevel( esop, vCube, c )
            {
                std::string s(nVars, '-');
                Vec_IntForEachEntry( vCube, Lit, k )
                {
                    if(Lit < 0) continue;
                    if(Lit%2 == 0)
                        s[Lit/2] = '1';
                    else
                        s[Lit/2] = '0';
                }
                outFile << s << " 1\n";
            }
        }

        outFile << ".e\n";
        outFile.close();
    }
}

static int countNumberOfCubes(std::vector<Vec_Wec_t*> vEsopList)
{
    int ret = 0;
    for(Vec_Wec_t* esop: vEsopList)
        ret += Vec_WecSize(esop);

    return ret;

}

void DcMinimizeMain(char* pFileNameIn, char* pFileNameOut, int fGroupSize, int fIteration)
{
    Vec_Wec_t *vEsop;
    int nVars;
    std::tie(vEsop, nVars) = readPlaIntoESOP(pFileNameIn);

    std::vector<Vec_Wec_t*> vEsopList;
    vEsopList.push_back(vEsop);
    
    abctime clk = Abc_Clock();

    for(int i = 0; i < fIteration; ++i)
    {
        std::vector<Vec_Wec_t*> vEsopList_tem;

        splitESOPList(vEsopList, vEsopList_tem, nVars, fGroupSize);
        
        freeESOPList(vEsopList);
        vEsopList.clear();

        for(Vec_Wec_t* esop: vEsopList_tem)
            vEsopList.push_back(MyExorcism(esop, nVars));

    }
    
	double runtime = static_cast<double>(Abc_Clock() - clk)/CLOCKS_PER_SEC;
    std::cout << "Time used: " << runtime << " sec" << std::endl;
    std::cout << "Number of cubes: " << countNumberOfCubes(vEsopList) << std::endl;

    if(pFileNameOut)
        writeESOPListIntoPla(vEsopList, pFileNameOut, nVars);
    return;
}
