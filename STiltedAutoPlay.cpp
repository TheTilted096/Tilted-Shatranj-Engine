/*
Simple Program For Engine to play vs itself
Data Generation

TheTilted096
8-6-2024

*/

#include "STiltedAutoLibV2.h"

int main(int argc, char* argv[]){
    srand(time(0));

    if (argc != 5){
        std::cout << "Correct Usage: ./sautoplay numGames maxNodes numThreads fileName\n";
        return 0;
    }

    std::cout << "numGames: " << argv[1] << '\n';
    std::cout << "maxNodes: " << argv[2] << '\n';
    std::cout << "numThreads: " << argv[3] << '\n';
    std::cout << "fileName: " << argv[4] << '\n';

    int gameSpecs[3] = {stoi(std::string(argv[1])), stoi(std::string(argv[2])), stoi(std::string(argv[3]))};
    int outputSpecs[3] = {10, 0, 140};
    
    //num games, max nodes, num threads
    //opening moves, begin output, max half moves

    Game::outFile.open("fun.txt");
    Game::outFile << "";
    Game::outFile.close();

    EvalVars dums[2] = {{nullptr, nullptr}, {nullptr, nullptr}};

    Match mat(gameSpecs, outputSpecs, dums, "fun.txt");

    mat.runMatch(true, true);

    

    return 0;
}