#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "memMeasure.h"

#include <iostream>
#include <fstream>
#include <climits>

extern int NtkXorBidecMain(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis, int fOutput);
extern void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);
extern void ArExtractMain(Abc_Ntk_t* pNtk, char* filename, int fLevel, int fBound, int fRefine, int fVerbose);
extern void DcExtractMain(Abc_Ntk_t* pNtk, int fNumCofVar, int fVerbose, char* filename);
extern void IsfExtractMain(Abc_Ntk_t* pNtk, int fVerbose, int fNaive, char* filename);

/**Function*************************************************************

  Synopsis    [XorBidec command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandXorBidec(Abc_Frame_t* pAbc, int argc, char** argv)
{
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    int c;
    int fPrintParti;
    int fSynthesis;
    int fOutput;

    fPrintParti = 0;
    fSynthesis = 0;
    fOutput = -1;
    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "hpso")) != EOF)
    {
        switch (c) 
        {
            case 'h':
                goto usage;
            case 'p':
                fPrintParti ^= 1;
                break;
            case 's':
                fSynthesis ^= 1;
                break;
            case 'o':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-C\" should be followed by an integer.\n" );
                    goto usage;
                }
                fOutput = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fOutput < 0 )
                    goto usage;
                break;
          default:
            goto usage;
        }
    }
    if (!pNtk)
    {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }

    if(fOutput != -1 && fOutput >= Abc_NtkPoNum(pNtk))
    {
        Abc_Print( -1, "PO's idx out of range.\n" );
        return 0;
    }

    if(!Abc_NtkIsStrash(pNtk))
        pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);

    NtkXorBidecMain(pNtk, fPrintParti, fSynthesis, fOutput);
    Abc_NtkDelete(pNtk);
    return 0;

usage:
    Abc_Print(-2, "usage: xorbidec [-h] [-p] [-s] [-o ith]\n");
    Abc_Print(-2, "\t        for each PO, print the xor bidecomposition result\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    Abc_Print(-2, "\t-p    : print the partition result\n");
    Abc_Print(-2, "\t-s    : synthesis fA, fB and print the result\n");
    Abc_Print(-2, "\t-o    : specify the PO to be processed\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [BDD Extract command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandBddExtract(Abc_Frame_t* pAbc, int argc, char** argv)
{
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Obj_t* pPo = nullptr;
    int iPo;
    int c;
    int fOutput = -1;
    int fVerbose = 0;
    int fLUT = 0;
    char* pFileNameOut = NULL;

    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "hovu")) != EOF)
    {
        switch (c)
        {
            case 'h':
                goto usage;
            case 'o':
                if ( globalUtilOptind >= argc)
                {
                    Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
                    goto usage;
                }
                fOutput = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fOutput < 0 )
                    goto usage;
                break;
            case 'v':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-v\" should be followed by an integer.\n" );
                    goto usage;
                }
                fVerbose = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fVerbose < 0  || fVerbose > 1)
                    goto usage;
                break;
            case 'u':
                fLUT ^= 1;
                break;
            default:
                goto usage;
        }
    }

    if ( argc == globalUtilOptind + 1 )
    {
        pFileNameOut = argv[globalUtilOptind];
    }

    if (!pNtk)
    {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }

    if(fLUT)
    {
        Abc_NtkForEachNode( pNtk, pPo, iPo)
        {
            if(Abc_ObjIsPi(pPo) || Abc_ObjIsPo(pPo)) continue;
            if(fOutput != -1 && iPo != fOutput) continue;

            Abc_Ntk_t* pSubNtk = Abc_NtkCreateFromNode(pNtk, pPo);

            if(!Abc_NtkIsStrash(pSubNtk))
                pSubNtk = Abc_NtkStrash(pSubNtk, 0, 0, 0 );

            std::cout << "--------Obj[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;
            
            BddExtractMain(pSubNtk, pFileNameOut, fVerbose);

            Abc_NtkDelete(pSubNtk);
        }
    }
    else 
    {
        if(!Abc_NtkIsStrash(pNtk))
            pNtk = Abc_NtkStrash(pNtk, 0, 0, 0 );

        Abc_NtkForEachPo(pNtk, pPo, iPo)
        {
            if(fOutput != -1 && iPo != fOutput) continue;

            // create cone for the current PO
            Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 1);

            if( Abc_ObjFaninC0(pPo) )
                Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

            std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

            BddExtractMain(pSubNtk, pFileNameOut, fVerbose);

            Abc_NtkDelete(pSubNtk);
        }
    }
    return 0;

usage:
    Abc_Print(-2, "usage: bddextract [-hl] [-o <ith PO>] [-v [0/1]]\n");
    Abc_Print(-2, "\t        synthesis ESOP with BDD extract\n");
    Abc_Print(-2, "\t-o    : specify the output to be processed\n");
    Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
    Abc_Print(-2, "\t-u    : toggle using LUT mapping. Default: 0\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [ArExtract command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandArExtract(Abc_Frame_t* pAbc, int argc, char** argv)
{
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Obj_t* pPo;
    int iPo;
    int c;
    int fOutput = -1;
    int fVerbose = 0;
    int fLevel = 4;
    int fBound = 300;
    int fRefine = 1;
    int fLUT = 0;
    char* pFileNameOut = NULL;

    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "holbvru")) != EOF)
    {
        switch (c)
        {
            case 'h':
                goto usage;
            case 'o':
                if ( globalUtilOptind >= argc)
                {
                    Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
                    goto usage;
                }
                fOutput = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fOutput < 0 )
                    goto usage;
                break;
            case 'l':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-l\" should be followed by an integer.\n" );
                    goto usage;
                }
                fLevel = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fLevel < 0 )
                    goto usage;
                break;
            case 'b':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-l\" should be followed by an integer.\n" );
                    goto usage;
                }
                fBound = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if (fBound < 0)
                    goto usage;
                break;
            case 'v':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-v\" should be followed by an integer.\n" );
                    goto usage;
                }
                fVerbose = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fVerbose < 0  || fVerbose > 1)
                    goto usage;
                break;
            case 'r':
                fRefine ^= 1;
                break;
            case 'u':
                fLUT ^= 1;
                break;
            default:
                goto usage;
        }
    }

    if ( argc == globalUtilOptind + 1 )
    {
        pFileNameOut = argv[globalUtilOptind];
    }

    if (!pNtk)
    {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }


    if(fLUT)
    {
        Abc_NtkForEachNode( pNtk, pPo, iPo)
        {
            if(Abc_ObjIsPi(pPo) || Abc_ObjIsPo(pPo)) continue;
            if(fOutput != -1 && iPo != fOutput) continue;

            Abc_Ntk_t* pSubNtk = Abc_NtkCreateFromNode(pNtk, pPo);

            if(!Abc_NtkIsStrash(pSubNtk))
                pSubNtk = Abc_NtkStrash(pSubNtk, 0, 0, 0 );

            std::cout << "--------Obj[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

            ArExtractMain(pSubNtk, pFileNameOut, fLevel, fBound, fRefine, fVerbose);
            Abc_NtkDelete(pSubNtk);
        }
    }
    else
    {
        if(!Abc_NtkIsStrash(pNtk))
            pNtk = Abc_NtkStrash(pNtk, 0, 0, 0 );

        Abc_NtkForEachPo(pNtk, pPo, iPo)
        {
            if(fOutput != -1 && iPo != fOutput) continue;

            // create cone for the current PO
            Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

            if( Abc_ObjFaninC0(pPo) )
                Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

            std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

            ArExtractMain(pSubNtk, pFileNameOut, fLevel, fBound, fRefine, fVerbose);

            Abc_NtkDelete(pSubNtk);
        }
    }
    return 0;

usage:
    Abc_Print(-2, "usage: arextract [-h][-l <level>] [-t <0/1/2>] [-b <bound>] [-o <ith PO>] [-v [0/1]] [-ru]\n");
    Abc_Print(-2, "\t        synthesis ESOP with ArExtract\n");
    Abc_Print(-2, "\t-o    : specify the output to be processed\n");
    Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
    Abc_Print(-2, "\t-l    : specify the level of cost function. Default: 4\n");
    Abc_Print(-2, "\t-b    : specify the bound (BDD size) of full/partial expansion. Default: 300\n");
    Abc_Print(-2, "\t-r    : toggle refinement or not. Default: 1\n");
    Abc_Print(-2, "\t-u    : toggle LUT synthesis or not. Default: 0\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

/**Function*************************************************************

  Synopsis    [DcExtract command function.]

  Description []

  SideEffects []

  SeeAlso     []

 ***********************************************************************/
