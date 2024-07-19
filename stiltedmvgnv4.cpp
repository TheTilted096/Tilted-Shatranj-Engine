/*
4th generation movegen, fitted for OOP refactoring
TheTilted096, 5-25-2024

*/

#include "stiltedengin.h"

void Bitboards::printAsBitboard(Bitboard board){
    for (int i = 0; i < 64; i++) {
        std::cout << (board & 1ULL);
        if ((i & 7) == 7) {
            std::cout << '\n';
        }
        board >>= 1;
    }
    std::cout << '\n';
}

void Bitboards::printMoveAsBinary(Move move){
    int spaces[8] = {25, 19, 18, 15, 12, 11, 8, 7};
    uint32_t msb = 1 << 31;
    for (int i = 0; i < 32; i++) {
        std::cout << move / msb;
        move <<= 1;
        for (int j : spaces) {
            if (i == j) {
                std::cout << ' ';
                break;
            }
        }
    }
    std::cout << '\n';
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

bool Bitboards::kingBare(){ //returns true if 'toMove' king is bare, false otherwise, and false when both bare.
    bool b = (__builtin_popcountll(sides[0]) == 1);
    bool w = (__builtin_popcountll(sides[1]) == 1);

    return ((toMove and !w and b) or (!toMove and w and !b));
    // (black tM and white not bare and black bare) OR (white tM and white bare and black not bare);
}

bool Bitboards::ownKingBare(){
    return (__builtin_popcountll(sides[toMove]) == 1);
}

bool Bitboards::isChecked(bool sd){  // checks if the opposing side is left in check; called after makeMove
    Bitboard inc;
    int ksq = __builtin_ctzll(sides[sd] & pieces[0]);

    Bitboard rookSet = hqRookAttack(ksq, sides[0] | sides[1]);
    //rookSet |= ((uint64_t)hlt[ksq & 7][((((RANK0 << ((ksq & 56))) & (sides[0] | sides[1])) >> (ksq & 56)) >> 1) & 63])
    //              << (ksq & 56); 

    //std::cout << "kingSquare: ksq\n";
    inc = llt[0][ksq] & pieces[0] & sides[!sd];
    inc |= rookSet & pieces[1] & sides[!sd];
    inc |= llt[2][ksq] & pieces[2] & sides[!sd];
    inc |= llt[3][ksq] & pieces[3] & sides[!sd];
    inc |= llt[4][ksq] & pieces[4] & sides[!sd];
    inc |= plt[sd][ksq] & pieces[5] & sides[!sd];

    //phasma = inc;

    return inc; 
}

void Bitboards::printAllBitboards(){
    std::cout << "sides[0] (black)\tsides[1] (white)\n";
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[0] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << "\t\t";
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[1] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << '\n';
    }

    std::cout << "\n\npieces[0]\tpieces[1]\tpieces[2]\tpieces[3]\tpieces[4]\tpieces[5]\n";
    for (int i = 0; i < 8; i++){
        for (int k = 0; k < 6; k++){
            for (int j = 0; j < 8; j++){
                std::cout << ((pieces[k] >> ((i << 3) + j)) & 1ULL);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    std::cout << "toMove: " << toMove << "\n\n";
}

Bitboard Bitboards::hqRookAttack(int& sq, Bitboard occ){
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

Position::Position(){
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            mps[i][j] += matVals[i];
            eps[i][j] += matVals[i];

            historyTable[0][i][j] = 0;
            historyTable[1][i][j] = 0;
        }
    }
    setStartPos();
    beginZobristHash();
    nodes = 0ULL;
}

void Position::beginZobristHash(){
    zhist[thm] = 0;
    chm[thm] = thm;

    Bitboard tpbd;
    int f;

    for (int i = 0; i < 6; i++) {
        tpbd = pieces[i] & sides[1];
        while (tpbd) {
            f = __builtin_ctzll(tpbd);
            zhist[thm] ^= zpk[1][i][f];
            tpbd ^= (1ULL << f);
        }

        tpbd = pieces[i] & sides[0];
        while (tpbd) {
            f = __builtin_ctzll(tpbd);
            zhist[thm] ^= zpk[0][i][f];
            tpbd ^= (1ULL << f);
        }
    }
}

void Position::eraseHistoryTable(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            historyTable[0][i][j] = 0;
            historyTable[1][i][j] = 0;
        }
    }
}

