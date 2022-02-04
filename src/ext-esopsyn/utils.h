#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <climits>
#include <bitset>
#include <utility>
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include "aig/aig/aig.h"

enum Set{
  XC,
  XB,
  XA,
  XAB
};


typedef struct PSDKRONode PSDKRONode;
struct PSDKRONode{
    std::vector<std::string> Esop;
    int Cost;
};

namespace bddpsdkro{
	constexpr int bitwidth = 64;

	enum var_value : std::uint8_t {
		POSITIVE, // xi = 1
		NEGATIVE, // xi = 0
		UNUSED // xi don't care
	};

	enum exp_type : std::uint8_t {
		POSITIVE_DAVIO,
		NEGATIVE_DAVIO,
		SHANNON
	};

	struct cube{
		std::bitset<bitwidth> polarity;
		std::bitset<bitwidth> mask;

		cube(){
			polarity.reset();
			mask.reset();
		}

		std::string str(const std::uint32_t n_inputs) const
		{
			std::string s;
			for (auto i = 0; i < n_inputs; ++i) {
				if (!mask.test(i)) {
					s.push_back('-');
				} else if (polarity.test(i))
					s.push_back('1');
				else
					s.push_back('0');
			}
			return s;
		}
	};
}

extern "C" Aig_Man_t *  Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
extern "C" int Abc_ExorcismMain( Vec_Wec_t * vEsop, int nIns, int nOuts, char * pFileNameOut, int Quality, int Verbosity, int nCubesMax, int fUseQCost );
extern "C" Vec_Wec_t * Abc_ExorcismNtk2Esop( Abc_Ntk_t * pNtk );
extern "C" Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );
extern "C" int Abc_NtkDarCec( Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nConfLimit, int fPartition, int fVerbose );
extern "C" Gia_Man_t * Gia_ManFromAig( Aig_Man_t * p );
extern "C" void Abc_NodeShowBdd( Abc_Obj_t * pNode, int fCompl );

extern void MintEsopMain(Abc_Obj_t* pNode, std::ofstream& OutFile);
extern int NtkXorBidecMain(Abc_Ntk_t* pNtk, int fPrintParti, int fSynthesis, int fOutput);
extern void BidecEsopMain(Abc_Ntk_t* pNtk, int fOutput);
extern void AigPSDKROMain(Abc_Ntk_t* pNtk);
extern void BddExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);
extern void PrunedExtractMain(Abc_Ntk_t* pNtk, char* filename, int fVerbose);

extern int NtkXorBidecSingleOutput(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti);
extern int NtkXorBidecSynthesis(Abc_Ntk_t* pNtk, std::vector<enum Set>& vParti, Abc_Ntk_t*& fA, Abc_Ntk_t*& fB);
extern void PrintAig(Abc_Ntk_t* pNtk);
extern Abc_Ntk_t* AIGCOF(Abc_Ntk_t* pNtk, int var, int phase);
extern Abc_Ntk_t* AIGXOR(Abc_Ntk_t* pNtk1, Abc_Ntk_t* pNtk2);

