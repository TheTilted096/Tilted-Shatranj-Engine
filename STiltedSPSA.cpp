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
    double theta[2] = {0.0, 80.0};
    double thetaPlus[2];
    double thetaMinus[2];
    EvalVars vars[] = {{thetaPlus, nullptr}, {thetaMinus, nullptr}};

    std::string output = argv[5];
    int numPasses = stoi(std::string(argv[1]));

    //params: #games, #num moves, mnodes, lb, ub, #threads, start ID
    int testParams[7] = {stoi((std::string) argv[2]), 10, stoi((std::string) argv[3]), 0, 0, stoi((std::string) argv[4]), 1};

    for (int k = 0; k < numPasses; k++){
        std::cout << "\n=== PASS " << k + 1 << " ===\n";

        double ak = 200.0 * pow(k + 4.0, -0.808);
        double ck = 25.0 * pow(k + 13.6, -0.101);
        
        std::cout << "ak: " << ak << '\n';
        std::cout << "ck: " << ck << '\n';
        std::cout << "adjustValue: " << ak / ck << '\n';
        std::cout << "delta: ";

        for (int pp = 0; pp < 2; pp++){
            delta[pp] = 1 - 2 * (rand() % 2);
            std::cout << delta[pp] << "    ";
            thetaPlus[pp] = theta[pp] + delta[pp] * ck;
            thetaMinus[pp] = theta[pp] - delta[pp] * ck;
        }

        std::cout << '\n';
        double mar = matchPairs(output, testParams, vars, false) / (4.0 * testParams[0]);
        std::cout << "ThetaPlus Score = " << mar << '\n';
        double mul = (mar - 0.5) * 80.0;
        std::cout << "Multiplier: " << mul << '\n';
        
        for (int pp = 0; pp < 2; pp++){
            theta[pp] += mul * delta[pp] * ak / ck;
            std::cout << theta[pp] << "     ";
        }        
    }

    return 0;
}