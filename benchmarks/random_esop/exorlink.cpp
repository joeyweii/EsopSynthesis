#include<iostream>
#include<vector>
#include<cassert>
#include<fstream>
#include<string>
#include<cstdlib>
#include<algorithm>
#include<random>
#include<sstream>
#include<ctime>

#define DEBUG 0
using namespace std;

/********* Usage
./p2v <input_esoppla> <output_verilog> <#exlink>
********/

int numPI = -1;
int numPO = -1;
int numExlink = -1;

int main(int argc, char* argv[]){
    srand(time(NULL));
    ifstream InFile;
    assert(argc == 4);
    InFile.open(argv[1], ios::in);

    numExlink = atoi(argv[3]);


    std::vector< std::string > Esop;
    std::string line;
    if(!InFile.is_open()) std::cout << "Error occured when opening the input file." << std::endl;
    else{
        while(std::getline(InFile, line)){
            assert(line.size() > 0);
            if(line[0] == '#') continue;
            std::stringstream linestream(line);
            std::string data;
            while(std::getline(linestream, data, ' ')){
                if(data == ".i"){
                    std::getline(linestream, data, ' ');
                    numPI = stoi(data);
                }
                else if(data == ".o"){
                    std::getline(linestream, data, ' ');
                    numPO = stoi(data);
                }
                else if(data == ".type"){
                    std::getline(linestream, data, ' ');
                    assert(data == "esop");
                }
                else if(data == ".p"){
                    std::getline(linestream, data, ' ');
                    continue;
                }
                else if(data == ".e") continue;
                else{
                    assert(data.size() == numPI);
                    Esop.push_back(data);
                    std::getline(linestream, data, ' ');
                    assert(data == "1");
                }
            }
           
        }
    }

    assert(numPI != -1);
    assert(numPO != -1);
    assert(numExlink > 0);
    assert(Esop.size() > 0);
    
    if(DEBUG){
        std::cout << "numPI: " << numPI << std::endl;
        std::cout << "numPO: " << numPO << std::endl;
    }
    if(DEBUG){
        std::cout << "----Before----" << std::endl; 
        for(int i = 0; i < Esop.size(); i++){
            std::cout << Esop[i] << " 1" << std::endl;
        }
    }

    for(int i = 0; i < numExlink; i++){
        int c1 = rand()%Esop.size();
        int c2 = rand()%Esop.size();
        
        while(c2 == c1) 
            c2 = rand()%Esop.size();

        if(c1 > c2) std::swap(c1, c2);
        
        string s1 = Esop[c1];
        string s2 = Esop[c2];

        Esop.erase(Esop.begin()+c2);
        Esop.erase(Esop.begin()+c1);

        for(int l = 0; l < numPI; l++){
            if(s1[l] == s2[l]) continue;
            string snew;
            for(int k = 0; k < numPI; k++){
                if(k == l){
                    if(s1[k] == '-'){
                        snew += ((s2[k] == '0')? "1" : "0");
                    }
                    else if(s2[l] == '-'){
                        snew += ((s1[k] == '0')? "1" : "0");
                    }
                    else{
                        assert(s1[k] == '1' || s1[k] == '0');
                        assert(s2[k] == '1' || s2[k] == '0');

                        if(s1[k] == '1' && s2[k] == '0') snew += "-";
                        else if(s1[k] == '0' && s2[k] == '1') snew += "-";
                        else snew += "0";
                    }
                }
                else if(k < l){
                    snew += s1[k];
                }
                else{
                    snew += s2[k];
                }
            }
            Esop.push_back(snew);
        }
    }

    std::random_shuffle(Esop.begin(), Esop.end());

    if(DEBUG){
        std::cout << "----After----" << std::endl; 
        for(int i = 0; i < Esop.size(); i++){
            std::cout << Esop[i] << " 1" << std::endl;
        }
    }

    ofstream OutFile;
    OutFile.open(argv[2], ios::out);

    OutFile << "# numCube = " << Esop.size() << std::endl;
    OutFile << ".i " << numPI << std::endl;
    assert(numPO == 1);
    OutFile << ".o " << numPO << std::endl;
    OutFile << ".type esop" << std::endl;
    for(int i = 0; i < Esop.size(); i++){
        OutFile << Esop[i] << " 1" << std::endl;
    }
    OutFile << ".e" << std::endl;

    return 0;
}