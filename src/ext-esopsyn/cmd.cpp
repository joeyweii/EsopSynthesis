#include "utils.h"

/**Function*************************************************************

  Synopsis    [Synthesis minterm esop.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandMintEsop(Abc_Frame_t* pAbc, int argc, char** argv) {
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    Abc_Ntk_t * pNtkBdd = NULL;
    Abc_Obj_t * pNode = NULL;
    char * pFileNameOut = NULL;
    std::ofstream OutFile;
    int c;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        switch ( c )
        {
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    assert ( argc == globalUtilOptind + 1 );

    pFileNameOut = argv[globalUtilOptind];
    OutFile.open(pFileNameOut, std::ios::out);
  

    if ( pNtk == NULL)
    {
      Abc_Print( -1, "Abc_CommandAbc9MintEsop(): Empty current network.\n" );
      return 0;
    }

    if ( !Abc_NtkIsLogic(pNtk) && !Abc_NtkIsStrash(pNtk) )
    {
        Abc_Print( -1, "Can only collapse a logic network or an AIG.\n" );
        return 1;
    }

    if ( Abc_NtkIsStrash(pNtk) )
        pNtkBdd = Abc_NtkCollapse( pNtk, ABC_INFINITY, 0, 1, 0, 0, 0);
    else
    {
        pNtk = Abc_NtkStrash( pNtk, 0, 0, 0 );
        pNtkBdd = Abc_NtkCollapse( pNtk, ABC_INFINITY, 0, 1, 0, 0, 0);
        Abc_NtkDelete( pNtk );
    }

    assert(Abc_NtkPoNum(pNtkBdd) == 1); 

    pNode = Abc_ObjFanin0( Abc_NtkPo(pNtkBdd, 0) );
    Abc_NtkToBdd(pNtkBdd);
    MintEsopMain(pNode, OutFile);

    return 0;

usage:
    Abc_Print( -2, "usage: mintesop  \n" );
    Abc_Print( -2, "                     synthesis the esop where all cubes are minterms\n" );
    Abc_Print( -2, "\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    [Modified exorcism command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandMyExorCism(Abc_Frame_t* pAbc, int argc, char** argv) {
    Abc_Ntk_t * pNtk = NULL;
    int c, nCubesMax = 20000;

    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "C" ) ) != EOF )
    {
        switch ( c )
        {
        case 'C':
            if ( globalUtilOptind >= argc )
            {
                Abc_Print( -1, "Command line switch \"-C\" should be followed by an integer.\n" );
                goto usage;
            }
            nCubesMax = atoi(argv[globalUtilOptind]);
            globalUtilOptind++;
            if ( nCubesMax < 0 )
                goto usage;
            break;
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }

    if(argc != globalUtilOptind)
    {
        Abc_Print( -1, "Abc_CommandAbc9MyExorcism(): Argument error.\n" );
        goto usage;
    }

    if ( pAbc->pNtkCur == NULL)
    {
      Abc_Print( -1, "Abc_CommandAbc9MyExorcism(): Empty current network.\n" );
      return 0;
    }

    pNtk = pAbc->pNtkCur;
    assert(pNtk != NULL);

    My_Exorcism(pNtk, nCubesMax);

    return 0;

usage:
    Abc_Print( -2, "usage: myexorcism [-C N] [file_in] \n" );
    Abc_Print( -2, "                     performs heuristic exclusive sum-of-project minimization\n" );
    Abc_Print( -2, "        -C N       : maximum number of cubes in starting cover [default = %d]\n", nCubesMax );
    Abc_Print( -2, "        [file_in]  : optional input file in ESOP-PLA format (otherwise current AIG is used)\n");
    Abc_Print( -2, "\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    [XorBidec command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandBidecEsop(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(!Abc_NtkIsStrash(pNtk)){
    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
  }

  BidecEsopMain(pNtk);

  Abc_NtkDelete(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: bidecesop [-h] \n");
  Abc_Print(-2, "\t        for each PO, recursively do xor bidecomposition and synthesis esop\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}

/**Function*************************************************************

  Synopsis    [XorBidec command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandXorBidec(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  int fPrintParti;
  int fSynthesis;

  fPrintParti = 0;
  fSynthesis = 0;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hps")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      case 'p':
        fPrintParti ^= 1;
        break;
      case 's':
        fSynthesis ^= 1;
        break;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(!Abc_NtkIsStrash(pNtk)){
    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
  }

  if(!Abc_NtkIsStrash(pNtk)){
    Abc_Print(-1, "Current network is not an AIG.\n");
    return 1;
  }

  NtkXorBidec(pNtk, fPrintParti, fSynthesis);
  Abc_NtkDelete(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: xorbidec [-h] [-p] [-s]\n");
  Abc_Print(-2, "\t        for each PO, print the xor bidecomposition result\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  Abc_Print(-2, "\t-p    : print the partition result\n");
  Abc_Print(-2, "\t-s    : synthesis fA, fB and print the result\n");
  return 1;
  
}

// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "esopsyn", "xorbidec", EsopSyn_CommandXorBidec, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "myexorcism", EsopSyn_CommandMyExorCism, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "mintesop", EsopSyn_CommandMintEsop, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bidecesop", EsopSyn_CommandBidecEsop, 0);
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