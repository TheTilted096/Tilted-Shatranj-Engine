/*
2nd Version of Data Generation Program Functions

5-26-2024
TheTilted096

*/

#include "STiltedSearchEvalV3.cpp"
#include <thread>

class Game{
    Engine* players[2];
    std::string* positions;
    int gameClock;
    double gameResult;

    double adjudicate(int, bool);

    public:
        std::string opening;
        uint64_t mn;

        Game();
        Game(uint64_t, std::string);

        double play();
        void report(std::string, int, int, int);
        void setEngineEvals(EvalVars[]);

        ~Game();

        static std::string* makeOpeningsBook(int, int);
};

Game::Game(){
    players[0] = new Engine();
    players[1] = new Engine();
    opening = "Not an Opening";
    gameClock = 0;
    gameResult = -1;

    positions = new std::string[2048];

    mn = 0;
}

Game::Game(uint64_t m, std::string op){
    players[0] = new Engine();
    players[1] = new Engine();
    opening = op;
    gameClock = 0;
    gameResult = -1;

    positions = new std::string[2048];

    mn = m;
}

Game::~Game(){
    delete players[0];
    delete players[1];
    delete[] positions;
}

void Game::setEngineEvals(EvalVars evs[]){
    players[0]->copyEval(evs[0]);
    players[1]->copyEval(evs[1]);
}

std::string* Game::makeOpeningsBook(int nb, int nm){
    Engine g;
    std::string* bb = new std::string[nb];
    for (int i = 0; i < nb; i++){
        g.setStartPos();
        g.makeRandMoves(nm);
        bb[i] = g.makeFen();
    }

    return bb;
}

double Game::adjudicate(int wcs, bool ep){
    if ((players[ep]->halfMoveCount() >= 140)){
        return 0.5;
    }
    if (wcs < -28000){
        return 0;
    }
    if (wcs > 28000){
        return 1;
    }

    if ((players[ep]->countReps() > 2)){
        return 0.5;
    }

    if (players[0]->ownKingBare() and players[1]->ownKingBare()){
        return 0.5;
    }

    if (gameClock > 2046){
        return 0.5;
    }

    return -1;    
}

double Game::play(){
    int wcScore = 0;
    uint32_t playedMove;
    bool engineToMove = true;

    std::string fencmd = opening + " moves ";

    players[0]->newGame();
    players[1]->newGame();

    //std::cout << "M: ";

    while (true){
        players[engineToMove]->readFen(fencmd);
        assert(gameClock < 2048);
        positions[gameClock] = players[engineToMove]->makeFen();
        gameResult = adjudicate(wcScore, engineToMove);
        if (gameResult != -1){
            //std::cout << gameClock << '\n';
            break;
        }
        wcScore = players[engineToMove]->search(0xFFFFFFFF, 63, mn, false) * (2 * engineToMove - 1);
        playedMove = players[engineToMove]->bestMove;

        fencmd += moveToAlgebraic(playedMove);
        fencmd += ' ';

        gameClock++;
        engineToMove = !engineToMove;
        /*
        if (gameClock % 50 == 0){
            std::cout << gameClock << ' ';
        }
        */
    }

    //std::cout << "Finished Game\n";

    return gameResult;
}

void Game::report(std::string ofile, int lb, int ub, int id){
    std::ofstream fenFile(ofile, std::ios::app);
    fenFile << "GAME " << id << '\n';
    fenFile << "Result: " << gameResult << '\n';
    fenFile << "StartPos: " << positions[0] << '\n';

    for (int gg = lb; gg <= gameClock - ub; gg++){
        fenFile << positions[gg] << '\n';
    }
    
    fenFile << "FinalPos: " << positions[gameClock] << '\n';
    fenFile.close();
}

void executeSchedule(Game* lineup, int num, int id, double& res, bool output){
    res = 0;
    if (output){std::cout << "=== THREAD " << id << " STARTED ===\n";}
    for (int i = 0; i < num; i++){
        if (output){ std::cout << "Thread " << id << ": Game " << i + 1 << " of " << num << " Started\n";}
        res += lineup[i].play(); 
        //std::cout << "Thread " << id << ": Game " << i + 1 << " of " << num << " Finished\n";
    }
}

int matchSingle(std::string ofile, int params[], EvalVars evs[], std::string* openingsBook, bool output){
    //params: #games, #num moves, mnodes, lb, ub, #threads, start ID
    if (params[5] > 12){ std::cout << "Match Denied\n"; return -1; }

    double sum = 0; //sum of all the scores
    int base = params[0] / params[5]; //games per thread
    int* gn = new int[params[5]]; //list of games per thread
    double* divScores = new double[params[5]]; //list of scores from threads
    Game** divisions = new Game*[params[5]]; //pointer to every game list

    int bi = 0;
    for (int i = 0; i < params[5]; i++){
        gn[i] = base; 
        if (i < (params[0] % params[5])){
            gn[i]++;
        } //decide num games in division
        divisions[i] = new Game[gn[i]]; //initialize games

        for (int j = 0; j < gn[i]; j++){
            divisions[i][j].mn = params[2];
            divisions[i][j].opening = openingsBook[bi];
            divisions[i][j].setEngineEvals(evs);
            bi++;
        }
    }

    std::thread workers[params[5]]; //list of threads
    for (int k = 0; k < params[5]; k++){
        workers[k] = std::thread(executeSchedule, divisions[k], gn[k], k, std::ref(divScores[k]), output);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int k = 0; k < params[5]; k++){
        workers[k].join();
        if (output){std::cout << "=== THREAD " << k << " JOINED ===\n"; }
        sum += divScores[k];
        for (int l = 0; l < gn[k]; l++){
            divisions[k][l].report(ofile, params[3], params[4], params[6]);
            params[6]++;
        }
    }

    for (int ii = 0; ii < params[5]; ii++){
        delete[] divisions[ii];
    }
    delete[] divisions;
    delete[] divScores;
    delete[] gn;
    return (2 * sum);
}

double matchPairs(std::string ofile, int params[], EvalVars evs[], bool output){
    std::string* matchBook = Game::makeOpeningsBook(params[0], params[1]);
    std::cout << "### Forward Match Started ###\n";
    int mf = matchSingle(ofile, params, evs, matchBook, output);
    EvalVars bevs[] = {evs[1], evs[0]};
    std::cout << "### Backwards Match Started ###\n";
    int mb = matchSingle(ofile, params, bevs, matchBook, output);

    delete[] matchBook;

    return (mf + 2 * params[0] - mb);
}