#include "base/main/main.h"
#include "base/main/mainInt.h"

#include <iostream>
#include <fstream>

extern void MintEsopMain(Abc_Obj_t* pNode, std::ofstream& OutFile);
extern int NtkXorBidecMain(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis, int fOutput);
extern void BidecEsopMain(Abc_Ntk_t* pNtk, int fOutput);
extern void AigPSDKROMain(Abc_Ntk_t* pNtk);
extern void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);
extern void ArExtractMain(Abc_Ntk_t* pNtk, char* filename, int fLevel, int fRefine, int fVerbose);
extern void DcExtractMain(Abc_Ntk_t* pNtk, int fNumCofVar, int fVerbose, char* filename);

extern void CleanUnusedPi(Abc_Ntk_t* pNtk);
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
  
  return 0;

usage:
  Abc_Print(-2, "usage: bidecesop [-h] [-o ith] \n");
  Abc_Print(-2, "\t        for each PO, recursively do xor bidecomposition and synthesis esop\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  Abc_Print(-2, "\t-o ith   : specify the PO to be processed \n");
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

  Synopsis    [BDD Extract command function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int EsopSyn_CommandBddExtract(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  Abc_Obj_t* pPo;
  int iPo;
  int c;
  int fOutput = -1;
  int fVerbose = 0;
  int fLUT = 0;
  char* pFileNameOut = NULL;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hovu")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
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
      case 'v':
        if ( globalUtilOptind >= argc ){
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

  if ( argc == globalUtilOptind + 1 ){
      pFileNameOut = argv[globalUtilOptind];
  }

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(fLUT)
  {
    Abc_NtkForEachNode( pNtk, pPo, iPo)
    {
      if(Abc_ObjIsPi(pPo) || Abc_ObjIsPo(pPo)) continue;
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateFromNode(pNtk, pPo);
      std::cout << "--------Obj[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
      std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

      BddExtractMain(pSubNtk, pFileNameOut, fVerbose);
      Abc_NtkDelete(pSubNtk);
    }
  }
  else 
  {
    Abc_NtkForEachPo(pNtk, pPo, iPo)
    {
      if(fOutput != -1 && iPo != fOutput) continue;

      // create cone for the current PO
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

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
int EsopSyn_CommandArExtract(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  Abc_Obj_t* pPo;
  int iPo;
  int c;
  int fOutput = -1;
  int fVerbose = 0;
  int fRefine = 1;
  int fLevel = 1;
  int fLUT = 0;
  char* pFileNameOut = NULL;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "holvru")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
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
      case 'l':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-l\" should be followed by an integer.\n" );
            goto usage;
        }
        fLevel = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        if ( fLevel < 0 )
            goto usage;
        break;
      case 'v':
        if ( globalUtilOptind >= argc ){
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

  if ( argc == globalUtilOptind + 1 ){
      pFileNameOut = argv[globalUtilOptind];
  }

  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }

  if(fLUT)
  {
    Abc_NtkForEachNode( pNtk, pPo, iPo)
    {
      if(Abc_ObjIsPi(pPo) || Abc_ObjIsPo(pPo)) continue;
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateFromNode(pNtk, pPo);
      std::cout << "--------Obj[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
      std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

      ArExtractMain(pSubNtk, pFileNameOut, fLevel, fRefine, fVerbose);
      Abc_NtkDelete(pSubNtk);
    }
  }
  else
  {
    Abc_NtkForEachPo(pNtk, pPo, iPo){
      if(fOutput != -1 && iPo != fOutput) continue;

      // create cone for the current PO
      Abc_Ntk_t* pSubNtk = Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);

      if( Abc_ObjFaninC0(pPo) )
        Abc_ObjXorFaninC( Abc_NtkPo(pSubNtk, 0), 0 );

      std::cout << "--------PO[" << iPo << "] " << Abc_ObjName(Abc_NtkPo(pSubNtk, 0)) << "--------" << std::endl;
      std::cout << "numPI: " << Abc_NtkPiNum(pSubNtk) << std::endl;

      ArExtractMain(pSubNtk, pFileNameOut, fLevel, fRefine, fVerbose);

      Abc_NtkDelete(pSubNtk);
    }
  }
  return 0;

usage:
  Abc_Print(-2, "usage: arextract [-h][-l <level>] [-o <ith PO>] [-v [0/1]] [-ru]\n");
  Abc_Print(-2, "\t        synthesis ESOP with ArExtract\n");
  Abc_Print(-2, "\t-o    : specify the output to be processed\n");
  Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
  Abc_Print(-2, "\t-l    : specify the level of cost function. Default: 1\n");
  Abc_Print(-2, "\t-r    : toggle refinement or not. Default: 0\n");
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
int EsopSyn_CommandDcExtract(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  Abc_Obj_t* pPo;
  int iPo;
  int c;
  int fOutput = -1;
  int fVerbose = 0;
  int fNumCofVar = 8;
  char* pFileNameOut = NULL;

  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "hovn")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
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
      case 'v':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-v\" should be followed by an integer.\n" );
            goto usage;
        }
        fVerbose = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        if ( fVerbose < 0  || fVerbose > 1)
            goto usage;
        break;
      case 'n':
        if ( globalUtilOptind >= argc ){
            Abc_Print( -1, "Command line switch \"-o\" should be followed by an integer.\n" );
            goto usage;
        }
        fNumCofVar = atoi(argv[globalUtilOptind]);
        globalUtilOptind++;
        if ( fNumCofVar < 0 )
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
  return 0;

usage:
  Abc_Print(-2, "usage: dcextract [-h] [-v <0/1>] [-n <int>] [-o <int>]\n");
  Abc_Print(-2, "\t        synthesis ESOP with DC extract\n");
  Abc_Print(-2, "\t-n    : specify the number of variable to be cofactored. Default: 8\n");
  Abc_Print(-2, "\t-v    : specify the level of verbose. Default: 0\n");
  Abc_Print(-2, "\t-o    : specify the output to be processed. Default: All outputs\n");
  Abc_Print(-2, "\t-h    : print the command usage.\n");
  return 1;
  
}
// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "esopsyn", "mintesop", EsopSyn_CommandMintEsop, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "xorbidec", EsopSyn_CommandXorBidec, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bidecesop", EsopSyn_CommandBidecEsop, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "aigpsdkro", EsopSyn_CommandAigPSDKRO, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "bddextract", EsopSyn_CommandBddExtract, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "arextract", EsopSyn_CommandArExtract, 0);
    Cmd_CommandAdd( pAbc, "esopsyn", "dcextract", EsopSyn_CommandDcExtract, 0);
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
