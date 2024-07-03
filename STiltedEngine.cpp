/*
Declaration and Methods of an Engine instance
TheTilted096, 5-25-2024
*/

#include "STiltedBase.cpp"

class Engine{
    uint64_t pieces[14];
    bool toMove;
    uint32_t bestMove;

    uint32_t moves[96][128];
    int mprior[96][128];
    uint64_t nodes, mnodes;

    int scores[2]; int eScores[2];
    int thm; int chm[1024];
    uint64_t zhist[1024];
    int inGamePhase;
    TTentry* ttable;
    uint32_t killers[64][2];
    int numKillers[64];

    int historyTable[2][6][64];

    int64_t thinkLimit; //think time limit
    std::chrono::_V2::steady_clock::time_point moment;
    
    uint64_t hyperbolaQuintessence(int&);

    int fullMoveGen(int, bool);
    void makeMove(uint32_t, bool, bool);
    bool isChecked();

    void endHandle();

    int evaluate();
    bool kingBare();
    bool isInteresting(uint32_t&, bool);
    void eraseTransposeTable();
    void eraseHistoryTable();
    int alphabeta(int, int, int, int, bool);

    public:
        double rfpCoef[2] = {3.0, 67.0};
        double aspWins[5] = {73.5, 120.0, 240.0, 480.0, 40000.0};

        int lmrReduces[64][128]; 
        double lmrCoef[2] = {-0.1, 0.4};

