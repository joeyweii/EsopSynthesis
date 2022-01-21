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

    assert(Abc_NtkPoNum(pNtk) == 1);

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
    int fOutput = -1;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "Co" ) ) != EOF )
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

    if(fOutput != -1 && fOutput >= Abc_NtkPoNum(pNtk)){
      Abc_Print( -1, "PO's idx out of range.\n" );
      return 0;
    }

    MyExorcismMain(pNtk, nCubesMax, fOutput);

    return 0;

usage:
    Abc_Print( -2, "usage: myexorcism [-C N] [-o ith] [file_in] \n" );
    Abc_Print( -2, "                     performs heuristic exclusive sum-of-project minimization\n" );
    Abc_Print( -2, "        -C N       : maximum number of cubes in starting cover [default = %d]\n", nCubesMax );
    Abc_Print( -2, "        -o ith       : specify the PO to be processed\n");
    Abc_Print( -2, "        [file_in]  : optional input file in ESOP-PLA format (otherwise current AIG is used)\n");
    Abc_Print( -2, "\n" );
    return 1;
}

/**Function*************************************************************

  Synopsis    [BidecEsop command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandBidecEsop(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  int fOutput = -1;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "ho")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
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
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(fOutput != -1 && fOutput >= Abc_NtkPoNum(pNtk)){
    Abc_Print( -1, "PO's idx out of range.\n" );
    return 0;
  }

  if(!Abc_NtkIsStrash(pNtk)){
    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
  }

  BidecEsopMain(pNtk, fOutput);

  Abc_NtkDelete(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: bidecesop [-h] [-o ith] \n");
  Abc_Print(-2, "\t        for each PO, recursively do xor bidecomposition and synthesis esop\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  Abc_Print(-2, "\t-o ith   : specify the PO to be processed \n");
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
  int fOutput;

  fPrintParti = 0;
  fSynthesis = 0;
  fOutput = -1;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hpso")) != EOF) {
    switch (c) {
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
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(fOutput != -1 && fOutput >= Abc_NtkPoNum(pNtk)){
      Abc_Print( -1, "PO's idx out of range.\n" );
      return 0;
    }

  if(!Abc_NtkIsStrash(pNtk)){
    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
  }

  if(!Abc_NtkIsStrash(pNtk)){
    Abc_Print(-1, "Current network is not an AIG.\n");
    return 1;
  }

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

  Synopsis    [AIG PSDKRO command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandAigPSDKRO(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  abctime clk;

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

  assert(Abc_NtkPoNum(pNtk) == 1);

  if(!Abc_NtkIsStrash(pNtk)){
    pNtk = Abc_NtkStrash(pNtk, 0, 0, 0);
  }

  assert(Abc_NtkIsStrash(pNtk));

  clk = Abc_Clock();

  AigPSDKROMain(pNtk);
  
  Abc_PrintTime( 1, "Time used:", Abc_Clock() - clk );
  return 0;

usage:
  Abc_Print(-2, "usage: aigpsdkro [-h] \n");
  Abc_Print(-2, "\t        synthesis ESOP with aig psdkro extraction\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}

/**Function*************************************************************

  Synopsis    [BDD PSDKRO command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandBddPSDKRO(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  Abc_Ntk_t* pNtkBdd = NULL;
  Abc_Obj_t* pPo;
  int iPo;
  int c;
  int fOutput = -1;
  int type = 1;
  abctime clk;
  char* pFileNameOut = NULL;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hto")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      case 't':
        type = 0;
        break;
      case 'o':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
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

  if ( argc == globalUtilOptind + 1 ){
      pFileNameOut = argv[globalUtilOptind];
  }

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  Abc_NtkForEachPo(pNtk, pPo, iPo){
    if(fOutput != -1 && iPo != fOutput) continue;

    // create cone for the current PO
    Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

    if( Abc_ObjFaninC0(pPo) )
      Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

    std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
    std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

    clk = Abc_Clock();

    if ( Abc_NtkIsStrash(pSubNtk) )
        pNtkBdd = Abc_NtkCollapse( pSubNtk, 1000000000, 0, 0, 0, 0, 0);
    else{
        Abc_Ntk_t* pStrNtk;
        pStrNtk = Abc_NtkStrash( pSubNtk, 0, 0, 0 );
        pNtkBdd = Abc_NtkCollapse( pStrNtk, 1000000000, 0, 0, 0, 0, 0);
        Abc_NtkDelete( pStrNtk );
    }

    
    Abc_PrintTime( 1, "BDD construction time used:", Abc_Clock() - clk );
  
    clk = Abc_Clock();

    BddPSDKROMain(pNtkBdd, pFileNameOut, type);
    
    Abc_PrintTime( 1, "PSDKRO time used:", Abc_Clock() - clk );

    Abc_NtkDelete(pSubNtk);
    Abc_NtkDelete(pNtkBdd);

  }
  return 0;

usage:
  Abc_Print(-2, "usage: bddpsdkro [-ht] [-o <ith PO>] [output_file] \n");
  Abc_Print(-2, "\t        synthesis ESOP with BDD extract or Pruned extract\n");
  Abc_Print(-2, "\t-t    : toggle the type of extraction (BDD / Prunded) default: Pruned\n");
  Abc_Print(-2, "\t-o    : specify the output to be processed\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}

/**Function*************************************************************

  Synopsis    [Scalable PSDKRO command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandScalablePSDKRO(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  Abc_Obj_t* pPo;
  int iPo;
  int numCof = 0;
  int fOutput = -1;
  int c;
  abctime clk;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hco")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      case 'c':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-c\" should be followed by an integer.\n" );
            goto usage;
        }
        numCof = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        if ( numCof < 0 )
            goto usage;
        break;
      case 'o':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
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

  std::cout << "numCof: " << numCof << std::endl;

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  Abc_NtkForEachPo(pNtk, pPo, iPo){
    if(fOutput != -1 && iPo != fOutput) continue;
    // create cone for the current PO
    Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

    if( Abc_ObjFaninC0(pPo) )
      Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

    std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
    std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

    if (!Abc_NtkIsStrash(pSubNtk) ){
        Abc_Ntk_t* trashNtk;
        pSubNtk = Abc_NtkStrash( trashNtk = pSubNtk, 0, 0, 0 );
        Abc_NtkDelete( trashNtk );
    }

    clk = Abc_Clock();

    ScalablePSDKROMain(pSubNtk, numCof);

    Abc_PrintTime( 1, "Scalable PSDKRO time used:", Abc_Clock() - clk );

  }
  return 0;

usage:
  Abc_Print(-2, "usage: scalpsdkro [-h] \n");
  Abc_Print(-2, "\t        synthesis ESOP with scalable psdkro extraction\n");
  Abc_Print(-2, "\t-c    : how many variables to be cofactored\n");
  Abc_Print(-2, "\t-o    : specidy the output to be processed\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}

// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "esopsyn", "xorbidec", EsopSyn_CommandXorBidec, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "myexorcism", EsopSyn_CommandMyExorCism, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "mintesop", EsopSyn_CommandMintEsop, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bidecesop", EsopSyn_CommandBidecEsop, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "aigpsdkro", EsopSyn_CommandAigPSDKRO, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bddpsdkro", EsopSyn_CommandBddPSDKRO, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "scalpsdkro", EsopSyn_CommandScalablePSDKRO, 0);
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