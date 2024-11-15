/*
Main Driver Code for Tilted Shatranj Engine

TheTilted096, 5-25-2024

UCI interfacing and refitting to OOP design. 
*/

#include "STiltedEngine.h"

int main(int argc, char* argv[]){
    srand(time(0));

    Engine engine;

    if ((argc == 2) and (std::string(argv[1]) == "bench")){
        engine.bench();
        return 0;
    }

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
            std::cout << "option name UCI_Variant type combo default shatranj var shatranj\n";
            std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            std::cout << "option name Hash type spin default 32 min 32 max 32\n";
            std::cout << "uciok\n";
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
    
        // extra features for self
        if (command == "showTTZobrist"){
            engine.showZobrist();
        }
        if (command == "printpieces"){
            engine.printAllBitboards();
        }
        if (command == "testfeature"){
            /*
            std::string ds = "rnbkqb1r/pppppp1p/6pn/8/4P3/8/PPPP1PPP/RNBKQBNR w - - 0 1 moves b2b3 d7d6 b1c3 b8c6 d2d3 f7f6 e1d2 e7e6 d2e3 h6f7 f2f3 f8h6 g1e2 b7b6 f1h3 a7a6 a2a3 e8d7 h1e1 h8e8 a3a4 c6b4 d1d2 e6e5 a4a5 b6b5 d3d4 c8e6 c1a3 a8c8 a1d1 c7c6 c3b1 e6g4 c2c3 g4e2 c3b4 e2c4 b3c4 b5c4 d2c3 d6d5 d4e5 e8e5 a3c5 f7d6 c3c2 e5g5 e1g1 d7e6 b1c3 d8d7 d1e1 d5e4 e1d1 e6d5 f3e4 d5e6 e3d4 d7c7 c5e3 g5g4 e4e5 f6e5 d4e5 d6f7 e5f6 g4f4 g1f1 f4f1 d1f1 c8b8 c3a2 e6d5 f6g5 d5e6 c2c3 b8d8 a2c1 f7d6 f1e1 h6f8 e3c5 e6d5 e1e7 d8d7 e7d7 c7d7 c3d4 d7e6 g5h4 d6b5 d4e3 f8d6 c1a2 h7h6 h4g3 e6e5 g3f2 b5d4 g2g3 g6g5 h3f1 d5e4 h2h3 e4d5 f2e1 d4c2 e3d2 c2b4 a2b4 d6b4 d2e3 h6h5 e1f2 b4d6 c5e7 g5g4 e7c5 d5e4 f2g1 e5d5 c5a3 e4f3 g1h2 f3g2 h3h4 g2f3 f1h3 d5e5 h3f1 f3e4 h2g1 e4f3 a3c5 d6b4 c5a3 f3e4 g1f2 e5d5 f2g1 b4d6 g1f2 d6b4 f2g1 b4d6 g1f2 e4f3 f2g1 d5e5 a3c5 d6b4 c5a3 f3e4 g1f2 b4d6 f2e1 c4c3 a3c5 e5d5 c5a3 e4f3 e1f2 f3g2 f1d3 d5e5 a3c5 g2f3 c5a3 f3g2 a3c5 g2f3 c5a3 f3e4 d3f1 e5d5 f1d3 d6b4 d3f1 b4d6 f1d3 c3c2 d3f1 e4f3 f1d3 d5c4 d3f5 c4b4 a3c1 b4a5 e3d4 a5b4 f2e3 c6c5 d4e5 d6f8 e5d5 f8h6 e3d2 c5c4 c1e3 b4b3 e3c5 a6a5 c5e3 c4c3 d2c1 a5a4 e3c5 h6f8 f5d7 f3e2 d7f5 f8h6 d5d4 e2f3 d4d5 h6f8 f5d7 f3e2 d7f5 f8h6 d5d4 b3b4 d4d5 e2f3 f5d7 h6f8 d5d4 f8h6 d4d5 h6f8 d5d4 f3g2 d4d5 g2f1 d7f5 f1e2 d5d4 f8h6 d4d5 h6f8 d5d4 e2f1 d4d5 f1g2 f5d7 f8h6 d7f5 g2f1 f5d7 b4b3 d7f5 f1g2 d5d4 h6f8 d4d5 f8h6 d5d4 h6f8 d4d5 g2h3 c5a3 f8d6 f5d3 h3g2 a3c5 d6b4 d5d4 g2f3 d3f5 b4d2 c5a3 d2b4 a3c5 f3g2 c5a3 b4d2 d4d3 d2b4 d3d4 b4d6 a3c5 d6b8 d4d5 g2f3 d5d4 b3b4 d4d5 b4b3 d5d4";

            //failsoft played black in this case

            engine.newGame();

            int bs = 57;

            //std::cout << ds.substr(0, bs) << '\n';
            bs += 11;
            //std::cout << ds.substr(0, bs) << '\n';

            std::cout << "Loading Game\n";

            engine.readFen(ds.substr(0, bs));
            engine.search(0xFFFFFFFFU, 63, 524288, false);

            while (bs < ds.length() - 10){
                //engine.search(0xFFFFFFFF, 63, 524288, false);
                bs += 10;
                //std::cout << ds.substr(0, bs) << '\n';
                engine.readFen(ds.substr(0, bs));
                engine.search(0xFFFFFFFFU, 63, 524288, false);
            }

            std::cout << "Beginning Final Ply Analysis\n";

            engine.readFen(ds.substr(0, bs + 10));
            std::cout << ds.substr(0, bs + 10) << '\n';

            engine.special = true;
            engine.search(0xFFFFFFFFU, 4, 524288, true);  

            return 1;
            */
        
            std::cout << "nodesForever: " << engine.nodesForever << '\n';        
        }   
    }

    return 0;
}