int EsopSyn_CommandDcExtract(Abc_Frame_t* pAbc, int argc, char** argv)
{
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Obj_t* pPo;
    int iPo;
    int c;
    int fOutput = -1;
    int fLUT = 0;
    int fVerbose = 0;
    int fNumCofVar = 4;
    char* pFileNameOut = NULL;

    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "hovnu")) != EOF)
    {
        switch (c)
        {
            case 'h':
                goto usage;
            case 'o':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
                    goto usage;
                }
                fOutput = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fOutput < 0 )
                    goto usage;
                break;
            case 'v':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-v\" should be followed by an integer.\n" );
                    goto usage;
                }
                fVerbose = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fVerbose < 0  || fVerbose > 1)
                    goto usage;
                break;
            case 'n':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
                    goto usage;
                }
                fNumCofVar = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fNumCofVar < 0 )
                    goto usage;
                break;
            case 'u':
                fLUT ^= 1;
                break;
            default:
                goto usage;
        }
    }

    if ( argc == globalUtilOptind + 1 )
    {
        pFileNameOut = argv[globalUtilOptind];
    }

    if (!pNtk)
    {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }

    if(fLUT)
    {
        Abc_NtkForEachNode( pNtk, pPo, iPo)
        {
            if(Abc_ObjIsPi(pPo) || Abc_ObjIsPo(pPo)) continue;
            if(fOutput != -1 && iPo != fOutput) continue;

            Abc_Ntk_t* pSubNtk = Abc_NtkCreateFromNode(pNtk, pPo);

            if(!Abc_NtkIsStrash(pSubNtk))
                pSubNtk = Abc_NtkStrash(pSubNtk, 0, 0, 0 );

            std::cout << "--------Obj[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

            DcExtractMain(pSubNtk, fNumCofVar, fVerbose, pFileNameOut);
            Abc_NtkDelete(pSubNtk);
        }
    }
    else
    {
        if(!Abc_NtkIsStrash(pNtk))
            pNtk = Abc_NtkStrash(pNtk, 0, 0, 0 );

        Abc_NtkForEachPo(pNtk, pPo, iPo)
        {
            if(fOutput != -1 && iPo != fOutput) continue;

            // create cone for the current PO
            Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

            if( Abc_ObjFaninC0(pPo) )
                Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

            std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
            std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

            DcExtractMain(pSubNtk, fNumCofVar, fVerbose, pFileNameOut);

            Abc_NtkDelete(pSubNtk);
        }
    }
    return 0;

