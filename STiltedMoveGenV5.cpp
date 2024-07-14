/*
5th Generation MoveGen
2nd Refactor

Definition of Functions in
Position Class

*/

#include "STiltedPosition.h"

Bitboard Position::RookBoards[0x19000];
std::ofstream exStream("exFile.txt");

Position::Position(){}

Bitboard Position::hqRookAttack(int sq, Bitboard occ){
    Bitboard forward = occ & (0x0101010101010101ULL << (sq & 7));
    Bitboard reverse = __builtin_bswap64(forward);

    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));

    forward ^= __builtin_bswap64(reverse);
    forward &= (0x0101010101010101ULL << (sq & 7)); //forward is now vertical attacks

    //printAsBitboard(forward, std::cout);

    Bitboard horizontal = ((uint64_t)hlt[sq & 7]
        [((((0xFFULL << ((sq & 56))) & occ) >> (sq & 56)) >> 1) & 63]) << (sq & 56);

    //printAsBitboard(horizontal, std::cout);
    //printAsBitboard(forward | horizontal, std::cout);

    return (forward | horizontal);
}

void Position::initRookTable(){
    int pos = 0;
    Bitboard cons;

    //std::ofstream rt("rawTables.txt");

    for (int sq = 0; sq < 64; sq++){
        for (int idx = 0; idx < (1 << RookBits[sq]); idx++){
            cons = _pdep_u64(idx, RookMasks[sq]) | (1ULL << sq);
            RookBoards[pos] = hqRookAttack(sq, cons);
            //printAsBitboard(RookBoards[pos], rt);
            pos++;
        }
    }
}

/*
Move Representation:

0000 0000 0 000 0 000 000 0 000000 000000

0-5: start square
6-11: end square

12: Capture
13-15: Captured Type
16-18: Piece Type Moved
19: Promotion
20-22: Piece End Type
23: Color
*/

void Position::setStartPos(){
    sides[0] = 0xFFFFULL;
    sides[1] = 0xFFFF000000000000ULL;

    pieces[0] = 0x800000000000008ULL;
    pieces[1] = 0x8100000000000081ULL;
    pieces[2] = 0x4200000000000042ULL;
    pieces[3] = 0x1000000000000010ULL;
    pieces[4] = 0x2400000000000024ULL;
    pieces[5] = 0xFF00000000FF00ULL;

    toMove = true;
    gamePhase = 64;
    thm = 0; chm[thm] = 0;

    //zhist[0] = 0x6344017539DA3DCBULL;
}

