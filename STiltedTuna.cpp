/*
Second Attempt at a PSQT-Tuner

TheTilted096
5-9-2024


*/

#include "STiltedSearchEvalV2.cpp"
#include <iomanip>

double endgamePositive[6][64];
double endgameNegative[6][64];
double endgameNeutral[6][64];

double midgamePositive[6][64];
double midgameNegative[6][64];
double midgameNeutral[6][64];

double calcSingleError(double& gameResult){
    int qi = quiesce(-30000, 30000, 0);
    qi *= (2 * toMove - 1);
    double sigm = 1 / (1 + pow(10.0, -qi/(double) 400));

    return pow(gameResult - sigm, 2);
}

void updateMidgameAccs(int piece, int square, double& result){
    mps[piece][square]--;
    evaluateScratch();
    midgameNegative[piece][square] += calcSingleError(result);

    mps[piece][square] += 2;
    evaluateScratch();
    midgamePositive[piece][square] += calcSingleError(result);

    mps[piece][square]--;
}

void updateEndgameAccs(int piece, int square, double& result){
    eps[piece][square]--;
    evaluateScratch();
    endgameNegative[piece][square] += calcSingleError(result);

    eps[piece][square] += 2;
    evaluateScratch();
    endgamePositive[piece][square] += calcSingleError(result);

    eps[piece][square]--;
}

void updateOnPosition(double& result){
    uint64_t wtb, btb;
    int f, g;
    double neutralTerm = calcSingleError(result);
    for (int i = 0; i < 6; i++){ //all bb's
        wtb = pieces[i];
        //std::cout << "Board " << i << '\n';
        g = -1;
        while (wtb){
            f = __builtin_ctzll(wtb);
            g += (f + 1);

            midgameNeutral[i][56 ^ g] += neutralTerm;
            endgameNeutral[i][56 ^ g] += neutralTerm;
            //std::cout << "Calling UMA with piece type " << i << '\n';
            updateEndgameAccs(i, 56 ^ g, result);
            updateMidgameAccs(i, 56 ^ g, result);

            wtb >>= f;
            wtb >>= 1;
        }

        btb = pieces[i + 7];
        g = -1;
        while (btb){
            f = __builtin_ctzll(btb);
            g += (f + 1);

            midgameNeutral[i][g] += neutralTerm;
            endgameNeutral[i][g] += neutralTerm;
            updateEndgameAccs(i, g, result);
            updateMidgameAccs(i, g, result);

            btb >>= f;
            btb >>= 1;
        }
    }
}

int adjustValue(double plus, double neg, double neut){
    double best = std::min(std::min(plus, neut), neg);
    //std::cout << (best == plus) - (best == neg) << "     ";
    //std::cout << std::setprecision(25) << plus << "    " << neg << "    " << neut << '\n';
    return (best == plus) - (best == neg);
}

