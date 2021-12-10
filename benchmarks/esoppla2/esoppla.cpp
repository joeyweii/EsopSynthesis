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
 ./esoppla <outfile> <#variable> <#cube> <#max_literals_per_cube> <random_seed>
********/
int numVar;
int numCube;
int maxLit;
int seed; 

void Const1Cube(vector<char>& C){
    C.clear();
    for(int i = 0; i < numVar; i++){
        C.push_back('-');
    }
}
void AddConst1Cube(vector<vector<char> >& Cs){
    vector<char> C;
    Const1Cube(C);
    Cs.push_back(C);
}

void ExpandCube(vector<char> C, vector<vector<char> >& Cs){
    AddConst1Cube(Cs);

    for(int i = 0; i < C.size(); i++){
        if(C[i] == '-') continue;
        assert(C[i] == '1' || C[i] == '0');

        bool phase = C[i] - '0';
        if(!phase){
            vector<vector<char> > tem;
            for(int k = 0; k < Cs.size(); k++){
                assert(Cs[k][i] == '-');
                tem.push_back(Cs[k]);
                Cs[k][i] = '1';
                tem.push_back(Cs[k]);
            }
            Cs = tem;
        }
        else{
            for(int k = 0; k < Cs.size(); k++){
                assert(Cs[k][i] == '-');
                Cs[k][i] = '1';
            }
        }
    }

}

int main(int argc, char* argv[]){
    assert(argc == 6);

    numVar = atoi(argv[2]);
    numCube = atoi(argv[3]);
    maxLit = atoi(argv[4]);
    seed = atoi(argv[5]);

    srand(seed);

    vector<int> Vars;
    for(int i = 0; i < numVar; i++){
        Vars.push_back(i);
    }

    vector<vector<char> > C_final;

    for(int cube = 0; cube < numCube; cube++){
        vector<char> C;
        for(int i = 0; i < numVar; i++)
            C.push_back('-');
       
        int numLit = (rand() % maxLit) + 1;

        vector<int> vars_selected;
        sample(Vars.begin(), Vars.end(), back_inserter(vars_selected), numLit, mt19937{random_device{}()});

        for(int i = 0; i < vars_selected.size(); i++){
            int var = vars_selected[i];
            bool phase = rand() % 2;
            if(phase) C[var] = '1';
            else C[var] = '0';
        }

        C_final.push_back(C);
    }
    ofstream OutFile;
    OutFile.open(argv[1], ios::out);

    OutFile << "# Number variables: " << numVar << endl;
    OutFile << "# Max literals: " << maxLit << endl;
    OutFile << "# Cube number: " << C_final.size() << endl;


    OutFile << ".i " << numVar << endl;
    OutFile << ".o 1" << endl;
    OutFile << ".type esop" << endl;
    for(int i = 0; i < C_final.size(); i++){
        for(int j = 0; j < C_final[i].size(); j++){
            OutFile << C_final[i][j];
        }
        OutFile << " 1" << endl;
    }   

    OutFile << ".e" << endl;

    


    return 0;
}