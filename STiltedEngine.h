/*
Class Definition for the Engine Class
Contains Main Search Routine and Parameters

Inheirits from Position because
an Engine can be thought of as a position that
tries to improve itself.

7-15-2024, TheTilted096;

*/

#include "STiltedPosition.h"

struct EvalVars{
    double* rc;
    double* aw;
};

class TTentry{
    public:
        int eScore, enType, eDepth;
        uint64_t eHash;
        Move eMove;
        uint64_t* zref;

        TTentry();
        void setReference(uint64_t*);
        void update(int&, int, int&, Move, int);
        void reset();
        void print(std::ostream&);

};

class Engine : public Position{
    protected:
        Move bestMove;
        uint64_t nodes, mnodes;

        std::chrono::_V2::steady_clock::time_point moment;
        int64_t thinkLimit;
        bool timeKept;

        Move killers[64][2];

        TTentry* ttable;

    public:
        double rfpCoef[2] = {3.0, 67.0};
        double aspWins[5] = {73.5, 120.0, 240.0, 480.0, 40000.0};

        int lmrReduces[64][128]; 
        double lmrCoef[2] = {-0.1, 0.4};

        Engine();

        void copyEval(EvalVars);

        void showZobrist(std::ostream&);
        void eraseTransposeTable();
        int alphabeta(int, int, int, int, bool);
        int quiesce(int, int, int);

        Move getMove();
        void newGame();

        void endHandle();
        int search(uint32_t, int, uint64_t, bool);

        ~Engine();
};