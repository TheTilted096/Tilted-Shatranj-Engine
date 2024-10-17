/*
Main Driver Code for Tilted Shatranj Engine

TheTilted096, 5-25-2024

UCI interfacing and refitting to OOP design. 
*/

#include "STiltedEngine.h"

int main(){
    srand(time(0));

    Engine engine;

    std::string versionID = "Tilted Shatranj 28-dev";
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
        if (command == "testfeature"){
            //std::string ds = "position fen r1bkqbnr/pppnpppp/3p4/8/8/P6N/1PPPPPPP/RNBKQB1R w - - 0 1 moves d2d3 c7c6 b1c3 e7e6 e2e3 b7b6 b2b3 g8e7 f2f3 f7f6 h3f2 f8h6 f1h3 e8f7 f2e4 d8c7 e1f2 c8a6 h1e1 e6e5 b3b4 f7e6 b4b5 e6d5 e4g3 g7g6 b5a6 d7c5 e3e4 d5e6 f2e3 c5a6 a3a4 h8e8 g3e2 a8d8 g2g3 d6d5 e4d5 e6d5 c1a3 e7c8 d1d2 c8d6 f3f4 a6b8 f4e5 f6e5 a1b1 b8d7 e1f1 a7a6 b1b4 g6g5 b4b1 d5e6 b1b4 e6d5 b4b1 a6a5 h3f5 d7f6 h2h3 d5e6 f5h7 f6h7 d2e1 e6d5 e1d2 e8e6 e2g1 d8g8 b1e1 h7f8 g1f3 f8d7 e3d4 d6f7 d4e3 e6e8 d2e2 f7d6 e2f2 e5e4 d3e4 d5e4 f3d2 g8f8 f2g2 f8f1 g2f1 d7f6 e3d4 c7b7 f1g2 b7c7 g2f1 c7b8 f1f2 b8c7 f2f1";

            //std::cout << "r1bkqbnr/pppnpppp/3p4/8/8/P6N/1PPPPPPP/RNBKQB1R w - - 0 1" << '\n';
            //engine.readFen("r1bkqbnr/pppnpppp/3p4/8/8/P6N/1PPPPPPP/RNBKQB1R w - - 0 1");
            //engine.search(0xFFFFFFFF, 63, 524288, false);

            /* 
            std::cout << "Loading Game... \n";

            int bs = 69;
            for (int i = bs; i <= ds.length() - 13 - 10; i += 10){
                //std::cout << ds.substr(13, i) << '\n';
                engine.readFen(ds.substr(13, i));
                engine.search(0xFFFFFFFF, 63, 524288, false);
                bs += 10;
            }

            std::cout << "Begin Final Ply Search:\n";
            engine.readFen(ds.substr(13, bs));

            engine.special = true;
            engine.search(0xFFFFFFFF, 63, 524288, true);
            */

            

        }
        
    }

    return 0;
}