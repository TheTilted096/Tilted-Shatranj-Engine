/*

Driver Code for the Tilted Shatranj Engine

TheTilted096, 2024. 

*/

#include "STiltedSearchEval.cpp"

int main(){
    srand(time(0));

    initializeAll();

    std::string command;
    bool originalMove;

    std::cout << "Shatranj Tilted 13 by TheTilted096\n";

    int alpha = -30000; //assume position is bad (you want to increase this)
    int beta = 30000; //good for your opponent (you want to decrease this)

    while (true){
        getline(std::cin, command);
        //std::cout << "command received: " << command << '\n';
        if (command == "quit"){ //self explanatory; just quit the program.
            delete[] white;
            delete[] black; 
            deleteLookupTables();
            return 0;
        }
        if (command == "uci"){
            std::cout << "id name Shatranj Tilted 13\nid author TheTilted096\noption name UCI_Variant type combo default shatranj var shatranj\nuciok\n";
        }
        if (command == "isready"){
            std::cout << "readyok\n";
        }
        if (command.substr(0, 17) == "position startpos"){
            setStartPos();
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

                uint32_t* allMoves;

                for (std::string m : extraMoveList){ //for each move found
                    allMoves = fullMoveGen(); //generate all the moves
                    for (int i = 0; i < allMoves[0]; i++){ //for each of the moves generated
                        if (moveToAlgebraic(allMoves[i + 1]) == m){ //get their alg representation and compare
                            makeMove(allMoves[i + 1], 1); //if so, make the move
                        }
                    }
                    delete[] allMoves;
                }
                
            }
            /*
            std::cout << "\nWhite's Pieces:\n";
            printSidesBitboard(white);
            std::cout << "\nBlack's Pieces:\n";
            printSidesBitboard(black);
            */
        }
        if (command.substr(0, 12) == "position fen"){
            //std::cout << command.substr(13) << '\n';
            readFen(command.substr(13));
        }
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
            originalMove = toMove;
            boardEval = alphabeta(alpha, beta, 4, 0);
            std::cout << "info depth 4 score cp " << boardEval << '\n';
            std::cout << "bestmove " << moveToAlgebraic(bestMove) << '\n';
            toMove = originalMove;
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
        
        if (command == "printpieces"){
            std::cout << "\nWhite's Pieces\n";
            printSidesBitboard(white);
            std::cout << "\nBlack's Pieces\n";
            printSidesBitboard(black);
        }
        if (command == "eval"){
            std::cout << evaluate() << " : " << toMove << '\n';
        }


        //std::cout << "command executed: " << command << '\n';
        //std::cout << "readyok\n";
    }
    return 0;
}