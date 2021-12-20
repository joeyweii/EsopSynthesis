#include<iostream>
#include<vector>
#include<cassert>
#include<fstream>
#include<string>
#include<cstdlib>
#include<algorithm>
#include<random>
#include<sstream>

#define DEBUG 0
using namespace std;

/********* Usage
./p2v <input_esoppla> <output_verilog>
********/

int numPI = -1;
int numPO = -1;

int main(int argc, char* argv[]){
    ifstream InFile;
    assert(argc == 3);
    InFile.open(argv[1], ios::in);


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
    assert(Esop.size() > 0);
    
    if(DEBUG){
        std::cout << "numPI: " << numPI << std::endl;
        std::cout << "numPO: " << numPO << std::endl;
        for(int i = 0; i < Esop.size(); i++){
            std::cout << Esop[i] << " 1" << std::endl;
        }
    }

    ofstream OutFile;
    OutFile.open(argv[2], ios::out);
    if(!OutFile.is_open()) std::cout << "Error occured when opening the output file." << std::endl;
    else{
        OutFile << "module top(";
        for(int i = 0; i < numPI; i++){
            OutFile << "x" << i << ", ";
        }

        assert(numPO == 1);
        OutFile << " o);" << std::endl;

        OutFile << "input ";
        for(int i = 0; i < numPI; i++){
            OutFile << "x" << i;
            if(i == numPI - 1) OutFile << ";" << std::endl;
            else OutFile << ", ";
        }

        OutFile << "output o;" << std::endl;
        OutFile << "wire ";

        for(int i = 0; i < numPI; i++){
            OutFile << "x" << i << "_c";
            if(i == numPI - 1) OutFile << ";" << std::endl;
            else OutFile << ", ";
        }

        OutFile << "wire ";
        for(int i = 0; i < Esop.size(); i++){
            OutFile << "w" << i;
            if(i == Esop.size() - 1) OutFile << ";" << std::endl;
            else OutFile << ", ";

        }

        for(int i = 0; i < numPI; i++){
            OutFile << "assign x" << i << "_c = ~x" << i << ";" << std::endl;
        }

        for(int i = 0; i < Esop.size(); i++){
            assert(Esop[i].size() == numPI);
            int cnt = 0;
            for(int j = 0; j < numPI; j++){
                if(Esop[i][j] != '-'){
                    cnt += 1;
                }
            }

            assert(cnt > 0);
            if(cnt == 1){
                OutFile << "assign w" << i << " = x";
                for(int j = 0; j < numPI; j++){
                    if(Esop[i][j] != '-'){
                        if(Esop[i][j] == '0') OutFile << j << "_c;" << std::endl;
                        else OutFile << j << ";" << std::endl;
                        break;
                    }
                }
            }
            else{
                OutFile << "and (w" << i;
                for(int j = 0; j < numPI; j++){
                    if(Esop[i][j] != '-'){
                        if(Esop[i][j] == '0') OutFile << ", " << "x" << j << "_c";
                        else OutFile << ", " << "x" << j;
                    }
                }
                OutFile << ");" << std::endl;
            }
        }

        OutFile << "xor (o";
        for(int i = 0; i < Esop.size(); i++){
            OutFile << ", w" << i;
        }
        OutFile << ");" << std::endl;
        OutFile << "endmodule" << std::endl;

        return 0; 

        
    }
    


    return 0;
}