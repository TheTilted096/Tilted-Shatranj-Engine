/*
Driver Code for Tilted Shatranj Engine
Fitted for 2nd Refactor of OOP Design

UCI-Compatible Shatranj Engine "Tilted"

7-7-24

*/

#include "STiltedPosition.h"

int main(){
    Position::initRookTable();
    
    /*
    Bitboard testocc = 0x10182016200010ULL;
    int sq = 28;

    std::cout << "RookMask[28]\n";
    Position::printAsBitboard(Position::RookMasks[sq], std::cout);
    std::cout << "Occupied Squares\n";
    Position::printAsBitboard(testocc, std::cout);
    //Position::hqRookAttack(sq, testocc);

    Bitboard atks = Position::RookBoards[Position::RookOffset[sq] + 
        _pext_u64(testocc, Position::RookMasks[sq])];

    std::cout << "Rook Attacks at 28\n";
    Position::printAsBitboard(atks, std::cout);
    */

    Position testPos;

    testPos.readFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w - - 0 1");
    //testPos.sendMove("e2c4"); //12847284
    //testPos.sendMove("h3g2"); //5619119
    //testPos.sendMove("h1f1"); //9506687
    

    
    
    //testPos.makemove(10619902U); //g1h3
    //testPos.makemove(5571917U); //f7f6
    //testPos.makemove(10618799U); //h3g5
    //testPos.makemove(4503696U); //a6c4

    testPos.printAllBitboards(std::cout);
    testPos.showZobrist(std::cout);


    auto start = std::chrono::steady_clock::now();
    std::cout << testPos.perft(6, 0) << '\n';
    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    

    //std::cout << testPos.perft(4, 0) << '\n';
    
    /*
    int nn = testPos.fullMoveGen(0, false);
    for (int i = 0; i < nn; i++){
        std::cout << Position::moveToAlgebraic(testPos.moves[0][i]) << '\n';
        Position::printMoveAsBinary(testPos.moves[0][i], std::cout);
        std::cout << testPos.moves[0][i] << '\n';
    }
    */
    
    
    



    

    return 0;
}