int Position::fullMoveGen(int ply, bool cpex){
    int totalMoves = 0;

    Bitboard mvst, pcs, xset;
    Bitboard occ = sides[0] | sides[1];
    int f, p;


    //pawns
    //deal with pawn pushes first
    pcs = sides[toMove] & pieces[5];
    Bitboard pshtrgt = !cpex * ((pcs << 8) >> (toMove << 4) & ~occ);
    while (pshtrgt){
        f = __builtin_ctzll(pshtrgt); //get the least bit (destination square);
        moves[ply][totalMoves] = f - 8 + (toMove << 4); //deduce starting square from color
        moves[ply][totalMoves] |= (f << 6); //write in destination 
        moves[ply][totalMoves] |= (5U << 16); //piece type (5)
        
        bool prmt = (1ULL << f) & 0xFF000000000000FF; //determine promotion
        moves[ply][totalMoves] |= ((10U ^ (13U * prmt)) << 19); //brancless set promotion bits

        moves[ply][totalMoves] |= (toMove << 23); //set color bit
        pshtrgt ^= (1ULL << f); //pop out bit
        totalMoves++;
    }

    //then pawn captures
    while (pcs){
        f = __builtin_ctzll(pcs); //get least bit (starting square)
        mvst = plt[toMove][f] & sides[!toMove]; //pawn captures from that square
        while (mvst){ //for each set bit
            p = __builtin_ctzll(mvst); //get the least bit (destination)
            moves[ply][totalMoves] = f; //write in start
            moves[ply][totalMoves] |= (p << 6); //write in end

            for (int cc = 1; cc < 6; cc++){ //determine victim
                if ((1ULL << p) & pieces[cc]){
                    moves[ply][totalMoves] |= (1U << 12);
                    moves[ply][totalMoves] |= (cc << 13);
                    break;   
                }
            }

            moves[ply][totalMoves] |= (5U << 16); //piece type (5)

            bool prmt = ((1ULL << p) & 0xFF000000000000FF); //cast promotion mask to bool
            moves[ply][totalMoves] |= ((10U ^ (13U * prmt)) << 19); //branchless set promotion bits
            moves[ply][totalMoves] |= (toMove << 23); //set color bits
            
            mvst ^= (1ULL << p); //pop out bit
            totalMoves++; //next
        }
        pcs ^= (1ULL << f);       
    }

    //leapers
    for (int ll = 4; ll > 1; ll--){
        pcs = sides[toMove] & pieces[ll];
        while (pcs){
            f = __builtin_ctzll(pcs); //starting square
            //captures first
            xset = llt[ll][f] & sides[!toMove];
            mvst = !cpex * ((llt[ll][f] & ~sides[toMove]) ^ xset); //non-captures

            //printAsBitboard(xset, std::cout);
            while (xset){
                p = __builtin_ctzll(xset); //get destination square
                moves[ply][totalMoves] = f; //input start and end squraes
                moves[ply][totalMoves] |= (p << 6);
                
                for (int cc = 1; cc < 6; cc++){ //determine victim
                    if ((1ULL << p) & pieces[cc]){ 
                        moves[ply][totalMoves] |= (1U << 12);
                        moves[ply][totalMoves] |= (cc << 13);
                        break;   
                    }
                }

                moves[ply][totalMoves] |= (ll << 16); //set bits
                moves[ply][totalMoves] |= (ll << 20);
                moves[ply][totalMoves] |= (toMove << 23);

                totalMoves++;
                xset ^= (1ULL << p); //pop out
            }
            
            //NON captures [generated above]
            //printAsBitboard(mvst, std::cout);
            while (mvst){
                p = __builtin_ctzll(mvst); //get destination square
                moves[ply][totalMoves] = f; //input start and end squraes
                moves[ply][totalMoves] |= (p << 6);

                moves[ply][totalMoves] |= (ll << 16); //set bits
                moves[ply][totalMoves] |= (ll << 20);
                moves[ply][totalMoves] |= (toMove << 23);

                totalMoves++;
                mvst ^= (1ULL << p); //pop out
            }

            pcs ^= (1ULL << f);
        }
    }

    //rooks
    pcs = sides[toMove] & pieces[1];
    while (pcs){ //for each rook
        f = __builtin_ctzll(pcs); //get starting square
        Bitboard rtb = RookBoards[RookOffset[f] + _pext_u64(occ, RookMasks[f])]; //get attack board
        //Bitboard rtb = hqRookAttack(f, occ);
        xset = rtb & sides[!toMove]; //get captures first
        mvst = !cpex * ((rtb & ~sides[toMove]) ^ xset); //non captures
        while (xset){
            p = __builtin_ctzll(xset);
            moves[ply][totalMoves] = f; //start and end squares
            moves[ply][totalMoves] |= (p << 6);

            for (int cc = 1; cc < 6; cc++){ //find victim
                if ((1ULL << p) & pieces[cc]){
                    moves[ply][totalMoves] |= (1U << 12);
                    moves[ply][totalMoves] |= (cc << 13);
                    break;
                }
            }
            moves[ply][totalMoves] |= 0x110000U; //set bits about piece type
            moves[ply][totalMoves] |= (toMove << 23);

            xset ^= (1ULL << p); //pop out bit
            totalMoves++;
            //assert(totalMoves < 256);
        }

        //non captures (generated above
        while (mvst){ 
            p = __builtin_ctzll(mvst);
            moves[ply][totalMoves] = f;
            moves[ply][totalMoves] |= (p << 6);
            moves[ply][totalMoves] |= 0x110000U;
            moves[ply][totalMoves] |= (toMove << 23);
            totalMoves++;
            mvst ^= (1ULL << p);
            //assert(totalMoves < 256);
        }

        pcs ^= (1ULL << f);
    }
    
    //King
    pcs = sides[toMove] & pieces[0]; //get king
    f = __builtin_ctzll(pcs); //locate it
    xset = llt[0][f] & sides[!toMove]; //do captures
    mvst = !cpex * ((llt[0][f] & ~sides[toMove]) ^ xset); //non captures 
    while (xset){
        p = __builtin_ctzll(xset); //find destination
        moves[ply][totalMoves] = f; //set start/end bits
        moves[ply][totalMoves] |= (p << 6);

        for (int cc = 1; cc < 6; cc++){ //locate victim and set bits
            if ((1ULL << p) & pieces[cc]){
                moves[ply][totalMoves] |= (1U << 12);
                moves[ply][totalMoves] |= (cc << 13);
                break;
            }
        }

        moves[ply][totalMoves] |= (toMove << 23); //set side (everything else is 0)
        totalMoves++; //pop
        xset ^= (1ULL << p);
    }

    //do for non-captures (generated above)
    while (mvst){
        p = __builtin_ctzll(mvst); //get destination and set location and side bits.
        moves[ply][totalMoves] = f;
        moves[ply][totalMoves] |= (p << 6);
        moves[ply][totalMoves] |= (toMove << 23);
        totalMoves++;
        mvst ^= (1ULL << p);
    }
    

    return totalMoves;
}

