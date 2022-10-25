#include<iostream>
#include<fstream>
#include<vector>
#include<cassert>
#include<algorithm>
#include<cstdlib>

using namespace std;

/***** Usage ***** 
./random_function <out.pla> <num_PI> <num_PO> <random seed>
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
    assert(argc == 5);

    int numPI = atoi(argv[2]);
    int numPO = atoi(argv[3]);

    fstream f;
    f.open(argv[1], ios::out);
    f << ".i " << numPI << endl;
    f << ".o " << numPO << endl;

    int seed = atoi(argv[4]);
    srand(seed);

    int nMinterms = pow(numPI); 
    for(int i = 0; i < nMinterms; i++){
        string minterm;
        i2minterm(i, numPI, minterm);
        bool outAllZero = true;
        string output;
        for(int j = 0; j < numPO; ++j)
        {   
            char outvalue = '0' + rand()%2;
            if(outvalue == '1') outAllZero = false;  
            output += outvalue;
        }
        if(outAllZero) continue;
        f << minterm << ' ' << output << '\n';
    }

    f << ".e";

    return 0;
}
