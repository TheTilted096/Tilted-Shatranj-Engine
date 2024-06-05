/*
Second Version of SPSA Tuner
TheTilted096
5-31-2024

*/

#include "STiltedAutoLib.cpp"

int main(int argc, char** argv){
    srand(time(0));
    initializeTables();

    if (argc != 6){
        std::cout << "Correct Usage: ./sspsa numPasses numPairs maxNodes numThreads outputFile\n";
        return 0;
    }

    std::cout << "numPasses: " << argv[1] << '\n';
    std::cout << "numPairs: " << argv[2] << '\n';
    std::cout << "maxNodes: " << argv[3] << '\n';
    std::cout << "numThreads: " << argv[4] << '\n';
    std::cout << "outputFile: " << argv[5] << '\n';

    int delta[2];
    int theta[2] = {0, 80};
    int thetaPlus[2];
    int thetaMinus[2];
    EvalVars vars[] = {{thetaPlus, nullptr}, {thetaMinus, nullptr}};

    std::string output = argv[5];
    int numPasses = stoi(std::string(argv[1]));

    //params: #games, #num moves, mnodes, lb, ub, #threads, start ID
    int testParams[7] = {stoi((std::string) argv[2]), 10, stoi((std::string) argv[3]), 0, 0, stoi((std::string) argv[4]), 1};

    for (int k = 0; k < numPasses; k++){
        std::cout << "\n=== PASS " << k + 1 << " ===\n";

        int ak = std::floor(513.0 * pow(k + 44.0, -0.602));
        int ck = std::floor(29.6 * pow(k + 4.6, -0.101));
        
        std::cout << "ak: " << ak << '\n';
        std::cout << "ck: " << ck << '\n';
        std::cout << "delta: ";

        for (int pp = 0; pp < 2; pp++){
            delta[pp] = 1 - 2 * (rand() % 2);
            std::cout << delta[pp] << "    ";
            thetaPlus[pp] = theta[pp] + delta[pp] * ck;
            thetaMinus[pp] = theta[pp] - delta[pp] * ck;
        }

        std::cout << '\n';
        int mar = 2 * testParams[0] - matchPairs(output, testParams, vars, false);
        std::cout << "ThetaPlus Score = " << mar << '\n';
        

        for (int pp = 0; pp < 2; pp++){
            theta[pp] += mar * delta[pp] * ak / ck;
            std::cout << theta[pp] << "     ";
        }        
    }



    return 0;
}