bool Position::kingBare(){
    bool b = (__builtin_popcountll(sides[0]) == 1);
    bool w = (__builtin_popcountll(sides[1]) == 1);
    return (toMove and !w and b) or (!toMove and w and !b);
}

bool Position::isChecked(bool sd){
    uint64_t inc;
    int ksq = __builtin_ctzll(sides[sd] & pieces[0]);
    //std::cout << "kingSquare: ksq\n";
    inc = llt[0][ksq] & pieces[0] & sides[!sd];
    inc |= RookBoards[RookOffset[ksq] + _pext_u64((sides[0] | sides[1]), RookMasks[ksq])] 
        & pieces[1] & sides[!sd];
    inc |= llt[2][ksq] & pieces[2] & sides[!sd];
    inc |= llt[3][ksq] & pieces[3] & sides[!sd];
    inc |= llt[4][ksq] & pieces[4] & sides[!sd];
    inc |= plt[sd][ksq] & pieces[5] & sides[!sd];

    //phasma = inc;

    return inc; 
}

int Position::countReps(){
    int rind = thm;
    int reps = 1;
    do {
        rind -= 2;
        reps += (zhist[thm] == zhist[rind]);
    } while (chm[rind] and (rind >= 0));

    return reps;
}

void Position::beginZobristHash(){
    zhist[thm] = 0ULL; chm[thm] = thm;

    Bitboard tpbd;
    int f;
    for (int i = 0; i < 6; i++){
        tpbd = sides[0] & pieces[i];
        while (tpbd){
            f = __builtin_ctzll(tpbd);
            zhist[thm] ^= zpk[0][i][f];
            tpbd ^= (1ULL << f);
        }

        tpbd = sides[1] & pieces[i];
        while (tpbd){
            f = __builtin_ctzll(tpbd);
            zhist[thm] ^= zpk[1][i][f];
            tpbd ^= (1ULL << f);
        }
    }

}

void Position::showZobrist(std::ostream& ou){
    for (int j = 0; j < thm + 1; j++){
        ou << "ZH " << j << ": " << zhist[j] << "\tIndex: " << (zhist[j] & 0xFFFFF) << '\n';
    }
}

