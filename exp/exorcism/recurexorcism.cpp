#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<cassert>

int main()
{
    std::fstream inFile;
    inFile.open("esop.pla", std::ios::in);

    std::string buffer;
    inFile >> buffer; assert(buffer == ".i");

    std::cout << buffer << std::endl;
    

    int nVars = -1;
    inFile >> nVars;

    inFile >> buffer; assert(buffer == ".o");
    inFile >> buffer; assert(buffer == "1");
    inFile >> buffer; assert(buffer == ".type");
    inFile >> buffer; assert(buffer == "esop");

    std::vector<std::string> cubes;

    while(inFile >> buffer)
    {
        if(buffer == ".e") break;
        cubes.push_back(buffer);

        inFile >> buffer; assert(buffer == "1");
    }

    for(int i = 0; i < cubes.size(); ++i)
    {
        std::cout << cubes[i] << std::endl;
    }

    int group_idx = 0;
    int accu = 0;

    std::fstream outFile;
    outFile.open(std::to_string(group_idx)+".esop", std::ios::out);
    outFile << ".i " << nVars << '\n';
    outFile << ".o 1\n";
    outFile << ".type esop\n";

    int i = 0;
    while(i < cubes.size())
    {
        outFile << cubes[i] << " 1\n";
        accu++;

        if(accu == 3000)
        {
            outFile << ".e\n"; 
            outFile.close();
            group_idx++;
            accu = 0;
            outFile.open(std::to_string(group_idx)+".esop", std::ios::out);
            outFile << ".i " << nVars << '\n';
            outFile << ".o 1\n";
            outFile << ".type esop\n";
        }

        ++i;
    }

    return 0;
}