        int mps[6][64] = 
        {{-38, -55, -15, -5, -5, -15, -25, -35, 
        -55, -18, 15, 15, 15, 15, 5, -25, 
        -45, -8, 18, 11, 0, 25, 12, -34, 
        -40, -5, 4, 0, 0, 22, 25, -10, 
        -6, -5, 2, 0, 0, 0, 14, -33, 
        -10, 15, 9, 15, 25, 7, 20, -12, 
        -16, 15, 10, 9, 4, 15, 15, -21, 
        -26, -45, -5, 2, 5, -5, -15, -32},

        {15, 5, -15, -15, -15, -15, -14, 15, 
        2, 15, 39, 15, 17, 26, 15, 28, 
        -5, 3, -15, -15, -15, 9, 1, 11, 
        -4, -8, 15, 15, -15, 15, -6, -11, 
        0, 15, -15, 1, -15, 14, 15, -2, 
        0, 15, 15, -15, 15, -8, 14, -4, 
        -30, -15, -15, -15, -15, 12, -2, -30, 
        -15, -15, 23, 15, 15, 16, 15, -15},

        {-65, -50, -15, -15, -45, -31, -46, -64, 
        -55, -5, -25, -15, 15, -21, -5, -55, 
        -32, -10, 20, -5, -3, 21, 2, -39, 
        -5, 10, 0, 5, 31, 0, 19, -5, 
        -5, 19, 0, 21, 34, 19, -10, -15, 
        -35, 15, -5, 25, 22, 0, 15, -45, 
        -28, -35, -20, -11, 10, -14, -35, -41, 
        -38, -25, -15, -41, -43, -21, -25, -65},

        {-30, -50, -23, -34, -15, -45, -50, -55, 
        -18, 12, -10, -5, -5, -9, -15, -42, 
        -29, 9, 8, 5, 7, 5, 14, -5, 
        -30, 1, 25, 15, 44, 5, 15, -30, 
        -14, -10, 0, 43, 15, 29, 19, -25, 
        -5, -6, -5, 5, 19, 5, -10, -18, 
        -35, -25, -7, 0, 0, -13, -17, -45, 
        -38, -35, -15, -39, -25, -27, -50, -40},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        -10, 0, 0, 15, -5, 0, 0, -10, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 5, 15, 0, 0, 19, 5, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        -15, 0, 0, -5, -5, 0, 0, 7, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, -35, -32, 0, 0, -13, -35, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        6, 5, 12, 15, 15, 10, 10, 5, 
        0, 4, 3, 13, 13, 4, 3, 0, 
        10, -5, -3, 36, 10, 16, -6, -7, 
        -3, -3, 17, 31, 30, 19, -3, -3, 
        0, 4, 27, 35, 19, 15, 1, -1, 
        -20, -25, -10, -15, -15, -19, -25, -20, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        int eps[6][64] = 
        {{-85, -77, -65, -55, -55, -65, -75, -85, 
        -83, -25, -15, -5, -5, -15, -25, -75, 
        -65, -15, 30, 20, 20, 30, -15, -65, 
        -55, -5, 23, 55, 55, 24, -5, -55, 
        -63, -5, 20, 55, 55, 33, -5, -55, 
        -65, -15, 23, 22, 43, 21, -15, -65, 
        -75, -25, -15, -5, -5, -15, -25, -75, 
        -85, -92, -65, -56, -55, -65, -75, -85},

        {15, 6, -14, -12, 10, -13, -15, 15, 
        -10, -5, -2, -4, -3, 1, -5, 11, 
        -15, -15, -15, -15, -15, -15, -13, -15, 
        10, 15, -9, 5, -15, -5, -15, -6, 
        10, 15, -15, -15, -15, -4, 7, 10, 
        10, -13, 2, 12, 15, -5, 13, 10, 
        -10, 8, -15, -15, -14, -15, -14, -20, 
        5, 0, 20, -2, 18, -5, 3, 5},

        {-65, -27, -15, -16, -45, -19, -28, -36, 
        -54, -5, -25, -15, 15, -12, -8, -55, 
        -45, 12, 25, -5, -4, -4, -8, -45, 
        -5, -6, 21, 5, 6, 0, 20, -34, 
        -6, 19, 1, 5, 6, 10, 8, -13, 
        -45, 15, -5, 14, -5, 19, 11, -15, 
        -42, -31, -24, 0, 14, -14, -16, -25, 
        -36, -25, -31, -40, -15, -15, -25, -56},

        {-25, -50, -37, -40, -40, -45, -50, -55, 
        -16, 12, -10, -5, -5, -10, -15, -40, 
        -19, -10, -5, 5, 5, 5, 16, -5, 
        -25, 1, -5, 5, 5, 5, 12, -30, 
        -25, -10, 0, 5, 5, 0, 19, -25, 
        -5, -9, -10, 5, 5, 5, -10, -5, 
        -45, -17, -15, 0, 0, -15, -25, -16, 
        -26, -35, -15, -40, -38, -45, -50, -40},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        -10, 0, 0, -2, -5, 0, 0, -10, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, 5, 15, 0, 0, 15, 5, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        -4, 0, 0, -5, -5, 0, 0, 3, 
        0, 0, 0, 0, 0, 0, 0, 0, 
        0, -27, -26, 0, 0, -29, -35, 0},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        27, 5, 11, 15, 15, 10, 10, 5, 
        15, 4, 3, 13, 13, 4, 3, 0, 
        -4, -5, 6, 10, 10, -5, -6, -7, 
        -3, -3, 5, 10, 10, 5, -3, -3, 
        0, 1, 2, 23, 28, 1, 0, -1, 
        -20, -25, -8, -15, -15, -5, -25, -18, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        Engine();

        void printAllBitboards();
        void setStartPos();

        bool ownKingBare();
        uint64_t perft(int, int);
        void readFen(std::string);
        std::string makeFen();
        void sendMove(std::string);
        uint32_t getMove();
        bool getSide();

        std::string makeRandMoves(int);

        void beginZobristHash();
        void showZobrist();
        int halfMoveCount();
        int evaluateScratch();
        void newGame();
        void copyEval(EvalVars);

        int quiesce(int, int, int);
        int countReps();
        int search(uint32_t, int, uint64_t, bool);

        ~Engine();
};


