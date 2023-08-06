## Introduction
This repository helps you reproduce the experiments in [1].

## Build Project
clone the project
```
$ git clone https://github.com/joeyweii/EsopSynthesis.git
```
compile abc
```
$ make
```
compile scripts
```
$ ./compile_scripts.sh
```

## Files Description
- ```benchmarks/EPFL_suite/*```: EPFL combinational benchmarks
- ```benchmarks/*.cpp```: benchmark scripts
- ```src/ext-esopsyn/*.cpp```: source codes of algorithms

## Algorithms
Algotithms are implemented into abc commands.
Source codes are at directory ```src/ext-esopsyn/```.
- ```bddextract```: re-implemention of the BDD-based ESOP extraction algorithm [2]
- ```rdextract```: BDD-based ESOP extraction with reduced-decomposition-tree exploration [1] (Section III)
- ```dcextract```: re-implementation of the BDD-based extraction algorithm with divide and conquer [3]
- ```dcmin```: divide-and-minimize algorithm for ESOPs [1] (Section V)
- ```isfextract```: BDD-based ESOP extraction algorithm for incompletely-specified functions [1] (Section IV) 


For example, to use the command ```bddextract```
```
$ ./abc
$ abc> read in.pla
$ abc> bddextract
```
To see the usage of the command ```bddextract```
```
$ abc> bddextract -h
```

## Figure 1 in [1]
Generate a random ESOP
``` 
$ ./random_esop <pla file> <num_vars> <num_cube> <seed>
```
Read ESOP into abc
```
$ ./p2v <pla file> <verilog file>
$ ./abc
$ abc> read <verilog file>
```
Get the real cost
```
$ abc> bddextract
```
To get the BDD size, use the function Cudd_DagSize(DdNode* f) and add it in 
bddExtract.cpp file.

Example:
To generate a 12-variables ESOP with 100 product terms and get its real cost & BDD size
```
./random_esop in.pla 12 100 0
./p2v in.pla in.v
./abc
$ abc> read in.v
$ abc> bddextract
```

## Table I & Table II
EPFL benchmarks are under directory ```benchmarks/EPFL_suite/```

Read a circuit and perform LUP mapping
```
$ ./abc
$ abc> read benchmarks/EPFL_suite/<circuit>.aig
$ abc> if -a -K <LUT size>
```

To extract esop on a specific LUT
``` 
$ abc> bddextract -u -o <LUT index>
$ abc> dcextract -u -o <LUT index>
$ abc> rdextract -u -o <LUT index>
```
PS: The LUT index may not start from 0.

Example:
To perform LUT mapping with LUT size 32 on adder circuit and extract esop for the LUT with LUT index 500 
```
$ ./abc
$ abc> read benchmarks/EPFL_suite/adder.aig
$ abc> if -a -K 32
$ abc> bddextract -u -o 500
```

## Table III
Generate a incompletely-specified random funtion (onset is specified by the first PO and don't careset is specified by the second PO)
```
$ ./random_function <pla file> <num_PIs> <num_POs> <prob_onset> <prob_dcset> <seed>
```
Read the imcompletely-specified function
```
$ ./abc
$ abc> read <pla file>
```
Extract the ESOP
```
$ abc> bddextract
$ abc> isfextract
```

Example:
To generate a imcompletely-specified function with 10 variables, onset probability 0.5 ,and don't careset probability 0.25, then extract the ESOP
```
$ ./random_function in.pla 10 2 0.5 0.25 0
$ ./abc
$ abc> read in.pla
$ abc> bddextract
```

## Table IV
Perform ESOP extraction on a specific PO of a circuit
```
$ ./abc
$ abc> ./read benchmarks/EPFL_suite/<circuit>.aig
$ abc> rdextract -o <PO index> <input pla file>
```

Perform minimization on the extracted ESOP
```
$ abc> &exorcism -C 100000 -Q 0 <input pla file> <output pla file>
$ abc> dcmin <input pla file> <output pla file>
```

Example:
To extract an initial ESOP of the PO asqrt50 of the circuit sqrt and minimize its extracted ESOP
```
$ ./abc
$ abc> read benchmarks/EPFL_suite/sqrt.aig
$ abc> rdextract -o 50 in.pla
$ abc> dcmin in.pla out.pla
```

## References
- [1] C.-Y. Wei and J.-H. R. Jiang. Don’t-Care Aware ESOP Extraction via Reduced Decomposition-Tree Exploration. In Proceedings of the the Design Automation Conference, 2023.
- [2] R. Drechsler. Pseudo-Kronecker Expressions for Symmetric Functions. IEEE Transactions on Computers, 48(9):987–990, 1999.
- [3] B. Schmitt et al. Scaling-up ESOP Synthesis for Quantum Compilation. In Proceedings of the IEEE International Symposium on Multiple-Valued Logic, pages 13–18, 2019.