void Position::makemove(Move move){
    uint8_t stsq = move & 0x3FU; uint8_t edsq = (move >> 6) & 0x3FU; //start square, end square
    uint8_t tpmv = (move >> 16) & 7U; uint8_t tpnd = (move >> 20) & 7U; //typeMoved, typeEnded
    bool capt = (move >> 12) & 1U; uint8_t cptp = (move >> 13) & 7U; //capturing, capturetype
    //color = toMove 

    //deal with captured piece
    pieces[cptp] ^= ((Bitboard) capt * (1ULL << edsq)); //remove from piece type board
    //if (capt){ std::cout << (int) cptp << '\t' << (int) edsq << '\n';}
    sides[!toMove] ^= ((Bitboard) capt * (1ULL << edsq)); //remove from opps board

    pieces[tpmv] ^= (1ULL << stsq); //move the piece
    pieces[tpnd] ^= (1ULL << edsq);
    sides[toMove] ^= ((1ULL << stsq) | (1ULL << edsq));

    gamePhase += (((move >> 19) & 1U) - phases[cptp]); 

    thm++; //increment half-move
    uint64_t zFactor = capt * zpk[!toMove][cptp][edsq];
    zFactor ^= zpk[toMove][tpmv][stsq];
    zFactor ^= zpk[toMove][tpnd][edsq];
    zFactor ^= ztk;

    zhist[thm] = zhist[thm - 1] ^ zFactor;
    chm[thm] = (capt or (tpmv == 5)) * (chm[thm - 1] + 1);

    toMove = !toMove;
}

void Position::unmakemove(Move move){
    //when unmaking a move, color that played is !toMove
    uint8_t stsq = move & 0x3FU; uint8_t edsq = (move >> 6) & 0x3FU; //start square, end square
    uint8_t tpmv = (move >> 16) & 7U; uint8_t tpnd = (move >> 20) & 7U; //typeMoved, typeEnded
    bool capt = (move >> 12) & 1U; uint8_t cptp = (move >> 13) & 7U; //capturing, capturetype

    pieces[tpmv] ^= (1ULL << stsq); //move the piece
    pieces[tpnd] ^= (1ULL << edsq);
    sides[!toMove] ^= ((1ULL << stsq) | (1ULL << edsq));

    //deal with captured piece
    pieces[cptp] ^= ((Bitboard) capt * (1ULL << edsq)); //place back to piece type board
    sides[toMove] ^= ((Bitboard) capt * (1ULL << edsq)); //update opps board

    gamePhase -= (((move >> 19) & 1U) - phases[cptp]);

    thm--;
    toMove = !toMove;
}

std::string Position::moveToAlgebraic(Move& move){
    uint8_t start = move & 63U;
    uint8_t end = (move >> 6) & 63U;

    std::string result;

    result += ((start & 7) + 97);
    result += (8 - (start >> 3)) + 48;

    result += ((end & 7) + 97);
    result += (8 - (end >> 3)) + 48;

    if ((move >> 19) & 1U) {
        result += 'q';
    }

    return result;
}

void Position::printMoveAsBinary(Move move, std::ostream& ou){
    int spaces[8] = {25, 19, 18, 15, 12, 11, 8, 7};

    std::bitset<32> mb(move);
    for (int i = 0; i < 32; i++){
        ou << mb[31 - i];
        for (int j : spaces){
            if (i == j){
                ou << ' ';
                break;
            }
        }
    }
    ou << '\n';
}

uint64_t Position::perft(int depth, int ply){
    uint64_t pnodes = 0ULL;
    if (depth == 0){
        return 1ULL;
    }

    int nm = fullMoveGen(ply, false);
    for (int i = 0 ; i < nm; i++){
        makemove(moves[ply][i]);
        
        //exStream << "\nconsidering @ ply " << ply << ": " << moveToAlgebraic(moves[ply][i]);
        //exStream << "\ndec: " << moves[ply][i] << '\n';
        //printMoveAsBinary(moves[ply][i], exStream);
        //exStream << "made " << moveToAlgebraic(moves[ply][i]) << '\n';
        //printAllBitboards(exStream);
            
        
        if (isChecked(!toMove)){
            //exStream << "illegal: " << moveToAlgebraic(moves[ply][i]) << '\n';
            //printAsBitboard(phasma, exStream);
            //printAllBitboards(exStream);

            unmakemove(moves[ply][i]);
            continue;
        }
        uint64_t additional = perft(depth - 1, ply + 1);
        if (ply == 0){
            std::cout << moveToAlgebraic(moves[ply][i]) << ": " << additional << '\n';
            //printMoveAsBinary(moves[ply][i], std::cout);
            //std::cout << "dec: " << moves[ply][i] << '\n';
        }
        pnodes += additional;
        unmakemove(moves[ply][i]);

        //exStream << "unmade " << moveToAlgebraic(moves[ply][i]) << '\n';
        //printAllBitboards(exStream);
        
    }

    return pnodes;
}

