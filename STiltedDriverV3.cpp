/*
Main Driver Code for Tilted Shatranj Engine

TheTilted096, 5-25-2024

UCI interfacing and refitting to OOP design. 
*/

#include "STiltedSearchEvalV3.cpp"

int main(){
    srand(time(0));
    initializeTables();

    Engine engine;

    std::string versionID = "Tilted Shatranj 26-zfix";
    std::string command, param;

    std::cout << "0000000000    0000000000    00            0000000000    0000000000    000000  \n";
    std::cout << "    00            00        00                00        00            00    00\n";
    std::cout << "    00            00        00                00        00            00      00\n";
    std::cout << "    00            00        00                00        0000000000    00      00\n";
    std::cout << "    00            00        00                00        00            00      00\n";
    std::cout << "    00            00        00                00        00            00    00\n";
    std::cout << "    00        0000000000    0000000000        00        0000000000    000000  \n\n";
        
    std::cout << versionID << " by TheTilted096\n";
    std::cout << "(with much help from sscg13 and ChessMaster121)\n";

    while (true){
        getline(std::cin, command);
        if (command == "quit"){
            return 0;
        }
        if (command == "uci"){
            std::cout << "id name " << versionID << "\nid author TheTilted096\n";
            std::cout << "option name UCI_Variant type combo default shatranj var shatranj\nuciok\n";
        }
        if (command == "isready"){
            std::cout << "readyok\n";
        }
        if (command == "ucinewgame"){
            engine.newGame();
        }
        if (command.substr(0, 17) == "position startpos"){
            engine.setStartPos();
            engine.beginZobristHash();
            if (command.length() > 25){
                std::stringstream extraMoves(command.substr(24));

                while (!extraMoves.eof()){
                    extraMoves >> param;
                    engine.sendMove(param);
                }
            }
        }
        if (command.substr(0, 12) == "position fen"){
            engine.readFen(command.substr(13));
        }
        if (command.substr(0, 2) == "go"){
            std::stringstream goStream(command.substr(3));
            std::string ourTime = engine.getSide() ? "wtime" : "btime";
            std::string ourInc = engine.getSide() ? "winc" : "binc";

            uint32_t tTime = 0xFFFFFFFF;
            int tDepth = 63;
            uint64_t maxNodes = 0x3FFFFFFFULL;

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
                    maxNodes = stoi(param);
                }
                if (param == "movetime"){
                    goStream >> param;
                    tTime = stoi(param);
                }
            }

            engine.search(tTime, tDepth, maxNodes, true);
        }
        if (command.substr(0, 6) == "perft "){
            auto start = std::chrono::steady_clock::now();
            std::cout << '\n' << engine.perft((int) command[6] - 48, 0) << '\n';
            auto end = std::chrono::steady_clock::now();

            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
        }
    
        if (command == "showTTZobrist"){
            engine.showZobrist();
        }
        if (command == "printpieces"){
            engine.printAllBitboards();
        }
    
    }

    return 0;
}