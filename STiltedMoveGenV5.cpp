/*
Function Definition for Move Generation Classes

Contains Bitboard methods and some Position methods

TheTilted096, 7-15-2024
*/

#include "STiltedEngine.h"

bool Bitboards::RookInit = false;
Bitboard Bitboards::RookBoards[0x19000];

Bitboards::Bitboards(){ 
    if (!RookInit){ initRookTable(); }
    RookInit = true;
}

Bitboard Bitboards::hqRookAttack(int sq, Bitboard occ){
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

void Bitboards::initRookTable(){
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

void Bitboards::printAsBitboard(Bitboard bb, std::ostream& ou){
    for (int i = 0; i < 64; i++){
        ou << (bb & 1);
        if ((i & 7) == 7){
            ou << '\n';
        }
        bb >>= 1;
    }
    ou << '\n';
}

void Bitboards::printMoveAsBinary(Move move, std::ostream& ou){
    int spaces[8] = {25, 19, 18, 15, 12, 11, 8, 7};

    for (int i = 0; i < 32; i++){
        ou << ((move >> (31 - i)) & 1U);
        for (int j : spaces){
            if (i == j){
                ou << ' ';
                break;
            }
        }
    }
    ou << '\n';
}

void Bitboards::printAllBitboards(std::ostream& ou){
    ou << "sides[0] (black)\tsides[1] (white)\n";
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            ou << ((sides[0] >> ((i << 3) + j)) & 1ULL);
        }
        ou << "\t\t";
        for (int j = 0; j < 8; j++){
            ou << ((sides[1] >> ((i << 3) + j)) & 1ULL);
        }
        ou << '\n';
    }

    ou << "\n\npieces[0]\tpieces[1]\tpieces[2]\tpieces[3]\tpieces[4]\tpieces[5]\n";
    for (int i = 0; i < 8; i++){
        for (int k = 0; k < 6; k++){
            for (int j = 0; j < 8; j++){
                ou << ((pieces[k] >> ((i << 3) + j)) & 1ULL);
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

std::string Bitboards::moveToAlgebraic(Move& move){
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

bool Bitboards::ownBare(bool sd){
    return (__builtin_popcountll(sides[sd]) == 1);
}

bool Bitboards::kingBare(){
    bool b = (__builtin_popcountll(sides[0]) == 1);
    bool w = (__builtin_popcountll(sides[1]) == 1);
    return (toMove and !w and b) or (!toMove and w and !b);
}

bool Bitboards::isChecked(bool sd){
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


Position::Position(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            mps[i][j] += matVals[i];
            eps[i][j] += matVals[i];
        }
    }
    setStartPos();
}

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
    zhist[0] = 0x205E74FA7ED216FEULL;
    //beginZobristHash();
}

void Position::eraseHistoryTable(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            historyTable[0][i][j] = -(1 << 25);
            historyTable[1][i][j] = -(1 << 25);
        }
    }
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

        mprior[ply][totalMoves] = historyTable[toMove][5][f];

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

            int cc;
            for (cc = 1; cc < 6; cc++){ //determine victim
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

            mprior[ply][totalMoves] = (1 << 20) + 5 - (cc << 12);
            
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
                
                int cc;
                for (cc = 1; cc < 6; cc++){ //determine victim
                    if ((1ULL << p) & pieces[cc]){ 
                        moves[ply][totalMoves] |= (1U << 12);
                        moves[ply][totalMoves] |= (cc << 13);
                        break;   
                    }
                }

                moves[ply][totalMoves] |= (ll << 16); //set bits
                moves[ply][totalMoves] |= (ll << 20);
                moves[ply][totalMoves] |= (toMove << 23);

                mprior[ply][totalMoves] = (1 << 20) + ll - (cc << 12);

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

                mprior[ply][totalMoves] = historyTable[toMove][ll][p];

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

            int cc;
            for (cc = 1; cc < 6; cc++){ //find victim
                if ((1ULL << p) & pieces[cc]){
                    moves[ply][totalMoves] |= (1U << 12);
                    moves[ply][totalMoves] |= (cc << 13);
                    break;
                }
            }
            moves[ply][totalMoves] |= 0x110000U; //set bits about piece type
            moves[ply][totalMoves] |= (toMove << 23);

            mprior[ply][totalMoves] = (1 << 20) + 1 - (cc << 12);

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

            mprior[ply][totalMoves] = historyTable[toMove][1][p];

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

        int cc;
        for (cc = 1; cc < 6; cc++){ //locate victim and set bits
            if ((1ULL << p) & pieces[cc]){
                moves[ply][totalMoves] |= (1U << 12);
                moves[ply][totalMoves] |= (cc << 13);
                break;
            }
        }

        mprior[ply][totalMoves] = (1 << 20) + 0 - (cc << 12);

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

        mprior[ply][totalMoves] = historyTable[toMove][0][p];

        totalMoves++;
        mvst ^= (1ULL << p);
    }
    

    return totalMoves;
}

void Position::beginZobristHash(){
    zhist[thm] = toMove * ztk; 
    chm[thm] = thm;

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

int Position::countReps(){    
    int rind = thm;
    int reps = 1;
    do {
        rind -= 2;
        reps += (zhist[thm] == zhist[rind]);
    } while (chm[rind] and (rind >= 0));

    return reps;
}

void Position::makeMove(Move move, bool ev){
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

    if (ev){
        int psb = mps[tpnd][edsq ^ (!toMove * 56)] - mps[tpmv][stsq ^ (!toMove * 56)];
        int csb = capt * mps[cptp][edsq ^ (toMove * 56)];

        scores[toMove] += psb;
        scores[!toMove] -= csb;

        psb = eps[tpnd][edsq ^ (!toMove * 56)] - eps[tpmv][stsq ^ (!toMove * 56)];
        csb = capt * eps[cptp][edsq ^ (toMove * 56)];

        eScores[toMove] += psb;
        eScores[!toMove] -= csb;        
    }

    toMove = !toMove;
}

void Position::unmakeMove(Move move, bool ev){
    uint8_t stsq = move & 0x3FU; uint8_t edsq = (move >> 6) & 0x3FU; //start square, end square
    uint8_t tpmv = (move >> 16) & 7U; uint8_t tpnd = (move >> 20) & 7U; //typeMoved, typeEnded
    bool capt = (move >> 12) & 1U; uint8_t cptp = (move >> 13) & 7U; //capturing, capturetype

    //color = !toMove

    pieces[tpmv] ^= (1ULL << stsq); //move the piece
    pieces[tpnd] ^= (1ULL << edsq);
    sides[!toMove] ^= ((1ULL << stsq) | (1ULL << edsq));

    //deal with captured piece
    pieces[cptp] ^= ((Bitboard) capt * (1ULL << edsq)); //place back to piece type board
    sides[toMove] ^= ((Bitboard) capt * (1ULL << edsq)); //update opps board

    gamePhase -= (((move >> 19) & 1U) - phases[cptp]);
    thm--;

    if (ev){
        int psb = mps[tpnd][edsq ^ (toMove * 56)] - mps[tpmv][stsq ^ (toMove * 56)];
        int csb = capt * mps[cptp][edsq ^ (!toMove * 56)];

        scores[!toMove] -= psb;
        scores[toMove] += csb;

        psb = eps[tpnd][edsq ^ (toMove * 56)] - eps[tpmv][stsq ^ (toMove * 56)];
        csb = capt * eps[cptp][edsq ^ (!toMove * 56)];

        eScores[!toMove] -= psb;
        eScores[toMove] += csb;
    }

    toMove = !toMove;
}

void Position::passMove(){
    //null move
    toMove = !toMove;
    thm++;
    zhist[thm] = zhist[thm - 1] ^ ztk;
    chm[thm] = chm[thm - 1] + 1;
}

void Position::unpassMove(){
    toMove = !toMove;
    thm--;
}

uint64_t Position::perft(int depth, int ply){
    uint64_t pnodes = 0ULL;
    if (depth == 0){
        return 1ULL;
    }

    int nm = fullMoveGen(ply, false);
    for (int i = 0 ; i < nm; i++){
        makeMove(moves[ply][i], false);
        
        //exStream << "\nconsidering @ ply " << ply << ": " << moveToAlgebraic(moves[ply][i]);
        //exStream << "\ndec: " << moves[ply][i] << '\n';
        //printMoveAsBinary(moves[ply][i], exStream);
        //exStream << "made " << moveToAlgebraic(moves[ply][i]) << '\n';
        //printAllBitboards(exStream);
            
        
        if (isChecked(!toMove)){
            //exStream << "illegal: " << moveToAlgebraic(moves[ply][i]) << '\n';
            //printAsBitboard(phasma, exStream);
            //printAllBitboards(exStream);

            unmakeMove(moves[ply][i], false);
            continue;
        }
        uint64_t additional = perft(depth - 1, ply + 1);
        if (ply == 0){
            std::cout << moveToAlgebraic(moves[ply][i]) << ": " << additional << '\n';
            //printMoveAsBinary(moves[ply][i], std::cout);
            //std::cout << "dec: " << moves[ply][i] << '\n';
        }
        pnodes += additional;
        unmakeMove(moves[ply][i], false);

        //exStream << "unmade " << moveToAlgebraic(moves[ply][i]) << '\n';
        //printAllBitboards(exStream);
        
    }

    return pnodes;
}

std::string Position::makeOpening(int toMake){
    int ni;
    for (int i = 0; i < toMake; i++){
        ni = rand() % fullMoveGen(0, 0);
        makeMove(moves[0][ni], false);
        if (isChecked(!toMove)){
            unmakeMove(moves[0][ni], false);
            i--;
        }
    }

    return makeFen();
}

bool Position::getSide(){ return toMove; }

int Position::halfMoveCount(){ return chm[thm]; }

void Position::sendMove(std::string expr){
    int aux = fullMoveGen(0, 0);
    for (int i = 0; i < aux; i++){
        if (moveToAlgebraic(moves[0][i]) == expr){
            makeMove(moves[0][i], false);
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

std::string Position::makeFen(){
	uint64_t occ = sides[0] | sides[1];
	std::string result = "";

	uint64_t squarebb;

	int emptyCount;

	for (int i = 0; i < 8; i++){ //for each of 8 rows
		emptyCount = 0;
		for (int j = 0; j < 8; j++){ //for each square in each row
			squarebb = 1ULL << (8 * i + j);

			//std::cout << "Considering Square: " << 8 * i + j << '\n';

			if (squarebb & occ){ //if landed on occupied space
				//std::cout << "Intersection Found\n";
				if (emptyCount != 0){ //unload stored empty squares
					result += emptyCount + 48;
					emptyCount = 0;
					//std::cout << "Unloaded Empty Squares\n";
				}

                /*
				for (int k = 0; k < 14; k++){ //find it by looping through bitboards
					if (squarebb & pieces[k]){ //if found which bb has it
						result += names[k]; //append label to FEN
						//std::cout << "Found " << names[k] << '\n';
						break;
					}
				}
                */
                for (int k = 0; k < 6; k++){
                    if (squarebb & pieces[k] & sides[1]){
                        result += names[k + 7];
                    }
                    if (squarebb & pieces[k] & sides[0]){
                        result += names[k];
                    }
                }

				emptyCount = 0; //no longer empty
			} else { //otherwise, count as empty square
				emptyCount++;
				//std::cout << "Is Empty Square\n";
			}
		}
		if (emptyCount != 0){
			result += emptyCount + 48;
		}
		if (i != 7){
			result += '/';
		}
	}

	result += ' ';
	result += (toMove ? 'w' : 'b');
	result += " - - ";
	result += std::to_string(chm[thm]);
	result += " 1";

	return result;
}