usage:
    Abc_Print(-2, "usage: dcextract [-h] [-v <0/1>] [-n <int>] [-o <int>] [-u]\n");
    Abc_Print(-2, "\t        synthesis ESOP with DC extract\n");
    Abc_Print(-2, "\t-n    : specify the number of variable to be cofactored. Default: 4\n");
    Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
    Abc_Print(-2, "\t-o    : specify the output to be processed. Default: All outputs\n");
    Abc_Print(-2, "\t-u    : toggle using LUT mapping. Default: 0\n");
    Abc_Print(-2, "\t-h    : print the command usage.\n");
    return 1;
}

int EsopSyn_CommandIsfExtract(Abc_Frame_t* pAbc, int argc, char** argv)
{
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    int c, fVerbose = 0, fNaive = 0;
    char* pFileNameOut = NULL;

    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "hvn")) != EOF)
    {
        switch (c)
        {
            case 'h':
                goto usage;
            case 'v':
                if ( globalUtilOptind >= argc )
                {
                    Abc_Print( -1, "Command line switch \"-v\" should be followed by an integer.\n" );
                    goto usage;
                }
                fVerbose = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( fVerbose < 0  || fVerbose > 1)
                    goto usage;
                break;
            case 'n':
                fNaive ^= 1;
                break;
            default:
                goto usage;
        }
    }

    if ( argc == globalUtilOptind + 1 )
    {
        pFileNameOut = argv[globalUtilOptind];
    }

    if (!pNtk)
    {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }

    if (Abc_NtkPoNum(pNtk) != 2)
    {
        Abc_Print(-1, "#PO != 2,\n");
        return 1;
    }

    if(!Abc_NtkIsStrash(pNtk))
        pNtk = Abc_NtkStrash(pNtk, 0, 0, 0 );

    IsfExtractMain(pNtk, fVerbose, fNaive, pFileNameOut);

    return 0;

usage:
    Abc_Print(-2, "usage: isfextract [-hn] [-v [0/1]]\n");
    Abc_Print(-2, "\t        synthesis ESOP for incompletely specified function\n");
    Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
    Abc_Print(-2, "\t-n    : toggle using naive careset of F2 (C0 + C1). Default: 0\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "esopsyn", "xorbidec", EsopSyn_CommandXorBidec, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bddextract", EsopSyn_CommandBddExtract, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "arextract", EsopSyn_CommandArExtract, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "dcextract", EsopSyn_CommandDcExtract, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "isfextract", EsopSyn_CommandIsfExtract, 0);
}

// called during ABC termination
void destroy(Abc_Frame_t* pAbc)
{
}

// this object should not be modified after the call to Abc_FrameAddInitializer
Abc_FrameInitializer_t frame_initializer = { init, destroy };

// register the initializer a constructor of a global object
// called before main (and ABC startup)
struct registrar
{
    registrar() 
    {
        Abc_FrameAddInitializer(&frame_initializer);
    }
} esopsyn_registrar;
