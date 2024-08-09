/*
Class Definitions for New Autoplay Library

TheTilted096
8-5-24

*/

#include "STiltedEngine.h"
#include <thread>
#include <mutex>

class Game{
    public:
        Engine* players;
        std::string* positions;
        int gameClock;
        int gameResult;

        int adjudicate(int, bool);
        
        std::string opening;

        Game();
        Game(std::string, EvalVars&, EvalVars&);

        void play();
        void report(int);

        ~Game();

        static int fenHalfMove(std::string);

        static uint64_t maxNodes;
        static int beginReport, stopReport;
        static std::ofstream outFile;
        static std::string filename;
};

class Match{
    public:
        static int openingMoves;

        int dirScores[2];

        int gamesToPlay, gamesPlayed;
        int numThreads;
        std::string* book;
        EvalVars* evals;

        std::thread* workers;

        Match(int*, int*, EvalVars*, std::string);

        void playing(bool, bool, int);

        std::thread playThread(bool, bool, int);

        int runMatch(bool, bool);

        ~Match();

        static std::string* makeOpeningBook(int, int);
        static std::mutex gameBlock;
};