void Position::sendMove(std::string expr){
    int aux = fullMoveGen(0, 0);
    for (int i = 0; i < aux; i++){
        if (moveToAlgebraic(moves[0][i]) == expr){
            makemove(moves[0][i]);
            break;
        }
    }
}

void Position::readFen(std::string fen){
    sides[0] = 0ULL; sides[1] = 0ULL;
    pieces[0] = 0ULL; pieces[1] = 0ULL; pieces[2] = 0ULL;
    pieces[3] = 0ULL; pieces[4] = 0ULL; pieces[5] = 0ULL;

    uint8_t sq = 0, sym, ind = -1;
    while (sq < 64){
        ind++;
        for (int c = 0; c < 21; c++){
            if (fen[ind] == posChars[c]){
                sym = c;
                break;
            }
        }
        if ((0 < sym) and (sym < 9)){
            sq += sym;
            continue;
        }
        if ((sym >= 9) and (sym <= 14)){
            pieces[sym - 9] |= (1ULL << sq);
            sides[0] |= (1ULL << sq);
            sq++;
            continue;
        }
        if ((sym >= 15) and (sym <= 20)){
            pieces[sym - 15] |= (1ULL << sq);
            sides[1] |= (1ULL << sq);
            sq++;
            continue;
        }
    }

    gamePhase = 0;
    gamePhase += __builtin_popcountll(pieces[1]) * phases[1];
    gamePhase += __builtin_popcountll(pieces[2]) * phases[2];
    gamePhase += __builtin_popcountll(pieces[3]) * phases[3];
    gamePhase += __builtin_popcountll(pieces[4]) * phases[4];
    gamePhase += __builtin_popcountll(pieces[5]) * phases[5];

    ind += 2; //skip a space
    toMove = fen[ind] & 1; //'w' = 119 and 'b' = 98;

    std::stringstream extraMoves(fen.substr(ind + 5));

    std::string fenTerm;
    extraMoves >> fenTerm;
    thm = stoi(fenTerm);  // get total half move counter

    beginZobristHash();

    extraMoves >> fenTerm;  // burn total move counter

    while (!extraMoves.eof()) {
        extraMoves >> fenTerm;
        sendMove(fenTerm);
    }
}

void Position::printAsBitboard(Bitboard bb, std::ostream& ou){
    std::bitset<64> bset(bb);
    for (int i = 0; i < 64; i++){
        ou << bset[i];
        if ((i & 7) == 7){
            ou << '\n';
        }
    }

    ou << '\n';
}

void Position::printAllBitboards(std::ostream& ou){
    std::bitset<64> seta(sides[0]); std::bitset<64> setb(sides[1]);
    ou << "sides[0] (black)\tsides[1] (white)\n";
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            ou << seta[(i << 3) + j];
        }
        ou << "\t\t";
        for (int j = 0; j < 8; j++){
            ou << setb[(i << 3) + j];
        }
        ou << '\n';
    }

    std::bitset<64> bds[6];
    for (int a = 0; a < 6; a++){
        bds[a] = std::bitset<64>(pieces[a]);
    }

    ou << "\n\npieces[0]\tpieces[1]\tpieces[2]\tpieces[3]\tpieces[4]\tpieces[5]\n";
    for (int i = 0; i < 8; i++){
        for (int k = 0; k < 6; k++){
            for (int j = 0; j < 8; j++){
                ou << bds[k][(i << 3) + j];
            }
            ou << '\t';
        }
        ou << '\n';
    }
    ou << '\n';
    ou << "wasChecked: " << isChecked(!toMove) << '\n';
    ou << "isChecked: " << isChecked(toMove) << '\n';
    ou << "toMove: " << toMove << "\n\n";
}
















