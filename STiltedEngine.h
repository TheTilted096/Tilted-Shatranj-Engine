/*
Class Definition of the Engine Class
Inheirits from Position for its attributes

Primarily a search class

TheTilted096, 7-19-24
*/


#include "STiltedPosition.h"

//Transposition Table Entry
class TTentry{
    public:
        int eScore;
        uint64_t eHash;
        int enType;
        int eDepth;
        uint32_t eMove;
        uint64_t* zref;

        TTentry();
        void setReference(uint64_t*);
        void update(int&, int, int&, uint32_t, int);
        void reset();
        void print();
};

struct EvalVars{
    double* rc;
    double* aw;
};

class Engine : public Position{
    public:
    Move bestMove;

    uint64_t mnodes;
    uint64_t nodesForever;

    TTentry* ttable;
    Move killers[64][2];
    
    //Move thisLine[64];

    bool timeKept;
    int64_t thinkLimit; //think time limit
    std::chrono::_V2::steady_clock::time_point moment;
    
    void endHandle();

    void eraseTransposeTable();
    void eraseKillers();

    void sortMoves(int, int);

    void pickMove(int, int, int);

    int alphabeta(int, int, int, int, bool);

        double rfpCoef[2] = {3.0, 67.0};
        double aspWins[5] = {73.5, 120.0, 240.0, 480.0, 100000.0};

        int lmrReduces[64][128]; 
        double lmrCoef[2] = {0.4, 0.4};

        double lmpCoef[2] = {10.0, 1.0};
        double fpCoef[2] = {0.0, 100.0};

        double seeVals[6] = {20000.0, 650.0, 400.0, 200.0, 150.0, 100.0};

        Engine();

        Move getMove();

        void showZobrist();
        void newGame();
        void copyEval(EvalVars);

        bool see(Move);
        int quiesce(int, int, int);
        double preciseQuiesce(double, double, int);
        int search(uint32_t, int, uint64_t, bool);

        void bench();

        ~Engine();

        bool special;
};

