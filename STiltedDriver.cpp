/*

Driver Code for the Tilted Shatranj Engine

TheTilted096, 2024. 

*/
#include "STiltedSearchEval.cpp"

int main(){
    srand(time(0));

    initializeAll();

    std::string command;
    std::string param;
    bool originalMove;
    uint64_t* woriginal = new uint64_t[7];
    uint64_t* boriginal = new uint64_t[7];

    //std::string move;
    //int64_t dur;

    std::cout << "Shatranj Tilted 18 by TheTilted096\n";

    int alpha = -30000; //assume position is bad (you want to increase this)
    int beta = 30000; //good for your opponent (you want to decrease this)



    while (true){
        getline(std::cin, command);
        //std::cout << "command received: " << command << '\n';
        if (command == "quit"){ //self explanatory; just quit the program.
            delete[] woriginal;
            delete[] boriginal;
            cleanupAll();
            return 0;
        }
        if (command == "uci"){
            std::cout << "id name Shatranj Tilted 18\nid author TheTilted096\n";
            std::cout << "option name UCI_Variant type combo default shatranj var shatranj\nuciok\n";
        }
        if (command == "isready"){
            std::cout << "readyok\n";
        }
        if (command == "ucinewgame"){
            setStartPos();
        }
        if (command.substr(0, 17) == "position startpos"){
            setStartPos();
            zobristHashPosition();

            //"position startpos moves "...;
            if (command.length() > 25){
                std::string movesstring = command.substr(24);

                std::stringstream movestream(movesstring);
                
                int extraMoves = 1;
                for (char c : movesstring){
                    if (c == ' '){
                        extraMoves++;
                    }
                }

                std::string extraMoveList[extraMoves];
                for (int i = 0; i < extraMoves; i++){
                    movestream >> extraMoveList[i];
                    //std::cout << extraMoveList[i] << '\n';
                }

                for (int i = 0; i < extraMoves; i++){ //for each move found
                    fullMoveGen(0, 0); //generate all the moves
                    for (int j = 0; j < moves[0][0]; j++){ //for each of the moves generated
                        if (moveToAlgebraic(moves[0][j + 1]) == extraMoveList[i]){ //get their alg representation and compare
                            makeMove(moves[0][j + 1], 1, 0); //if so, make the move
                            break;
                        }
                    }
                }

            }
        
            //std::cout << "Moves Read In Correctly\n";
        }
        if (command.substr(0, 12) == "position fen"){
            //std::cout << command.substr(13) << '\n';
            readFen(command.substr(13));
        }

        //commands
        if (command.substr(0, 2) == "go"){
            /* Random Mover (Tilted 2)
            uint32_t* allMoves = fullMoveGen();
            int randindex;
            bool isLegal = false;
            while (!isLegal){
                randindex = 1 + rand() % allMoves[0]; //select a random index
                makeMove(allMoves[randindex], 1); //make the move
                if (!isChecked()){ //if not in check, we can proceed. 
                    isLegal = true;
                }
                makeMove(allMoves[randindex], 0); //unmake the move
            }

            std::cout << "bestmove " << moveToAlgebraic(allMoves[randindex]) << '\n';
            */
            /* Tilted 14
            originalMove = toMove;
            nodes = 0;
            auto start = std::chrono::steady_clock::now();
            evaluate();
            boardEval = alphabeta(alpha, beta, 4, 0);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            std::cout << "info depth 4 score cp " << boardEval << '\n';

            dur = duration.count();

            std::cout << "info nodes " << nodes << " nps ";

            if (dur == 0){
                std::cout << "0\n";
            } else {
                std::cout << 1000 * nodes / dur << '\n';
            }

            std::cout << "bestmove " << moveToAlgebraic(bestMove) << '\n';
            
            toMove = originalMove;
            */
            if (command.length() < 4){
                std::cout << "Unsupported Use of 'go'\n";
                continue;
            }
            loadPos(woriginal, boriginal, originalMove, 1);
            int originalCounter = totalHalfMoves;

            std::stringstream goStream(command.substr(3));
            std::string ourTime = toMove ? "wtime" : "btime";
            std::string ourInc = toMove ? "winc" : "binc";

            int tTime = INT_MAX;
            int tDepth = INT_MAX;

            while (!goStream.eof()){
                goStream >> param;
                if (param == ourTime){
                    goStream >> param;
                    tTime = stoi(param) / 40;
                }
                if (param == ourInc){
                    goStream >> param;
                    tTime += stoi(param) / 2;
                }
                if (param == "depth"){
                    goStream >> param;
                    tDepth = stoi(param);
                }
                if (param == "nodes"){
                    goStream >> param;
                    mnodes = stoi(param);
                }
            }
            nodes = 0;
            //std::cout << "Thinktime: " << tTime << "\tThinkDepth: " << tDepth << '\n';
            iterativeDeepening(alpha, beta, tTime, tDepth);
            loadPos(woriginal, boriginal, originalMove, 0);
            nodes = 0;
            mnodes = 1000000000;
            totalHalfMoves = originalCounter;
        }
        if (command.substr(0, 6) == "perft "){
            originalMove = toMove;
            auto start = std::chrono::steady_clock::now();
            std::cout << '\n' << perft((int) command[6] - 48, 0) << " positions\n";
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << duration.count() << " ms\n";
            toMove = originalMove;
        }
        
        //debug section
        if (command == "printpieces"){
            std::cout << "\nWhite's Pieces\n";
            printSidesBitboard(white);
            std::cout << "\nBlack's Pieces\n";
            printSidesBitboard(black);

            std::cout << "toMove:\t" << toMove << '\n';
        }
        if (command == "testfeature"){
            /* Zobrist Hash Debug
            for (int i = 0; i < totalHalfMoves + 1; i++){
                std::cout << "ZH " << i << ":\t" << zHistory[i];
                std::cout << "\tCTR " << i << ":\t" << currentHalfMoves[i] << '\n';
            }
            */
        }


        //std::cout << "command executed: " << command << '\n';
        //std::cout << "readyok\n";
    }
    return 0;
}