int Position::halfMoveCount(){
    return chm[thm];
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

/*
Move Representation:

0000 0000 0000 0000 000 0 000000 000000

0-5: start square
6-11: end square

12: Capture
13-15: Captured Type
16-18: Piece Type Moved
19: Promotion
20-22: Piece End Type
23: Color
*/

int Position::fullMoveGen(int ply, bool capsOnly){
    Bitboard moveSet;
    Bitboard tempBoard;
    Bitboard occupied = sides[0] | sides[1];

    int numPieces;  // number of piees in bitboard
    int numMoves;   // moves in moveset
    int f, g;       // piece squares
    int p, q;       // destination squares
    int totalNumMoves = 0;

    // pawns
    g = -1;                              // initialize starting square
    tempBoard = sides[toMove] & pieces[5];  // get a copy of the pawns bitboard
    while (tempBoard) {                  // for each pawn
        f = __builtin_ctzll(tempBoard);  // find the next square
        g += (f + 1);

        moveSet = (plt[toMove][g] & (sides[!toMove]));      // possible captures
        moveSet |= ((((1ULL << (g + 8 - toMove * 16)) & (~occupied))) * (!capsOnly));  // possible push

        q = -1;
        while (moveSet) {  // if there are captures
            p = __builtin_ctzll(moveSet);
            q += (p + 1);

            moves[ply][totalNumMoves] = g;          // start
            moves[ply][totalNumMoves] |= (q << 6);  // end

            if ((sides[!toMove] & (1ULL << q))) {         // if is a capture
                for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                    if (sides[!toMove] & pieces[cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
                        moves[ply][totalNumMoves] |= (4096U);        // capture bit set
                        moves[ply][totalNumMoves] |= (cc << 13);     // set bits accordingly and exit loop
                        break;
                    }
                }
            }

            moves[ply][totalNumMoves] |= (5U << 16);
            if ((1ULL << q) & ((toMove) ? RANK0 : RANK7)) {  // if promoting
                moves[ply][totalNumMoves] |= (7U << 19);
                // std::cout << "promotion\n";
            } else {
                moves[ply][totalNumMoves] |= (5U << 20);
            }

            moves[ply][totalNumMoves] |= (toMove << 23);

            moveSet >>= p;
            moveSet >>= 1;
            totalNumMoves++;
        }
        tempBoard >>= f;
        tempBoard >>= 1;
    }

    int leapers[4] = {4, 3, 2, 0};
    for (int ll : leapers) {
        g = -1;
        tempBoard = sides[toMove] & pieces[ll];
        while (tempBoard) {
            f = __builtin_ctzll(tempBoard);
            g += (f + 1);

            moveSet = llt[ll][g];                // lookup moveset
            moveSet &= ~sides[toMove];  // and not friendly pieces
            if (capsOnly) {
                moveSet &= sides[!toMove];
            }
            q = -1;
            while (moveSet) {
                p = __builtin_ctzll(moveSet);
                q += (p + 1);

                moves[ply][totalNumMoves] = g;          // start
                moves[ply][totalNumMoves] |= (q << 6);  // end

                if ((sides[!toMove] & (1ULL << q))) {         // if is a capture
                    for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                        if (sides[!toMove] & pieces[cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
                            moves[ply][totalNumMoves] |= (4096U);        // capture bit set
                            moves[ply][totalNumMoves] |= (cc << 13);     // set bits accordingly and exit loop
                            break;
                        }
                    }
                }

                moves[ply][totalNumMoves] |= (ll << 16);
                moves[ply][totalNumMoves] |= (ll << 20);
                moves[ply][totalNumMoves] |= (toMove << 23);

                totalNumMoves++;
                moveSet >>= p;
                moveSet >>= 1;
            }
            tempBoard >>= f;
            tempBoard >>= 1;
        }

        // std::cout << "leaper " << ll << " finished\n";
    }

    // rooks
    g = -1;
    tempBoard = pieces[1] & sides[toMove];
    while (tempBoard) {
        f = __builtin_ctzll(tempBoard);
        g += (f + 1);

        // vertical, horizontal, avoid capturing friends
        moveSet = hqRookAttack(g, occupied);
        //moveSet |= ((uint64_t)hlt[g & 7][((((RANK0 << ((g & 56))) & occupied) >> (g & 56)) >> 1) & 63]) << (g & 56);
        moveSet &= ~sides[toMove];

        if (capsOnly) {
            moveSet &= sides[!toMove];
        }

        q = -1;
        while (moveSet) {
            p = __builtin_ctzll(moveSet);
            q += (p + 1);

            moves[ply][totalNumMoves] = g;
            moves[ply][totalNumMoves] |= (q << 6);

            // std::cout << "steps 1 2\n";

            if ((sides[!toMove] & (1ULL << q))) {
                // printAsBitboard(pieces[7 * !toMove + 6]);
                for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                    if (sides[!toMove] & pieces[cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
                        moves[ply][totalNumMoves] |= (4096U);        // capture bit set
                        moves[ply][totalNumMoves] |= (cc << 13);     // set bits accordingly and exit loop
                        break;
                    }
                }
            }
            moves[ply][totalNumMoves] |= 0x110000U;
            moves[ply][totalNumMoves] |= (toMove << 23);

            moveSet >>= p;
            moveSet >>= 1;
            totalNumMoves++;
        }
        tempBoard >>= f;
        tempBoard >>= 1;
    }

    return totalNumMoves;
}

void Position::makeMove(Move move, bool ev){
    uint8_t startsquare = move & 63U;
    uint8_t endsquare = (move >> 6) & 63U;

    uint8_t typeMoved = (move >> 16) & (7U);
    uint8_t typeEnded = (move >> 20) & (7U);

    bool capturing = (move >> 12) & (1U);
    uint8_t captureType = (move >> 13) & (7U);

    //bool color = (move >> 23);

    pieces[captureType] ^= capturing * (1ULL << endsquare); // remove the captured piece from its square
    sides[!toMove] ^= capturing * (1ULL << endsquare); // update the location of opp's pieces by removing the victim's bit

    sides[toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));  // update location of own pieces
    pieces[typeMoved] ^= (1ULL << startsquare);                  // remove piece its starting square & type
    pieces[typeEnded] ^= (1ULL << endsquare);                    // place the piece on its ending square & type

    // increment inGamePhase
    // std::cout << "before " << moveToAlgebraic(move) << "inGamePhase: " << inGamePhase << '\n';
    inGamePhase += (((move >> 19) & 1U) - phases[captureType]);
    // std::cout << "after " << moveToAlgebraic(move) << "inGamePhase: " << inGamePhase << '\n';

    nodes++;
    thm++;  // increment totalHalfMoves

    uint64_t zFactor = 0;
    zFactor ^= zpk[toMove][typeMoved][startsquare];
    zFactor ^= zpk[toMove][typeEnded][endsquare];

    zFactor ^= capturing * zpk[!toMove][captureType][endsquare];
    
    zFactor ^= ztk;
    zhist[thm] = zhist[thm - 1] ^ zFactor;

    chm[thm] = !(capturing or (typeMoved == 5)) * (chm[thm - 1] + 1);

    if (ev) {
        int psb = mps[typeEnded][endsquare ^ (!toMove * 56)] - mps[typeMoved][startsquare ^ (!toMove * 56)];
        int csb = capturing * mps[captureType][endsquare ^ (toMove * 56)];

        scores[toMove] += psb;
        scores[!toMove] -= csb;

        psb = eps[typeEnded][endsquare ^ (!toMove * 56)] - eps[typeMoved][startsquare ^ (!toMove * 56)];
        csb = capturing * eps[captureType][endsquare ^ (toMove * 56)];

        eScores[toMove] += psb;
        eScores[!toMove] -= csb;
    }

    toMove = !toMove;
}

