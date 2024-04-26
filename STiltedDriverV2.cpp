/*
Driver Code (V2) for the Tilted Shatranj Engine
Revised for optimization and compatibility with
STiltedMoveGenV3 and STiltedSearchEvalV2
4-24-2024
TheTilted096
*/

#include "STiltedSearchEvalV2.cpp"

int main(){
    initializeTables();
    setStartPos();
    beginZobristHash();

    std::string engineID = "Tilted Shatranj 22-Opto";
    std::string command, param;

    uint64_t oPos[14]; //original pieces
    bool oMove;  //original move

    std::cout << "0000000000    0000000000    00            0000000000    0000000000    000000  \n";
    std::cout << "    00            00        00                00        00            00    00\n";
    std::cout << "    00            00        00                00        00            00      00\n";
    std::cout << "    00            00        00                00        0000000000    00      00\n";
    std::cout << "    00            00        00                00        00            00      00\n";
    std::cout << "    00            00        00                00        00            00    00\n";
    std::cout << "    00        0000000000    0000000000        00        0000000000    000000  \n\n";
        
    //printf("\u2588  00000  0      00000  00000  0000 \n");

    std::cout << engineID << " by TheTilted096\n";
    std::cout << "(with LOTS of help from Prolix dev)\n";

    while (true){
        getline(std::cin, command);
        if (command == "quit"){
            return 0;
        }
        if (command == "uci"){
            std::cout << "id name " << engineID << "\nid author TheTilted096\n";
            std::cout << "option name UCI_Variant type combo default shatranj var shatranj\nuciok\n";
        }
        if (command == "isready"){
            std::cout << "readyok\n";
        }
        if (command == "ucinewgame"){
            setStartPos();
            thm = 0;
            beginZobristHash();
            for (int i = 0; i < 0xFFFFF; i++){
                if (ttable[i].enType != -1){
                    ttable[i] = TTentry();
                }
            }
        }
        if (command.substr(0, 17) == "position startpos"){
            setStartPos();
            thm = 0;
            beginZobristHash();

            if (command.length() > 25){
                std::stringstream extraMoves(command.substr(24));
                
                while (!extraMoves.eof()){
                    extraMoves >> param;
                    int aux = fullMoveGen(0, 0);
                    for (int i = 0; i < aux; i++){
                        if (moveToAlgebraic(moves[0][i]) == param){
                            makeMove(moves[0][i], 1, 0);
                            break;
                        }
                    }
                }
            }
        }
        if (command.substr(0, 12) == "position fen"){
            readFen(command.substr(13));
        }

        if (command.substr(0, 2) == "go"){
            for (int i = 0; i < 14; i++){
                oPos[i] = pieces[i];
            }
            oMove = toMove;
            int othm = thm;

            std::stringstream goStream(command.substr(3));
            std::string ourTime = toMove ? "wtime" : "btime";
            std::string ourInc = toMove ? "winc" : "binc";

            uint32_t tTime = 0xFFFFFFFF;
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

            iterativeDeepening(tTime, tDepth);

            for (int i = 0; i < 14; i++){
                pieces[i] = oPos[i];
            }
            thm = othm;

            nodes = 0;
            mnodes = 0x3FFFFFFF;


        }

        //debug functions
        if (command.substr(0, 6) == "perft "){
            oMove = toMove;
            auto start = std::chrono::steady_clock::now();
            std::cout << '\n' << perft((int) command[6] - 48, 0) << " positions\n";
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << duration.count() << " ms\n";
            toMove = oMove;
        }
        if (command == "printpieces"){
            printAllBitboards();
        }
        if (command == "testfeature"){
            //nothing to test
        }
        if (command == "showTTZobrist"){
            std::cout << "Tranposition Table:\n";
            for (int k = 1; k < 10; k++){
                for (int i = 0; i < 0xFFFFF; i++){
                    if (ttable[i].eDepth == k){
                        ttable[i].print();
                    }
                }
            }
            std::cout << "\nZobrist History + Last 20 Bits\n";
            for (int j = 0; j < thm + 1; j++){
                std::cout << "ZH " << j << ": " << zhist[j] << "\tIndex: " << (zhist[j] & 0xFFFFF) << '\n';
            }
        }
    }
}