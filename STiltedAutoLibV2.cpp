/*
Function Definitions for Auto Library

8-5-2024
TheTilted096
*/

#include "STiltedAutoLibV2.h"

//Game Class
uint64_t Game::maxNodes;
int Game::beginReport;
int Game::stopReport;
int Game::endReport;
std::ofstream Game::outFile;
std::string Game::filename;

Game::Game(){
    players = nullptr;
    positions = nullptr;
}

Game::Game(std::string op, EvalVars& a, EvalVars& b){
    opening = op;
    gameClock = 0;
    gameResult = -1;

    players = new Engine[2];

    players[0].copyEval(a);
    players[1].copyEval(b);

    positions = new std::string[1024];
}

Game::~Game(){
    delete[] players;
    delete[] positions;
}

int Game::fenHalfMove(std::string fen){
    int back = fen.length() - 3;
    int factor = 1;
    int col = 0;
    while (fen[back] != ' '){
        col += factor * (fen[back] - 48);
        factor *= 10;
        back--;
    }

    return col;
}

int Game::adjudicate(int wcs, bool ep){
    if (wcs < -28000){
        return 0;
    }
    if (wcs > 28000){
        return 2;
    }

    if ((players[ep].halfMoveCount() >= 140)){
        return 1;
    }
    if ((players[ep].countReps() > 2)){
        return 1;
    }
    if (players[0].ownKingBare() and players[1].ownKingBare()){
        return 1;
    }
    if (gameClock > 1022){
        return 1;
    }

    return -1;    
}

void Game::play(){
    int wcScore = 0;
    uint32_t playedMove;
    bool engineToMove = true;

    std::string fencmd = opening;

    players[0].newGame();
    players[1].newGame();

    //std::cout << "M: ";

    while (true){
        players[engineToMove].readFen(fencmd);
        assert(gameClock < 1024);
        positions[gameClock] = players[engineToMove].makeFen();
        gameResult = adjudicate(wcScore, engineToMove);
        if (gameResult != -1){
            //std::cout << gameClock << '\n';
            break;
        }
        //std::cout << fencmd << '\n';
        wcScore = players[engineToMove].search(0xFFFFFFFF, 63, maxNodes, false) * (2 * engineToMove - 1);
        playedMove = players[engineToMove].getMove();

        if (gameClock == 0){
            fencmd += " moves";
        }
        fencmd += ' ';
        fencmd += Bitboards::moveToAlgebraic(playedMove);

        gameClock++;
        engineToMove = !engineToMove;
    }
}

void Game::report(int id){
    std::ofstream outputFile(filename, std::ios::app);
    outputFile << "GAME " << id << '\n';
    outputFile << "Result: " << gameResult / 2.0 << '\n';
    outputFile << "StartPos: " << positions[0] << '\n';

    for (int gg = beginReport; gg <= gameClock - endReport; gg++){
        if (fenHalfMove(positions[gg]) <= stopReport){
            outputFile << positions[gg] << '\n';
        }
    }
    
    outputFile << "FinalPos: " << positions[gameClock] << '\n';
    outputFile.close();
}


//Match Class
/*
Runs Games in parallel among the threads.
Threads will pick up any available games. 
*/

std::mutex Match::gameBlock;
int Match::openingMoves;

Match::Match(int* gameInfo, int* reporting, EvalVars* eptr, std::string ofile){
    gamesToPlay = gameInfo[0];
    gamesPlayed = 0;

    Game::maxNodes = gameInfo[1];
    numThreads = gameInfo[2];

    Game::beginReport = reporting[1];
    Game::stopReport = reporting[2];
    Game::endReport = reporting[3];

    openingMoves = reporting[0];

    book = makeOpeningBook(gamesToPlay, openingMoves);

    evals = eptr;

    workers = new std::thread[numThreads];

    dirScores[0] = 0; dirScores[1] = 0;

    Game::filename = ofile;
}

void Match::playing(bool forward, bool output, int threadID){
    Game* gameptr = new Game();
    int currID;
    while (true){
        gameBlock.lock(); //use mutex
        if (gamesToPlay == gamesPlayed){ //if there are no games left
            gameBlock.unlock();
            delete gameptr;
            break; //exit loop
        } else { //otherwise, construct game, save ID, and increment game counter
            delete gameptr;

            gameptr = new Game(book[gamesPlayed], evals[forward], evals[!forward]);
            currID = gamesPlayed;
            if (output){ std::cout << "(Thread " << threadID << ") Started Game " << currID << '\n'; }
            gamesPlayed++;
        }
        gameBlock.unlock(); //unlock

        gameptr->play(); // let game continue

        gameBlock.lock(); //ensure no conflict while outputting
        gameptr->report(currID); 
        dirScores[forward] += gameptr->gameResult;
        if (output){
            std::cout << "(Thread " << threadID << ") Finished Game " << currID 
                << "  (" << gameptr->gameResult / 2.0 << ")\n";
        }
        gameBlock.unlock();        
    }
}

std::thread Match::playThread(bool forward, bool output, int threadID){
    return std::thread(Match::playing, this, forward, output, threadID);
}

int Match::runMatch(bool forward, bool output){
    gamesPlayed = 0;
    dirScores[forward] = 0;
    for (int k = 0; k < numThreads; k++){
        workers[k] = playThread(forward, output, k);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int l = 0; l < numThreads; l++){
        workers[l].join();
    }

    return dirScores[forward];
}

std::string* Match::makeOpeningBook(int nb, int nm){
    Position g;
    std::string* bb = new std::string[nb];
    for (int i = 0; i < nb; i++){
        g.setStartPos();
        g.makeOpening(nm);
        bb[i] = g.makeFen();
    }

    return bb;
}

Match::~Match(){
    delete[] book;
    delete[] workers;
}

