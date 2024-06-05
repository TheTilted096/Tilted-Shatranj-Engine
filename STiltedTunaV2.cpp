/*
Second Version of Tuning Program fitted to OOP design
TheTilted096, 5-30-2024

Will Finish When PSQT Tuning Again
*/

#include "STiltedSearchEvalV3.cpp"

class Tuna{
    Engine e;

    double endgamePositive[6][64];
    double endgameNegative[6][64];
    double endgameNeutral[6][64];

    double midgamePositive[6][64];
    double midgameNegative[6][64];
    double midgameNeutral[6][64];

    double calcSingleError(double&);
    void updateMidgameAccs(int, int, double&);
    void updateEndgameAccs(int, int, double&);

    void updateOnPosition(double&);
    int adjustValue(double, double, double);

    public:
        Tuna();
        void step(std::string);
        void report();
};

Tuna::Tuna(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            endgamePositive[i][j] = 0;
            endgameNegative[i][j] = 0;
            endgameNeutral[i][j] = 0;
            midgamePositive[i][j] = 0;
            midgameNegative[i][j] = 0;
            midgameNeutral[i][j] = 0;
        }
    }

    Engine e = Engine();
}

double Tuna::calcSingleError(double& gameResult){
    int qi = e.quiesce(-30000, 30000, 0);
    qi *= (2 * e.toMove - 1);
    double sigm = 1 / (1 + pow(10.0, -qi/(double) 400));

    return pow(gameResult - sigm, 2);
}

void Tuna::updateMidgameAccs(int piece, int square, double& result){
    e.mps[piece][square]--;
    e.evaluateScratch();
    midgameNegative[piece][square] += calcSingleError(result);

    e.mps[piece][square] += 2;
    e.evaluateScratch();
    midgamePositive[piece][square] += calcSingleError(result);

    e.mps[piece][square]--;
}

void Tuna::updateEndgameAccs(int piece, int square, double& result){
    e.eps[piece][square]--;
    e.evaluateScratch();
    endgameNegative[piece][square] += calcSingleError(result);

    e.eps[piece][square] += 2;
    e.evaluateScratch();
    endgamePositive[piece][square] += calcSingleError(result);

    e.eps[piece][square]--;
}

void Tuna::updateOnPosition(double& result){
    uint64_t wtb, btb;
    int f, g;
    double neutralTerm = calcSingleError(result);
    for (int i = 0; i < 6; i++){ //all bb's
        wtb = e.pieces[i];
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

        btb = e.pieces[i + 7];
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

int Tuna::adjustValue(double plus, double neg, double neut){
    double best = std::min(std::min(plus, neut), neg);
    //std::cout << (best == plus) - (best == neg) << "     ";
    //std::cout << std::setprecision(25) << plus << "    " << neg << "    " << neut << '\n';
    return (best == plus) - (best == neg);
}

void Tuna::step(std::string fenfile){
    double gameScore;
    uint64_t posSeen = 0;

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

    std::ifstream posFile(fenfile);
    std::string line;

    while (true){
        getline(posFile, line);
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

            e.readFen(line);
            e.evaluateScratch();
            updateOnPosition(gameScore);
            posSeen++;
            if (posSeen % 50000 == 0){
                std::cout << posSeen/1000 << "k ";
            }
        }
    }

int main(int argc, char** argv){


    return 0;
}

