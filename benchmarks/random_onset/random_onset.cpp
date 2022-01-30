#include<iostream>
#include<vector>
#include<cassert>
#include<fstream>
#include<string>
#include<cstdlib>
#include<algorithm>
#include<random>

using namespace std;

/********* Usage
 ./random_onset <outfile> <#variable> <#cube> <random_seed>
********/

int numVar;
int numCube;
int seed; 

int main(int argc, char* argv[]){
    assert(argc == 5);

    numVar = atoi(argv[2]);
    numCube = atoi(argv[3]);
    seed = atoi(argv[4]);

    srand(seed);

    ofstream OutFile;
    OutFile.open(argv[1], ios::out);

    OutFile << "# Number variables: " << numVar << endl;
    OutFile << "# Cube number: " << numCube << endl;

    vector<int> Vars;
    for(int i = 0; i < numVar; i++){
        Vars.push_back(i);
    }

    OutFile << ".i " << numVar << endl;
    OutFile << ".o 1" << endl;
    for(int i = 0; i < numCube; i++){
        int numLit = numVar - (rand()%(int)(0.6*numVar));

        vector<int> vars_selected;
        sample(Vars.begin(), Vars.end(), back_inserter(vars_selected), numLit, mt19937{random_device{}()});

        string s;
        for(int j = 0; j < numVar; j++){
            s += '-';
        }

        for(int j = 0; j < vars_selected.size(); j++){
            s[vars_selected[j]] = rand()%2 + '0';
        }

        OutFile << s << " 1" << endl;
    }   

    OutFile << ".e" << endl;
    return 0;
}