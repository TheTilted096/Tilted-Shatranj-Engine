/*
4th generation movegen, fitted for OOP refactoring
TheTilted096, 5-25-2024

*/

#include "STiltedEngine.cpp"

Engine::Engine(){
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
    mnodes= ~0ULL;
    nodes = 0;
    
    thinkLimit = 0xFFFFFFFF;
    moment = std::chrono::steady_clock::now();

    ttable = new TTentry[0x100000];
    for (int i = 0; i < 0x100000; i++){
        ttable[i] = TTentry(zhist);
    }

    for (int i = 1; i < 64; i++){
        for (int j = 1; j < 128; j++){
            lmrReduces[i][j] = std::max(0.0, floor(lmrCoef[0] + lmrCoef[1] * log(i) * log(j)));
        }
    }
}

Engine::~Engine(){
    delete[] ttable;
}

bool Engine::ownKingBare(){
    return (pieces[toMove * 7] == pieces[toMove * 7 + 6]);
}

uint64_t Engine::hyperbolaQuintessence(int& square){
    uint64_t forward = (pieces[6] | pieces[13]) & (FILE0 << (square & 7));
    uint64_t reverse = __builtin_bswap64(forward);

    forward -= 2 * (1ULL << square);
    reverse -= 2 * (1ULL << (56 ^ square));

    forward ^= __builtin_bswap64(reverse);
    forward &= (FILE0 << (square & 7));

    return forward;
}

void Engine::beginZobristHash(){
    zhist[thm] = 0;
    chm[thm] = thm;

    uint64_t wtb, btb;
    int f, g;

    for (int i = 0; i < 6; i++) {
        g = -1;
        wtb = pieces[i];
        while (wtb) {
            f = __builtin_ctzll(wtb);
            g += (f + 1);

            zhist[thm] ^= zpk[1][i][g];

            wtb >>= f;
            wtb >>= 1;
        }

        g = -1;
        btb = pieces[i + 7];
        while (btb) {
            f = __builtin_ctzll(btb);
            g += (f + 1);

            zhist[thm] ^= zpk[0][i][g];

            btb >>= f;
            btb >>= 1;
        }
    }
}

