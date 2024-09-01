/*
Second Version of Tuning Program fitted to OOP design
TheTilted096, 8-17-24

Will Finish When PSQT Tuning Again
*/

#include "STiltedEngine.h"

class Tuna{
    public:       
        Engine e;

        int posAn;

        double* endMins;
        double* endNeut;
        double* endPlus;
        double* midMins;
        double* midNeut;
        double* midPlus;
 
        Tuna();
        ~Tuna();

        double singleError(double);
        void updateMidAccs(int&, int, double);
        void updateEndAccs(int&, int, double);

        void updateOnBitboard(int, bool, double, double);
        void updateOnPosition(double);

        void updateMobs(double, double);
        
        int adjustValue(double, double, double);

        void passData(std::string);

        void emptyAccs();
        void updateParams();

        void reportNow(int);
};

Tuna::Tuna(){
    endMins = new double[427];
    endNeut = new double[427];
    endPlus = new double[427];
    midMins = new double[427];
    midNeut = new double[427];
    midPlus = new double[427];

    posAn = 0;
}

Tuna::~Tuna(){
    delete[] endMins;
    delete[] endNeut;
    delete[] endPlus;
    delete[] midMins;
    delete[] midNeut;
    delete[] midPlus;
}

double Tuna::singleError(double gr){
    double qi = e.preciseQuiesce(-30000, 30000, 0);
    qi *= (2 * e.getSide() - 1);
    //std::cout << qi << '\n';

    double sigm = 1.0 / (1.0 + pow(10.0, -qi/400.0));
    //std::cout << sigm << '\n';

    return pow(gr - sigm, 2.0);
}

void Tuna::updateMidAccs(int& elem, int ind, double gr){
    //std::cout << "MidMinus\n";
    //std::cout << "Updating Mid Accs\n";
    elem--;
    e.evaluateScratch();
    e.beginMobility(true); e.beginMobility(false);
    
    //std::cout << "Minus Eval: " << e.mobil[0] << '\t' << e.mobil[1] << '\n';
    midMins[ind] += singleError(gr);

    //std::cout << "MidPlus\n";
    elem += 2;
    e.evaluateScratch();
    e.beginMobility(true); e.beginMobility(false);
    //std::cout << "Plus Eval: " << e.mobil[0] << '\t' << e.mobil[1] << '\n';

    midPlus[ind] += singleError(gr);

    elem--;
}

void Tuna::updateEndAccs(int& elem, int ind, double gr){
    elem--;
    e.evaluateScratch();
    e.beginMobility(true); e.beginMobility(false);
    endMins[ind] += singleError(gr);

    elem += 2;
    e.evaluateScratch();
    e.beginMobility(true); e.beginMobility(false);
    endPlus[ind] += singleError(gr);

    elem--;
}

void Tuna::updateOnBitboard(int pstp, bool s, double gr, double nt){
    Bitboard mybb = e.sides[s] & e.pieces[pstp];
    int* bl = Bitboards::bitboardToList(mybb);

    uint8_t esq;

    for (int i = 0; i < bl[0]; i++){
        esq = bl[i + 1] ^ (56 * !s);
        midNeut[(pstp << 6) + esq] += nt;
        endNeut[(pstp << 6) + esq] += nt;

        updateMidAccs(e.mps[pstp][esq], (pstp << 6) + esq, gr);
        updateEndAccs(e.eps[pstp][esq], (pstp << 6) + esq, gr);
    }

    delete bl;
}

void Tuna::updateMobs(double gr, double nt){
    int* am = e.availableMobs();

    int efid;

    for (int i = 0; i < am[0]; i++){
        efid = am[i + 1] + 384;
        midNeut[efid] += nt;
        endNeut[efid] += nt;

        updateMidAccs(e.mobVals[am[i + 1]], efid, gr);
        updateEndAccs(e.mobValsE[am[i + 1]], efid, gr);
    }

    delete[] am;
}

void Tuna::updateOnPosition(double gr){
    e.evaluateScratch();
    e.beginMobility(true); e.beginMobility(false);
    double neutTerm = singleError(gr);

    for (int i = 0; i < 6; i++){
        updateOnBitboard(i, false, gr, neutTerm);
        updateOnBitboard(i, true, gr, neutTerm);
    }

    updateMobs(gr, neutTerm);

    posAn++;
}

int Tuna::adjustValue(double ad, double sb, double eq){
    double best = std::min(std::min(ad, eq), sb);
    return (best == ad) - (best == sb);
}

