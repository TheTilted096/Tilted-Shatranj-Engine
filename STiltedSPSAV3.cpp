/*
3rd Generation of SPSA Tuner
Refitted for Object Design

TheTilted096
8-8-24

*/

#include "STiltedAutoLibV2.h"

class Tuner{
    public:
        Match* matchptr;
        EvalVars evals[2];
        int numParams;

        std::string valuefile;

        int gamesPerMatch;

        int* delta;
        double* theta;
        double* thetaPlus;
        double* thetaMinus;

        double* lowerBounds;
        double* upperBounds;

        double ak, ck;
        double a1, a2, c1, c2;
        double alpha, gamma;

        Tuner(int*, std::string*);

        void printValues();
        bool validate();
        void step(int);
        void tune(int);

        ~Tuner();
};

Tuner::Tuner(int* specs, std::string* fnames){
    //num games, max nodes, num threads
    //opening moves, begin output, max half moves, end report

    numParams = 0;
    valuefile = fnames[1];

    if (fnames[0] == "rfp"){
        numParams = 2;
    } else if (fnames[0] == "asp"){
        numParams = 5;
    }

    theta = new double[numParams];
    thetaPlus = new double[numParams];
    thetaMinus = new double[numParams];

    lowerBounds = new double[numParams];
    upperBounds = new double[numParams];

    delta = new int[numParams];

    if (fnames[0] == "rfp"){
        theta[0] = 3.0; theta[1] = 67.0;
        lowerBounds[0] = -10.0; lowerBounds[1] = 40.0;
        upperBounds[0] = 40.0; upperBounds[1] = 95.0;

        a1 = 2800.0; a2 = 1.0;
        c1 = 13.0; c2 = 10.0;

        evals[0] = {thetaPlus, nullptr};
        evals[1] = {thetaMinus, nullptr};
    } else if (fnames[0] == "asp"){
        theta[0] = 73.5; theta[1] = 120.0; theta[2] = 240.0; theta[3] = 480.0, theta[4] = 40000.0;

        thetaPlus[2] = 240.0; thetaPlus[3] = 480.0; thetaPlus[4] = 40000.0;
        thetaMinus[2] = 240.0; thetaMinus[3] = 480.0; thetaMinus[4] = 40000.0;

        numParams = 2;

        lowerBounds[0] = 50.0; lowerBounds[1] = 90.0;
        upperBounds[0] = 100.0; upperBounds[1] = 150.0;

        a1 = 4200.0; a2 = 1.0;
        c1 = 13.0; c2 = 10.0;

        evals[0] = {nullptr, thetaPlus};
        evals[1] =  {nullptr, thetaMinus};
    }
    
    alpha = -0.602;
    gamma = -0.101;
    gamesPerMatch = specs[0];
    matchptr = new Match(specs, specs + 3, evals, fnames[2]);
}

Tuner::~Tuner(){
    delete[] theta;
    delete[] thetaPlus;
    delete[] thetaMinus;
    delete[] delta;
    delete[] lowerBounds;
    delete[] upperBounds;
    delete matchptr;
}

void Tuner::printValues(){
    std::cout << "ak: " << ak << '\n';
    std::cout << "ck: " << ck << '\n';
    std::cout << "adjustValue: " << ak / ck << '\n';
    std::cout << "delta: ";

    for (int j = 0; j < numParams; j++){
        std::cout << delta[j] << "     ";
    }

    std::cout << "\ntheta: ";
    for (int j = 0; j < numParams; j++){
        std::cout << theta[j] << "     ";
    }

    std::cout << "\nthetaPlus: ";
    for (int j = 0; j < numParams; j++){
        std::cout << thetaPlus[j] << "     ";
    }

    std::cout << "\nthetaMinus: ";
    for (int j = 0; j < numParams; j++){
        std::cout << thetaMinus[j] << "     ";
    }

    std::cout << "\n\n";
}

bool Tuner::validate(){
    /*
    for (int i = 0; i < numParams; i++){
        if ((theta[i] > upperBounds[i]) or (theta[i] < lowerBounds[i])){
            return false;
        }
    }
    */
    return true;
}

void Tuner::step(int st){
    ak = a1 * pow(st + a2, alpha);
    ck = c1 * pow(st + c2, gamma);
    //ck = 10.0;

    for (int i = 0; i < numParams; i++){
        delta[i] = 1 - 2 * (rand() & 1);
        //delta[i] = 1;
        thetaPlus[i] = theta[i] + delta[i] * ck;
        thetaMinus[i] = theta[i] - delta[i] * ck;
    }

    printValues();
    
    int mf;
    std::cout << "Begin Forward Match\n";
    mf = matchptr->runMatch(true, false);
    std::cout << "Forward Match Score: " << mf / 2.0 << '\n';

    int mb;
    std::cout << "Begin Backwards Match\n";
    mb = matchptr->runMatch(false, false);
    std::cout << "Backward Match Score: " << mb / 2.0 << '\n';

    double margin = (mf + 2 * gamesPerMatch - mb) / (4.0 * gamesPerMatch);
    std::cout << "ThetaPlus Score: " << margin << '\n';

    std::cout << "\nResult Theta: ";
    for (int k = 0; k < numParams; k++){
        theta[k] += (margin - 0.5) * delta[k] * ak / ck;
        std::cout << theta[k] << "     ";
    }
}

void Tuner::tune(int steps){
    for (int i = 0; i < steps; i++){
        std::cout << "\n=== Iteration " << i + 1 << " ===\n";
        step(i);
        if (!validate()){
            std::cout << "\nBounds Exceeded\n";
            break;
        }
    }

    std::ofstream vf(valuefile, std::ios::app);
    for (int j = 0; j < numParams; j++){
        vf << theta[j] << "     ";
    }
    vf << '\n';
    vf.close();
}


int main(int argc, char** argv){
    srand(time(0));

    if (argc != 7){
        std::cout << "Correct Usage: ./sspsa numPairs maxNodes numThreads valueFile tuneParam numPasses\n";
        return 0;
    }

    //num games, max nodes, num threads
    //opening moves, begin output, max half moves

    int stuffs[7] = {stoi(std::string(argv[1])), stoi(std::string(argv[2])), 
        stoi(std::string(argv[3])), 10, 0, 140, 0};

    std::string fstuf[3] = {argv[5], argv[4], "fens.txt"};
    
    Tuner t(stuffs, fstuf);

    t.tune(stoi(std::string(argv[6])));

    return 0;
}