void Engine::showZobrist(){
    std::cout << "Tranposition Table:\n";
    for (int k = 1; k < 15; k++){
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

void Engine::printAllBitboards(){
    std::cout << "\nWhite's Pieces\n";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 8 * i; k < 8 * i + 8; k++) {
                std::cout << ((pieces[7 + j] >> k) & 1);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }

    std::cout << "\nBlack's Pieces\n";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 8 * i; k < 8 * i + 8; k++) {
                std::cout << ((pieces[j] >> k) & 1);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }

    std::cout << "toMove: " << toMove << '\n';
    std::cout << "Repetitions: " << countReps() << "\n\n";
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

int Engine::fullMoveGen(int ply, bool capsOnly){
    uint64_t moveSet;
    uint64_t tempBoard;
    uint64_t occupied = pieces[6] | pieces[13];

    int numPieces;  // number of piees in bitboard
    int numMoves;   // moves in moveset
    int f, g;       // piece squares
    int p, q;       // destination squares
    int totalNumMoves = 0;

    // pawns
    g = -1;                              // initialize starting square
    tempBoard = pieces[7 * toMove + 5];  // get a copy of the pawns bitboard
    while (tempBoard) {                  // for each pawn
        f = __builtin_ctzll(tempBoard);  // find the next square
        g += (f + 1);

        moveSet = (plt[toMove][g] & (pieces[7 * (!toMove) + 6]));                      // possible captures
        moveSet |= ((((1ULL << (g + 8 - toMove * 16)) & (~occupied))) * (!capsOnly));  // possible push

        q = -1;
        while (moveSet) {  // if there are captures
            p = __builtin_ctzll(moveSet);
            q += (p + 1);

            moves[ply][totalNumMoves] = g;          // start
            moves[ply][totalNumMoves] |= (q << 6);  // end

            if ((pieces[7 * (!toMove) + 6] & (1ULL << q))) {         // if is a capture
                for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                    if (pieces[7 * (!toMove) + cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
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
        tempBoard = pieces[7 * toMove + ll];
        while (tempBoard) {
            f = __builtin_ctzll(tempBoard);
            g += (f + 1);

            moveSet = llt[ll][g];                // lookup moveset
            moveSet &= ~pieces[7 * toMove + 6];  // and not friendly pieces
            if (capsOnly) {
                moveSet &= pieces[7 * (!toMove) + 6];
            }
            q = -1;
            while (moveSet) {
                p = __builtin_ctzll(moveSet);
                q += (p + 1);

                moves[ply][totalNumMoves] = g;          // start
                moves[ply][totalNumMoves] |= (q << 6);  // end

                if ((pieces[7 * (!toMove) + 6] & (1ULL << q))) {         // if is a capture
                    for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                        if (pieces[7 * (!toMove) + cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
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
    tempBoard = pieces[7 * toMove + 1];
    while (tempBoard) {
        f = __builtin_ctzll(tempBoard);
        g += (f + 1);

        // vertical, horizontal, avoid capturing friends
        moveSet = hyperbolaQuintessence(g);
        moveSet |= ((uint64_t)hlt[g & 7][((((RANK0 << ((g & 56))) & occupied) >> (g & 56)) >> 1) & 63]) << (g & 56);
        moveSet &= ~pieces[7 * toMove + 6];

        if (capsOnly) {
            moveSet &= pieces[7 * (!toMove) + 6];
        }

        q = -1;
        while (moveSet) {
            p = __builtin_ctzll(moveSet);
            q += (p + 1);

            moves[ply][totalNumMoves] = g;
            moves[ply][totalNumMoves] |= (q << 6);

            // std::cout << "steps 1 2\n";

            if ((pieces[7 * (!toMove) + 6] & (1ULL << q))) {
                // printAsBitboard(pieces[7 * !toMove + 6]);
                for (int cc = 0; cc < 6; cc++) {                     // find the captured piece, if any by scanning through opp bbs
                    if (pieces[7 * (!toMove) + cc] & (1ULL << q)) {  // if enemy bitboard intersect with dest square
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

bool Engine::isChecked(){  // checks if the opposing side is left in check; called after makeMove
    uint64_t phantomSet;
    int kingSquare = __builtin_ctzll(pieces[7 * !toMove]);
    //! toMove = friends, toMove = enemy

    // rook checks
    phantomSet = hyperbolaQuintessence(kingSquare);
    phantomSet |= ((uint64_t)hlt[kingSquare & 7][((((RANK0 << ((kingSquare & 56))) & (pieces[6] | pieces[13])) >> (kingSquare & 56)) >> 1) & 63])
                  << (kingSquare & 56);         // rook moveset
    if (phantomSet & pieces[7 * toMove + 1]) {  // if intersects with opp rooks, in check
        return true;
    }

    // pawn checks
    if (plt[!toMove][kingSquare] & pieces[7 * toMove + 5]) {
        return true;
    }

    int leapers[4] = {2, 3, 0, 4};
    for (int ll : leapers) {
        if (llt[ll][kingSquare] & pieces[7 * toMove + ll]) {
            return true;
        }
    }
    return false;
}

void Engine::makeMove(uint32_t move, bool forward, bool ev){
    if (move == 0) {
        toMove = !toMove;
        if (forward) {
            thm++;
            zhist[thm] = zhist[thm - 1] ^ ztk;
            chm[thm] = chm[thm - 1] + 1;
        } else {
            thm--;
        }
        return;
    }

    uint8_t startsquare = move & 63U;
    uint8_t endsquare = (move >> 6) & 63U;

    uint8_t typeMoved = (move >> 16) & (7U);
    uint8_t typeEnded = (move >> 20) & (7U);

    bool capturing = (move >> 12) & (1U);
    uint8_t captureType = (move >> 13) & (7U);

    bool color = (move >> 23);

    pieces[7 * color + 6] ^= ((1ULL << startsquare) | (1ULL << endsquare));  // update location of own pieces
    pieces[7 * color + typeMoved] ^= (1ULL << startsquare);                  // remove piece its starting square & type
    pieces[7 * color + typeEnded] ^= (1ULL << endsquare);                    // place the piece on its ending square & type

    if (capturing) {                                                // if capturing
        pieces[7 * (!color) + captureType] ^= (1ULL << endsquare);  // remove the captured piece from its square
        pieces[7 * (!color) + 6] ^= (1ULL << endsquare);            // update the location of opp's pieces by removing the victim's bit
    }

    // increment gamephase
    // std::cout << "before " << moveToAlgebraic(move) << "gamephase: " << inGamePhase << '\n';
    inGamePhase += (((move >> 19) & 1U) - phases[captureType]) * (2 * forward - 1);
    // std::cout << "after " << moveToAlgebraic(move) << "gamephase: " << inGamePhase << '\n';

    if (forward) {
        nodes++;
        thm++;  // increment totalHalfMoves

        uint64_t zFactor = 0;
        zFactor ^= zpk[toMove][typeMoved][startsquare];
        zFactor ^= zpk[toMove][typeEnded][endsquare];
        if (capturing) {
            zFactor ^= zpk[!toMove][captureType][endsquare];
        }
        zFactor ^= ztk;
        zhist[thm] = zhist[thm - 1] ^ zFactor;

        if (capturing or (typeMoved == 5)) {
            chm[thm] = 0;
        } else {
            chm[thm] = chm[thm - 1] + 1;
        }
    } else {
        thm--;
    }

    if (ev) {
        int psb = mps[typeEnded][endsquare ^ (!color * 56)] - mps[typeMoved][startsquare ^ (!color * 56)];
        int csb = capturing * mps[captureType][endsquare ^ (color * 56)];

        scores[color] += (forward ? psb : -psb);
        scores[!color] -= (forward ? csb : -csb);

        psb = eps[typeEnded][endsquare ^ (!color * 56)] - eps[typeMoved][startsquare ^ (!color * 56)];
        csb = capturing * eps[captureType][endsquare ^ (color * 56)];

        eScores[color] += (forward ? psb : -psb);
        eScores[!color] -= (forward ? csb : -csb);
    }

    toMove = !toMove;
}

uint64_t Engine::perft(int depth, int ply){
    uint64_t pnodes = 0;
    if (depth == 0) {
        return 1ULL;
    }
    int nm = fullMoveGen(ply, 0);
    for (int i = 0; i < nm; i++) {
        makeMove(moves[ply][i], 1, 0);
        if (isChecked()) {
            makeMove(moves[ply][i], 0, 0);
            continue;
        }
        uint64_t additional = perft(depth - 1, ply + 1);

        if (ply == 0) {
            std::cout << moveToAlgebraic(moves[ply][i]) << ": " << additional << '\n';
            // printMoveAsBinary(moves[ply][i]);
        }

        pnodes += additional;
        makeMove(moves[ply][i], 0, 0);
    }
    return pnodes;
}

void Engine::setStartPos(){
    pieces[0] = 8ULL;
    pieces[1] = 129ULL;
    pieces[2] = 66ULL;
    pieces[3] = 16ULL;
    pieces[4] = 36ULL;
    pieces[5] = 0xFF00ULL;
    pieces[6] = 0xFFFFULL;

    pieces[7] = 0x0800000000000000ULL;
    pieces[8] = 0x8100000000000000ULL;
    pieces[9] = 0x4200000000000000ULL;
    pieces[10] = 0x1000000000000000ULL;
    pieces[11] = 0x2400000000000000ULL;
    pieces[12] = 0x00FF000000000000ULL;
    pieces[13] = 0xFFFF000000000000ULL;

    toMove = true;
    inGamePhase = 64;
    thm = 0;
    chm[0] = 0;
}

void Engine::sendMove(std::string expr){
    int aux = fullMoveGen(0, 0);
    for (int i = 0; i < aux; i++){
        if (moveToAlgebraic(moves[0][i]) == expr){
            makeMove(moves[0][i], 1, 0);
            break;
        }
    }
}

uint32_t Engine::getMove(){ return bestMove; }
bool Engine::getSide(){ return toMove; }

std::string Engine::makeRandMoves(int toMake){
    int ni;
    for (int i = 0; i < toMake; i++){
        ni = rand() % fullMoveGen(0, 0);
        makeMove(moves[0][ni], 1, 0);
        if (isChecked()){
            makeMove(moves[0][ni], 0, 0);
            i--;
        }
    }

    return makeFen();
}

void Engine::readFen(std::string fen){
    for (int i = 0; i < 14; i++){
        pieces[i] = 0;
    }

    uint8_t sq = 0;
    int ind = -1;
    uint8_t p;

    char posChars[] = {'/', '1', '2', '3', '4', '5', '6', '7', '8',
                       'k', 'r', 'n', 'q', 'b', 'p', 'K', 'R', 'N', 'Q', 'B', 'P'};
    while (sq < 64) {
        ind++;
        for (int i = 0; i < 21; i++){  // quantify the character
            if (fen[ind] == posChars[i]){
                p = i;
                break;
            }
        }
        if ((0 < p) and (p < 9)){
            sq += p;
            continue;
        }
        if ((p >= 9) and (p <= 14)){
            pieces[p - 9] |= (1ULL << sq);
            pieces[6] |= (1ULL << sq);
            sq++;
        }
        if ((p >= 15) and (p <= 20)){
            pieces[p - 8] |= (1ULL << sq);
            pieces[13] |= (1ULL << sq);
            sq++;
        }
    }

    inGamePhase = 0;               // calculate gamephase
    for (int i = 1; i < 6; i++){  // note phases[0] = 0 (king)
        inGamePhase += (__builtin_popcountll(pieces[i] | pieces[i + 7])) * phases[i];
    }

    ind += 2;               // skip over a space
    toMove = fen[ind] & 1;  //'w' = 119, and 'b' = 98.

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

std::string Engine::makeFen(){
	uint64_t occ = pieces[6] | pieces[13];
	std::string result = "";

	uint64_t squarebb;

	char names[14] = {'k', 'r', 'n', 'q', 'b', 'p', 'x',
						'K', 'R', 'N', 'Q', 'B', 'P', 'X'};

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

				for (int k = 0; k < 14; k++){ //find it by looping through bitboards
					if (squarebb & pieces[k]){ //if found which bb has it
						result += names[k]; //append label to FEN
						//std::cout << "Found " << names[k] << '\n';
						break;
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