void Tuna::emptyAccs(){
    for (int i = 0; i < 427; i++){
        endMins[i] = 0.0;
        endNeut[i] = 0.0;
        endPlus[i] = 0.0;
        midMins[i] = 0.0;
        midNeut[i] = 0.0;
        midPlus[i] = 0.0;
    }
}

void Tuna::updateParams(){
    int zi = 0;
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            e.mps[i][j] += adjustValue(midPlus[zi], midMins[zi], midNeut[zi]);
            e.eps[i][j] += adjustValue(endPlus[zi], endMins[zi], endNeut[zi]);
            zi++;
        }
    }

    for (int k = 0; k < 43; k++){
        e.mobVals[k] += adjustValue(midPlus[zi], midMins[zi], midNeut[zi]);
        e.mobValsE[k] += adjustValue(endPlus[zi], endMins[zi], endNeut[zi]);
        zi++;
    }
}

void Tuna::passData(std::string fname){
    std::ifstream posfile(fname);
    
    double gameScore = -1;
    std::string line = "x";

    emptyAccs();

    while (line != ""){
        //std::cout << "started\n";
        getline(posfile, line);
        if (line == ""){
            break; //reached end of file
        }
        if (line.substr(0, 4) == "GAME"){
            //std::cout << "game marker found\n";
            continue;
        }
        if (line.substr(0, 8) == "StartPos"){
            //std::cout << "start pos found\n";
            continue;
        }
        if (line.substr(0, 8) == "FinalPos"){
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
        updateOnPosition(gameScore);

        if (posAn % 50000 == 0){
            std::cout << posAn / 1000 << "k    ";
        }
    }

    updateParams();
}

void Tuna::reportNow(int ps){
    std::string onm = "tuned" + std::to_string(ps) + ".txt";
    std::ofstream tunedFile(onm);

    tunedFile << "Values After Pass " << ps << "\n\n";

    tunedFile << "int mps[6][64] =\n{";

    for (int i = 0; i < 6; i++){
        tunedFile << "{";
        for (int j = 0; j < 63; j++){
            tunedFile << e.mps[i][j] - Position::matVals[i] << ", ";
            if (j % 8 == 7){
                tunedFile << '\n';
            }
        }
        tunedFile << e.mps[i][63] - Position::matVals[i];
        tunedFile << ((i < 5) ? "},\n\n" : "}};\n\n");
    }

    tunedFile << "int eps[6][64] =\n{";

    for (int i = 0; i < 6; i++){
        tunedFile << "{";
        for (int j = 0; j < 63; j++){
            tunedFile << e.eps[i][j] - Position::matVals[i] << ", ";
            if (j % 8 == 7){
                tunedFile << '\n';
            }
        }
        tunedFile << e.eps[i][63] - Position::matVals[i];
        tunedFile << ((i < 5) ? "},\n\n" : "}};\n\n");
    }

    tunedFile << "int mobVals[43] = \n{";

    for (int i = 0; i < 4; i++){
        for (int j = Position::mIndx[i]; j < Position::mIndx[i + 1]; j++){
            tunedFile << e.mobVals[j] << ", ";
        }
        tunedFile << '\n';
    }
    for (int i = Position::mIndx[4]; i < 42; i++){
        tunedFile << e.mobVals[i] << ", ";
    }
    tunedFile << e.mobVals[42] << "};\n\n";

    tunedFile << "int mobValsE[43] = \n{";

    for (int i = 0; i < 4; i++){
        for (int j = Position::mIndx[i]; j < Position::mIndx[i + 1]; j++){
            tunedFile << e.mobValsE[j] << ", ";
        }
        tunedFile << '\n';
    }
    for (int i = Position::mIndx[4]; i < 42; i++){
        tunedFile << e.mobValsE[i] << ", ";
    }
    tunedFile << e.mobValsE[42] << "};";
}

int main(int argc, char** argv){
    if (argc != 3){
        std::cout << "Correct Usage: ./stuna fileName numPass\n";
        return 0;
    }

    Tuna t;

    std::string dataname(argv[1]);
    int numPasses = std::stoi(std::string(argv[2]));

    for (int i = 0; i < numPasses; i++){
        std::cout << "\n=== PASS " << i << " ===\n";
        std::cout << "Positions Analyzed: ";
        t.passData(dataname);
        if (i % 5 == 0){
            t.reportNow(i);
        }
    }
    
    return 0;
}