void Position::unmakeMove(Move move, bool ev){
    uint8_t startsquare = move & 63U;
    uint8_t endsquare = (move >> 6) & 63U;

    uint8_t typeMoved = (move >> 16) & (7U);
    uint8_t typeEnded = (move >> 20) & (7U);

    bool capturing = (move >> 12) & (1U);
    uint8_t captureType = (move >> 13) & (7U);

    //bool color = (move >> 23);

    sides[!toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));  // update location of own pieces
    pieces[typeMoved] ^= (1ULL << startsquare);                  // remove piece its starting square & type
    pieces[typeEnded] ^= (1ULL << endsquare);                    // place the piece on its ending square & type

    pieces[captureType] ^= capturing * (1ULL << endsquare);
    sides[toMove] ^= capturing * (1ULL << endsquare);

    // increment inGamePhase
    // std::cout << "before " << moveToAlgebraic(move) << "inGamePhase: " << inGamePhase << '\n';
    inGamePhase -= (((move >> 19) & 1U) - phases[captureType]);
    // std::cout << "after " << moveToAlgebraic(move) << "inGamePhase: " << inGamePhase << '\n';

    thm--;

    if (ev) {
        int psb = mps[typeEnded][endsquare ^ (toMove * 56)] - mps[typeMoved][startsquare ^ (toMove * 56)];
        int csb = capturing * mps[captureType][endsquare ^ (!toMove * 56)];

        scores[!toMove] -= psb;
        scores[toMove] += csb;

        psb = eps[typeEnded][endsquare ^ (toMove * 56)] - eps[typeMoved][startsquare ^ (toMove * 56)];
        csb = capturing * eps[captureType][endsquare ^ (!toMove * 56)];

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
    uint64_t pnodes = 0;
    if (depth == 0) {
        return 1ULL;
    }
    int nm = fullMoveGen(ply, 0);
    for (int i = 0; i < nm; i++) {
        makeMove(moves[ply][i], false);
        //std::cout << moveToAlgebraic(moves[ply][i]) << '\n';

        if (isChecked(!toMove)) {
            unmakeMove(moves[ply][i], false);
            continue;
        }
        uint64_t additional = perft(depth - 1, ply + 1);

        if (ply == 0) {
            std::cout << moveToAlgebraic(moves[ply][i]) << ": " << additional << '\n';
            // printMoveAsBinary(moves[ply][i]);
        }

        pnodes += additional;
        unmakeMove(moves[ply][i], false);
    }
    return pnodes;
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
    inGamePhase = 64;
    thm = 0; chm[thm] = 0;
    //zhist[0] = 0x205E74FA7ED216FEULL;
    //beginZobristHash();
}

void Position::sendMove(std::string expr){
    int aux = fullMoveGen(0, 0);
    for (int i = 0; i < aux; i++){
        if (moveToAlgebraic(moves[0][i]) == expr){
            makeMove(moves[0][i], false);
            break;
        }
    }
}

uint32_t Engine::getMove(){ return bestMove; }
bool Bitboards::getSide(){ return toMove; }

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

    inGamePhase = 0;
    inGamePhase += __builtin_popcountll(pieces[1]) * phases[1];
    inGamePhase += __builtin_popcountll(pieces[2]) * phases[2];
    inGamePhase += __builtin_popcountll(pieces[3]) * phases[3];
    inGamePhase += __builtin_popcountll(pieces[4]) * phases[4];
    inGamePhase += __builtin_popcountll(pieces[5]) * phases[5];

    ind += 2; //skip a space
    toMove = fen[ind] & 1; //'w' = 119 and 'b' = 98;

    std::stringstream extraMoves(fen.substr(ind + 5));

    std::string fenTerm;
    extraMoves >> fenTerm;
    thm = 0;
    chm[thm] = stoi(fenTerm);  // get total half move counter

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
