# ESOP synthesis research
## Description
### ESOP synthesis algorithm
1. bddextract: Pseudo-Kronecker expression (a special class of ESOP) synthesis algorithm based on [R. Drechsler, IEEE Trans. C 48(9), 1999, 987–990].
2. arextract: Optimize bddextract using approximation and refinement.

### Random Benchmark Generator
1. random_esop: generate a random esop in pla format.
2. random_function: generate a random function, with random output value for each assignment, in pla format. 
3. random_sop: generate a random sop, with 0.4n~n literals in each cube, in pla format.

### ESOP verification
1. verify_pla: verify if an ESOP realize a spec function provided by pla format.
2. verify_tt: verify if an ESOP realize a spec function provided by truth table format.

## Compile
### ABC
> make

### Random Benchmark Generator
> ./compile.sh

### easy library for verification
> cd easy
> mkdir build
> cd build
> cmake \.\.
> make

## Usage
### ESOP synthesis algorithm  (ABC command)
1. bddextract
> ./abc
> read <circuit/function file>
> bddextract [-hl] [-o <ith PO>] [-v [0/1]]
	-o    : specify the output to be processed
	-v    : specify the level of verbose. Default: 0
	-u    : toggle using LUT mapping. Default: 0
	-h    : print the command usage

2. arextract
> ./abc
> read <circuit/function file>
> arextract [-h][-l <level>] [-o <ith PO>] [-v [0/1]]
	-o    : specify the output to be processed
	-v    : specify the level of verbose. Default: 0
	-l    : specify the level of cost function. Default: 1
	-h    : print the command usage

### Random Benchmark Generator
1. random_esop
./random_esop <out.pla> <#inVar> <#cube> <#max_literals_per_cube> <random_seed>

2. random function
./random_function <out.pla> <#inVar> <#outVar> <random seed>

3. random sop
 ./random_sop <out.pla> <#inVar> <#cube> <random_seed>

### ESOP verification
1. verify_pla
./easy/build/verify/verify_pla <function.pla> <esop.pla>

2. verify_tt
./easy/build/verify/verify_tt <truth_table> <esop.pla>

## Reference
[easy library](https://github.com/hriener/easy)

[Rolf Drechsler. 1999. Pseudo-Kronecker expressions for symmetric functions. IEEE Transactions on Computers 48, 9 (1999), 987–990.](https://people.eecs.berkeley.edu/~alanmi/publications/other/tc99_drechsler.pdf)

[ABC: System for Sequential Logic Synthesis and Formal Verification](https://github.com/berkeley-abc/abc)
    
[BDD extract implementation](https://github.com/boschmitt/losys)
