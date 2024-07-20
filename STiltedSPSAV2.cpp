/*
Second Version of SPSA Tuner
TheTilted096
5-31-2024

*/

#include "STiltedAutoLib.cpp"

int main(int argc, char** argv){
    srand(time(0));

    if (argc != 7){
        std::cout << "Correct Usage: ./sspsa numPasses numPairs maxNodes numThreads outputFile tuneParam\n";
        return 0;
    }

    std::cout << "numPasses: " << argv[1] << '\n';
    std::cout << "numPairs: " << argv[2] << '\n';
    std::cout << "maxNodes: " << argv[3] << '\n';
    std::cout << "numThreads: " << argv[4] << '\n';
    std::cout << "outputFile: " << argv[5] << '\n';
    std::cout << "tuneParam: " << argv[6] << '\n';

    double a1, a2, c1, c2;
    int npar;
    int* delta;
    double* theta;
    double* thetaPlus;
    double* thetaMinus;
    EvalVars* vars;

    double* llb;

    if (std::string(argv[6]) == "rfp"){
        a1 = 2800.0; a2 = 1.0;
        c1 = 13.0; c2 = 10.0;
        npar = 2;

        delta = new int[2];
        theta = new double[2]; theta[0] = 3.0, theta[1] = 67.0;
        thetaPlus = new double[2]; thetaMinus = new double[2];

        vars = new EvalVars[2];
        vars[0] = {thetaPlus, nullptr};
        vars[1] =  {thetaMinus, nullptr};
        llb = new double[2]; llb[0] = -20.0; llb[1] = 40.0;
    } else if (std::string(argv[6]) == "asp"){
        a1 = 4200.0; a2 = 1.0;
        c1 = 13.0; c2 = 10.0;
        npar = 2;
        delta = new int[2];
        theta = new double[2]; 
        theta[0] = 60.0, theta[1] = 120.0; //theta[2] = 240.0; theta[3] = 480.0;
        thetaPlus = new double[2]; thetaMinus = new double[2];

        vars = new EvalVars[2];
        vars[0] = {nullptr, thetaPlus};
        vars[1] =  {nullptr, thetaMinus};
        llb = new double[4];
        llb[0] = 40.0; llb[1] = 40.0;
        llb[2] = 40.0; llb[3] = 40.0;
    } else {
        std::cout << "Invalid Param\n";
        return 0;
    }

    double alp = -0.602, gam = -0.101;

    std::string output = argv[5];
    int numPasses = stoi(std::string(argv[1]));

    //params: #games, #num moves, mnodes, lb, ub, #threads, start ID
    int testParams[7] = {stoi((std::string) argv[2]), 10, stoi((std::string) argv[3]), 0, 0, stoi((std::string) argv[4]), 1};

    for (int k = 0; k < numPasses; k++){
        std::cout << "\n=== PASS " << k + 1 << " ===\n";

        //a1 = 2800, a2 = 1; c1 = 13, c2 = 10; alp = -0.602, gam = -0.101
        double ak = a1 * pow(k + a2, alp);
        double ck = c1 * pow(k + c2, gam);
        
        std::cout << "ak: " << ak << '\n';
        std::cout << "ck: " << ck << '\n';
        std::cout << "adjustValue: " << ak / ck << '\n';
        std::cout << "delta: ";

        for (int pp = 0; pp < npar; pp++){
            delta[pp] = 1 - 2 * (rand() % 2);
            std::cout << delta[pp] << "    ";
            thetaPlus[pp] = theta[pp] + delta[pp] * ck;
            thetaMinus[pp] = theta[pp] - delta[pp] * ck;
        }

        std::cout << '\n';
        double mar = matchPairs(output, testParams, vars, false) / (4.0 * testParams[0]);
        std::cout << "ThetaPlus Score = " << mar << '\n';
        std::cout << "Multiplier: " << (mar - 0.5) << '\n';
        
        for (int pp = 0; pp < npar; pp++){
            theta[pp] += (mar - 0.5) * delta[pp] * ak / ck;
            if (theta[pp] < llb[pp]){
                std::cout << "Lower Bound Hit\n";
                return 0;
            }
            if ((pp < npar - 1) and (theta[pp] > theta[pp + 1])){
                std::cout << "Inconsistent Values\n";
                return 0;
            }
            std::cout << theta[pp] << "     ";
        } 
    }

    std::ofstream valueLog("valuelog.txt", std::ios::app);
    for (int i = 0; i < npar; i++){
        valueLog << theta[i] << ' ';
    }
    valueLog << '\n';

    delete[] delta;
    delete[] theta; delete[] thetaPlus; delete[] thetaMinus;
    delete[] vars;
    delete[] llb;


    return 0;
}