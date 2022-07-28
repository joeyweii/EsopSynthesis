#include<iostream>
#include<fstream>
#include<vector>
#include<cassert>
#include<algorithm>
#include<cstdlib>

using namespace std;

/***** Usage ***** 
./random_function <numPI> <out.pla> <random seed>
******************/

int pow(int num){
    int result = 1;
    for(int i = 0; i < num; i++)
        result *= 2;
    return result;
}

void i2minterm(int num, int numPI, string& minterm){
    for(int i = 0; i < numPI; i++){
        int bit = num%2;
        num = num / 2;
        minterm += (bit + '0');
    }

    reverse(minterm.begin(), minterm.end());
}

int main(int argc, char* argv[]){
    assert(argc == 4);

    int numPI = atoi(argv[1]);

    fstream f;
    f.open(argv[2], ios::out);
    f << ".i " << numPI << endl;
    f << ".o 1" << endl;

    int seed = atoi(argv[3]);
    srand(seed);

    int nMinterms = pow(numPI); 
    for(int i = 0; i < nMinterms; i++){
        if(rand()%2) continue;
        string minterm;
        i2minterm(i, numPI, minterm);
        f << minterm << " 1" << endl;
    }

    return 0;
}