int main(int argc, char** argv){
    if (argc < 3){
        std::cout << "Missing Command Line Args";
        return 0;
    }
    initializeTables();

    std::ifstream positionHolder;
    int numRuns = stoi(std::string(argv[1]));
    std::string outFileName = std::string(argv[2]);

    std::cout << "numRuns: " << numRuns << '\n';
    std::cout << "outFileName: " << outFileName << '\n';

    std::string line;
    double gameScore;

    uint64_t posSeen = 0;


    for (int passes = 0; passes < numRuns; passes++){
        std::cout << "\nPass " << passes << '\n';
        std::cout << "Positions Analyzed: ";
        for (int ff = 0; ff < 6; ff++){
            for (int gg = 0; gg < 64; gg++){
                endgamePositive[ff][gg] = 0;
                endgameNeutral[ff][gg] = 0;
                endgameNegative[ff][gg] = 0;

                midgamePositive[ff][gg] = 0;
                midgameNeutral[ff][gg] = 0;
                midgameNegative[ff][gg] = 0;
            }
        }

        for (int i = 0; i < 8; i++){
            std::string holderName = "psqt";
            holderName += ((char) (0x41 + i));
            holderName += ".txt";

            positionHolder.open(holderName);

            while (true){
                getline(positionHolder, line);
                //std::cout << line << '\n';
                if (line == ""){
                    break; //reached end of file
                }
                if (line.substr(0, 4) == "GAME"){
                    //std::cout << "game marker found\n";
                    continue;
                }
                if (line.substr(0, 5) == "Final"){
                    //std::cout << "final pos found\n";
                    continue;
                }

                if (line.substr(0, 8) == "Result: "){
                    std::string rline = line.substr(8);
                    if (rline == "1"){
                        gameScore = 1;
                    } else if (rline == "0.5"){
                        gameScore = 0.5;
                    } else {
                        gameScore = 0;
                    }
                    //std::cout << "next result found\n";
                    continue;
                }

                readFen(line);
                evaluateScratch();
                updateOnPosition(gameScore);
                posSeen++;
                if (posSeen % 50000 == 0){
                    std::cout << posSeen/1000 << "k ";
                }
                nodes = 0;
            }
            positionHolder.close();
        }

        double superSum = 0;

        for (int i = 0; i < 8; i++){
            std::string holderName = "psqt";
            holderName += ((char) (0x41 + i));
            holderName += ".txt";

            positionHolder.open(holderName);

            while (true){
                getline(positionHolder, line);
                //std::cout << line << '\n';
                if (line == ""){
                    break; //reached end of file
                }
                if (line.substr(0, 4) == "GAME"){
                    //std::cout << "game marker found\n";
                    continue;
                }
                if (line.substr(0, 5) == "Final"){
                    //std::cout << "final pos found\n";
                    continue;
                }

                if (line.substr(0, 8) == "Result: "){
                    std::string rline = line.substr(8);
                    if (rline == "1"){
                        gameScore = 1;
                    } else if (rline == "0.5"){
                        gameScore = 0.5;
                    } else {
                        gameScore = 0;
                    }
                    //std::cout << "next result found\n";
                    continue;
                }

                readFen(line);
                evaluateScratch();
                superSum += calcSingleError(gameScore);
                nodes = 0;
            }

            positionHolder.close();
        }

        std::cout << "\nTotal Error From Pass: " << superSum << '\n';

        for (int u = 0; u < 6; u++){
            for (int v = 0; v < 64; v++){
                mps[u][v] += adjustValue(midgamePositive[u][v], midgameNegative[u][v], midgameNeutral[u][v]);
                eps[u][v] += adjustValue(endgamePositive[u][v], endgameNegative[u][v], endgameNeutral[u][v]);
            }
        }

        std::ofstream tunedFile(outFileName);

        tunedFile << "Pass " << passes << '\n';
        tunedFile << "int mps[6][64] = \n";
        int mvals[6] = {0, 650, 400, 200, 150, 100};
        for (int aa = 0; aa < 6; aa++){
            tunedFile << '{';
            for (int bb = 0; bb < 64; bb++){
                tunedFile << mps[aa][bb] - mvals[aa];
                if (bb == 63){
                    tunedFile << "},";
                } else {
                    tunedFile << ", ";
                }
                if (bb % 8 == 7){
                    tunedFile << '\n';
                }
            }
            tunedFile << "\n";
        }

        tunedFile << "\nEnd-Game PSQT\n";
        for (int aa = 0; aa < 6; aa++){
            tunedFile << '{';
            for (int bb = 0; bb < 64; bb++){
                tunedFile << eps[aa][bb] - mvals[aa];
                if (bb == 63){
                    tunedFile << "},";
                } else {
                    tunedFile << ", ";
                }
                if (bb % 8 == 7){
                    tunedFile << '\n';
                }
            }
            tunedFile << "\n";
        }

        tunedFile.close();

    }

    /*
    for (int ff = 0; ff < 6; ff++){
        for (int gg = 0; gg < 64; gg++){
            endgamePositive[ff][gg] = 0;
            endgameNeutral[ff][gg] = 0;
            endgameNegative[ff][gg] = 0;

            midgamePositive[ff][gg] = 0;
            midgameNeutral[ff][gg] = 0;
            midgameNegative[ff][gg] = 0;
        }
    }

    readFen("2b5/5q2/3k1ppp/1P1p3P/1r2nP2/5NP1/5r2/2BK2RR b - - 0 1");
    evaluateScratch();

    std::cout << "Initial QScore: " << quiesce(-30000, 30000, 0) * (2 * toMove - 1) << '\n';

    gameScore = 0;
    updateOnPosition(gameScore);

    for (int u = 0; u < 6; u++){
        for (int v = 0; v < 64; v++){
            //std::cout << "mps[" << u << "][" << v << "]: ";
            mps[u][v] += adjustValue(midgamePositive[u][v], midgameNegative[u][v], midgameNeutral[u][v]);
            //std::cout << "eps[" << u << "][" << v << "]: ";
            eps[u][v] += adjustValue(endgamePositive[u][v], endgameNegative[u][v], endgameNeutral[u][v]);
        }
    }
    */
    
    /*
    std::ofstream tunedFile(outFileName);

    tunedFile << "Pass " << passes << '\n';
    tunedFile << "Middle-Game PSQT\n";
    int mvals[6] = {0, 650, 400, 200, 150, 100};
    for (int aa = 0; aa < 6; aa++){
        for (int bb = 0; bb < 64; bb++){
            tunedFile << mps[aa][bb] - mvals[aa] << ", ";
            if (bb % 8 == 7){
                tunedFile << '\n';
            }
        }
        tunedFile << '\n';
    }

    tunedFile << "\nEnd-Game PSQT\n";
    for (int aa = 0; aa < 6; aa++){
        for (int bb = 0; bb < 64; bb++){
            tunedFile << eps[aa][bb] - mvals[aa] << ", ";
            if (bb % 8 == 7){
                tunedFile << '\n';
            }
        }
        tunedFile << '\n';
    }

    tunedFile.close();
    */

    return 0;
}