/*
3rd generation Search-Eval file fitted to OOP design
TheTilted096, 5-25-2024

*/

#include "STiltedEngine.h"


int Position::evaluateScratch(){
    scores[1] = 0; scores[0] = 0;
    eScores[1] = 0; eScores[0] = 0;

    //kings
    scores[1] += mps[0][__builtin_ctzll(sides[1] & pieces[0])];
    eScores[1] += eps[0][__builtin_ctzll(sides[1] & pieces[0])];

    scores[0] += mps[0][56 ^ __builtin_ctzll(sides[0] & pieces[0])];
    eScores[0] += eps[0][56 ^ __builtin_ctzll(sides[0] & pieces[0])];

    Bitboard tpbd;
    int f;

    for (int i = 1; i < 6; i++){
        tpbd = pieces[i] & sides[1];
        while (tpbd){
            f = __builtin_ctzll(tpbd);

            scores[1] += mps[i][f];
            eScores[1] += eps[i][f];

            tpbd ^= (1ULL << f);
        }

        tpbd = pieces[i] & sides[0];
        while (tpbd){
            f = __builtin_ctzll(tpbd);

            scores[0] += mps[i][56 ^ f];
            eScores[0] += eps[i][56 ^ f];

            tpbd ^= (1ULL << f);
        }
    }

    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[!toMove];

    return (mdiff * inGamePhase + ediff * (64 - inGamePhase)) / 64;
}

int Position::evaluate(){
    //int mdiff = scores[toMove] - scores[!toMove];
    //int ediff = eScores[toMove] - eScores[!toMove];

    int mdiff = scores[toMove] + mobil[toMove] - scores[!toMove] - mobil[!toMove];
    int ediff = eScores[toMove] + emobil[toMove] - eScores[!toMove] - emobil[!toMove];

    return (mdiff * inGamePhase + ediff * (Position::totalGamePhase - inGamePhase)) / Position::totalGamePhase;
}

void Position::beginAttacks(bool sd){
    Bitboard pcs;
    int f;
    pcs = sides[sd] & pieces[5]; //for each of our pawns
    atktbl[sd][4] = 0ULL; //empty pawn attacks
    while (pcs){ 
        f = __builtin_ctzll(pcs);
        atktbl[sd][4] |= plt[sd][f]; //or in our pawn attacks
        pcs ^= (1ULL << f);
    }

    for (int ll = 4; ll > 1; ll--){ //for each leaper
        pcs = sides[sd] & pieces[ll]; //get leapers
        atktbl[sd][ll - 1] = atktbl[sd][ll]; //carry over previous attacks
        while (pcs){
            f = __builtin_ctzll(pcs);
            atktbl[sd][ll - 1] |= llt[ll][f]; //or in attack set
            pcs ^= (1ULL << f);
        }
    }

    pcs = sides[sd] & pieces[1]; //rooks
    atktbl[sd][0] = atktbl[sd][1]; //carry over
    while (pcs){ 
        f = __builtin_ctzll(pcs); //or attacks
        atktbl[sd][0] |= RookBoards[RookOffset[f] + _pext_u64(sides[0] | sides[1], RookMasks[f])];
        pcs ^= (1ULL << f);
    }

    f = __builtin_ctzll(sides[sd] & pieces[0]); //our king
    atktbl[sd][0] |= llt[0][f];
}

void Position::beginMobility(bool sd){
    beginAttacks(!sd);

    mobil[sd] = 0; emobil[sd] = 0;

    Bitboard pcs, sfst;
    int scnt, f;

    for (int ll = 4; ll > 1; ll--){
        pcs = sides[sd] & pieces[ll];
        while (pcs){
            f = __builtin_ctzll(pcs);

            sfst = llt[ll][f] & ~(atktbl[!sd][ll] | sides[sd]);
            scnt = __builtin_popcountll(sfst);
            mobil[sd] += mobVals[mIndx[ll] + scnt];
            emobil[sd] += mobValsE[mIndx[ll] + scnt];

            pcs ^= (1ULL << f);            
        }
    }

    pcs = sides[sd] & pieces[1];
    while (pcs){
        f = __builtin_ctzll(pcs);
        Bitboard rtb = RookBoards[RookOffset[f] + _pext_u64(sides[0] | sides[1], RookMasks[f])];

        sfst = rtb & ~(atktbl[!sd][1] | sides[sd]);
        scnt = __builtin_popcountll(sfst);
        mobil[sd] += mobVals[mIndx[1] + scnt];
        emobil[sd] += mobValsE[mIndx[1] + scnt];

        pcs ^= (1ULL << f);
    }

    f = __builtin_ctzll(sides[sd] & pieces[0]);
    sfst = llt[0][f] & ~(atktbl[!sd][0] | sides[sd]);
    scnt = __builtin_popcountll(sfst);
    mobil[sd] += mobVals[mIndx[0] + scnt];
    emobil[sd] += mobValsE[mIndx[0] + scnt];
}

int* Position::availableMobs(){
    beginMobility(true); beginMobility(false);

    Bitboard pcs, sfst, mvst; //pieces, safeset, moveset
    int scnt, f; //safe count, f = bit
    bool ca; //can add
    int* ml = new int[44]; //creatl ist
    ml[0] = 0; //first index is number of elements

    for (int pp = 0; pp < 5; pp++){
        pcs = pieces[pp] & sides[1]; //white

        while (pcs){ //for each white piece
            f = __builtin_ctzll(pcs); //get bit and moveset
            if (pp == 1){ //if rook
                mvst = RookBoards[RookOffset[f] + _pext_u64(sides[0] | sides[1], RookMasks[f])];
            } else {
                mvst = llt[pp][f];
            }

            sfst = mvst & ~(atktbl[0][pp] | sides[1]); //safeset = moveset and not (attacked or occupied)
            scnt = __builtin_popcountll(sfst) + mIndx[pp];

            ca = true; //assume can add
            for (int c = 0; c < ml[0]; c++){ //for each element in list
                if (scnt == ml[c + 1]){ //
                    ca = false;
                    break;
                }
            }

            if (ca){
                ml[0]++;
                ml[ml[0]] = scnt;
            }

            pcs ^= (1ULL << f);
        }
    
        pcs = pieces[pp] & sides[0]; //black

        while (pcs){ //for each white piece
            f = __builtin_ctzll(pcs); //get bit and moveset
            if (pp == 1){ //if rook
                mvst = RookBoards[RookOffset[f] + _pext_u64(sides[0] | sides[1], RookMasks[f])];
            } else {
                mvst = llt[pp][f];
            }

            sfst = mvst & ~(atktbl[1][pp] | sides[0]); //safeset = moveset and not (attacked or occupied)
            scnt = __builtin_popcountll(sfst) + mIndx[pp];

            ca = true; //assume can add
            for (int c = 0; c < ml[0]; c++){ //for each element in list
                if (scnt == ml[c + 1]){ //
                    ca = false;
                    break;
                }
            }

            if (ca){
                ml[0]++;
                ml[ml[0]] = scnt;
            }

            pcs ^= (1ULL << f);
        }
    }

    return ml;    
}

TTentry::TTentry(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
    zref = nullptr;
}

void TTentry::reset(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
}

void TTentry::setReference(uint64_t* zr){
    zref = zr;
}

void TTentry::update(int& sc, int nt, int& d, Move dm, int thm){
    if ((d >= eDepth) or (zref[thm] != eHash)){
        eScore = sc;
        enType = nt;
        eDepth = d;
        eHash = zref[thm];
        eMove = dm;
    }
}

void TTentry::print(){
    std::cout << "Index: " << (eHash & 0xFFFFF) << "     Score: " << eScore
        << "\tdepthAt: " << eDepth << "     nodeType: " << enType << "     Move: " 
            << Bitboards::moveToAlgebraic(eMove) << '\n';
}


Engine::Engine(){
    //std::cout << "Engine Constructed\n";
    mnodes= ~0ULL;
    nodesForever = 0ULL;
    
    thinkLimit = 0xFFFFFFFF;
    moment = std::chrono::steady_clock::now();

    ttable = new TTentry[0x100000];
    for (int i = 0; i < 0x100000; i++){
        ttable[i].setReference(zhist);
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

void Engine::showZobrist(){
    /*
    std::cout << "Tranposition Table:\n";
    for (int k = 1; k < 15; k++){
        for (int i = 0; i < 0xFFFFF; i++){
            if (ttable[i].eDepth == k){
                ttable[i].print();
            }
        }
    }
    */
    std::cout << "\nZobrist History + Last 20 Bits\n";
    for (int j = 0; j < thm + 1; j++){
        std::cout << "ZH " << j << ": " << zhist[j] << "\tIndex: " << (zhist[j] & 0xFFFFF) << '\n';
    }
}

void Engine::endHandle(){
    if ((nodes % 6000 == 0) and timeKept and
            (std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::steady_clock::now() - moment).count() >= thinkLimit)){
        throw "Time Expired\n";
    }
    if (nodes > mnodes) {
        throw "Nodes Exceeded\n";
    }
}

void Engine::eraseTransposeTable(){
    for (int i = 0; i < 0x100000; i++){
        if (ttable[i].enType != -1){
            ttable[i].reset();
        }
    }
}

void Engine::eraseKillers(){
    for (int i = 0; i < 64; i++){
        killers[i][0] = 0U;
        killers[i][1] = 0U;
    }
}

void Engine::copyEval(EvalVars e){
    /*
    std::cout << "attempt copyEval\n";
    std::cout << e.rc << '\n';
    std::cout << e.aw << '\n';
    */
    if (e.rc != nullptr){
        //std::cout << "rfpparams copied\n";
        rfpCoef[0] = e.rc[0];
        rfpCoef[1] = e.rc[1];
    }
    if (e.aw != nullptr){
        for (int i = 0; i < 5; i++){
            aspWins[i] = e.aw[i];
        }
    }
}

void Engine::newGame(){
    setStartPos();
    beginZobristHash();
    eraseTransposeTable();
    eraseHistoryTable();
    eraseKillers();
}

void Engine::sortMoves(int nc, int ply){
    int keyVal;
    Move keyMove;

    int j;
    for (int i = 1; i < nc; i++){
        keyVal = mprior[ply][i];
        keyMove = moves[ply][i];
        j = i - 1;
        while ((j >= 0) and (mprior[ply][j] < keyVal)){
            mprior[ply][j + 1] = mprior[ply][j];
            moves[ply][j + 1] = moves[ply][j];
            j--;
        }

        mprior[ply][j + 1] = keyVal;
        moves[ply][j + 1] = keyMove;
    }
}

void Engine::pickMove(int si, int ei, int ply){
    int ibest = si;
    for (int i = si + 1; i < ei; i++){
        
        if (mprior[ply][i] > mprior[ply][ibest]){
            ibest = i;
        }
        
        
        //bool better = mprior[ply][i] > mprior[ply][ibest];
        //ibest += (i - ibest) * better;
    }
    std::swap(mprior[ply][ibest], mprior[ply][si]);
    std::swap(moves[ply][ibest], moves[ply][si]);
}

/*
bool Engine::see(Move mv, double min){
    uint8_t stsq = mv & 63U; //start square 
    uint8_t edsq = (mv >> 6) & 63U; //end square
    bool bkrk = (1ULL << edsq) & 0xFF000000000000FFULL; //square is on back rank
    bool prmt = (mv >> 19) & 1U; //is promoting
    const double prgn = seeVals[3] - seeVals[5];

    uint8_t ao = (mv >> 16) & 7U;  // initial agressor
    uint8_t vo = (mv >> 13) & 7U;  // initial victim

    //double gain = seeVals[vo]; // gain begins as initial victim from perspective of toMove (aggressor)
    seeGains[0] = seeVals[vo] + prmt * prgn;
    double stdn = seeVals[ao - (prmt << 1)]; //piece standing on square

    if (seeGains[0] - stdn >= min){ //if piece was hanging
        return true;
    }

    //initial material check didn't work, time to search all the way

    Bitboard rks = pieces[1]; //save rook bitboard (rook bits are removed)
    Bitboard occ = sides[0] | sides[1]; //occupied squares

    bool istm = !toMove; // internal side to move

    uint32_t attl[2] = {0U, 0U}; //u32 that represent attackers
    int slva[2]; // index of current least valued aggressor

    //get attackers and insert them into the u32 (as a list)
    attl[0] |= (__builtin_popcountll(plt[1][edsq] & pieces[5] & sides[0]) << 20);
    attl[1] |= (__builtin_popcountll(plt[0][edsq] & pieces[5] & sides[1]) << 20);

    Bitboard rl;
    for (int i = 4; i > 1; i--){
        rl = llt[i][edsq] & pieces[i];
        attl[0] |= (__builtin_popcountll(rl & sides[0]) << (i << 2));
        attl[1] |= (__builtin_popcountll(rl & sides[1]) << (i << 2));
    }
    
    Bitboard rlkp = RookBoards[RookOffset[edsq] + _pext_u64(occ, RookMasks[edsq])] & pieces[1]; //rook-look up
    attl[0] |= (__builtin_popcountll(rlkp & sides[0]) << 4);
    attl[1] |= (__builtin_popcountll(rlkp & sides[1]) << 4);

    rl = llt[0][edsq] & pieces[0];
    attl[0] |= (__builtin_popcountll(rl & sides[0]));
    attl[1] |= (__builtin_popcountll(rl & sides[1]));    

    if ((seeGains[0] > min) and (attl[!toMove] == 0U)){ //if hanging
        return true;
    }

    //before playing own move:
    // winning if
    // (gain > min)  OR  (opponent has no attackers AND gain + stdn > min)

    // losing if
    // (no attackers AND gain < min) or (has attackers AND gain + stdn < min)

    //istm will also keep track of the winning side

    while (true){

    }


    
    return 0;
}
*/

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

bool Engine::see(Move mv){
    //check if piece is lower value than aggressor

    uint8_t vo = (mv >> 13) & 7U;
    uint8_t ao = (mv >> 16) & 7U;

    //aggressor less valuable than victim
    if (seeVals[vo] - seeVals[ao] >= 0.0){
        return true;
    }

    //get all the attackers

    uint32_t attl[2] = {0U, 0U}; //attacker list

    uint8_t edsq = (mv >> 6) & 63U; //end square
    uint8_t stsq = mv & 63U;

    Bitboard rks = pieces[1]; //original rook bitboard
    Bitboard occ = sides[0] | sides[1]; // occupancy bitboard
    Bitboard sqbb = (1ULL << stsq); // end square as bitboard

    pieces[ao] ^= sqbb; //remove attacker from consideration
    occ ^= sqbb;

    //store every attacker into u32 lists.
    attl[0] |= (__builtin_popcountll(plt[1][edsq] & pieces[5] & sides[0]) << 20);
    attl[1] |= (__builtin_popcountll(plt[0][edsq] & pieces[5] & sides[1]) << 20);

    Bitboard rl;
    for (int i = 4; i > 1; i--){
        rl = llt[i][edsq] & pieces[i];
        attl[0] |= (__builtin_popcountll(rl & sides[0]) << (i << 2));
        attl[1] |= (__builtin_popcountll(rl & sides[1]) << (i << 2));
    }
    
    Bitboard rlkp = RookBoards[RookOffset[edsq] + _pext_u64(occ, RookMasks[edsq])] & pieces[1]; //rook-look up
    attl[0] |= (__builtin_popcountll(rlkp & sides[0]) << 4);
    attl[1] |= (__builtin_popcountll(rlkp & sides[1]) << 4);

    rl = llt[0][edsq] & pieces[0];
    attl[0] |= (__builtin_popcountll(rl & sides[0]));
    attl[1] |= (__builtin_popcountll(rl & sides[1]));    

    //check if piece was hanging
    if ((attl[!toMove] == 0U) and (seeVals[vo] > 0.0)){
        pieces[ao] |= sqbb;
        return true;
    }

    //std::cout << "Attackers[2]: " << attl[1] << ", " << attl[0] << '\n';

    // main loop

    bool bkrk = (1ULL << edsq) & 0xFF000000000000FFULL; //square is on back rank
    const double prgn = seeVals[3] - seeVals[5]; // ferz - pawn value
    bool prmt = (mv >> 19) & 1U; // is promotion

    bool istm = !toMove; //internal-side-to-move
    uint8_t lva = 6;
    double stdn = seeVals[ao - (prmt << 1)]; //value of piece standing on square

    Bitboard rpop;

    double gains[2] = {0.0, 0.0};
    
    gains[toMove] = seeVals[vo] + prmt * prgn; //initial value of gain

    //std::cout << "gains[2] before loop: " << gains[1] << ", " << gains[0] << '\n';

    while (attl[istm] != 0U){ //while there are still defenders
        //simulate move

        gains[istm] += stdn; //add captured piece to gain

        lva = (31 - __builtin_clz(attl[istm])) >> 2; //get lva
        prmt = (lva == 5) and bkrk; //check for promotion
        stdn = seeVals[lva - (prmt << 1)]; //get new value of piece standing on square

        attl[istm] -= (1U << (lva << 2)); //remove attacking piece

        if (lva == 1){
            rpop = (1ULL << __builtin_ctzll(rlkp & sides[istm])); // capturing rook bit
            //std::cout << "rpop: " << __builtin_ctzll(rpop) << '\n';
            pieces[1] ^= rpop; // remove from rook bitboard
            occ ^= rpop; // and occupancy bitboard
            //find other rook attackers without the attacking rook
            rlkp = RookBoards[RookOffset[edsq] + _pext_u64(occ, RookMasks[edsq])] & pieces[1];

            // input re-calculated rook count
            attl[0] &= 0xFFFFFF0FU;
            attl[1] &= 0xFFFFFF0FU;
            attl[0] |= (__builtin_popcountll(rlkp & sides[0]) << 4);
            attl[1] |= (__builtin_popcountll(rlkp & sides[1]) << 4);      
        }

        // check if moving side is winning
        //if gain > min + standing  OR  gain > min with no defenders        
        //std::cout << "gains[2] in loop: " << gains[1] << ", " << gains[0] << '\n';
        //std::cout << "Opponent: " << attl[!istm] << '\n';

        /*
        std::cout << "attl[!istm]: ";
        for (int i = 0; i < 6; i++){
            std::cout << ((attl[!istm] & (0xFU << (i << 2))) >> (i << 2)) << ", ";
        }
        std::cout << '\n';
        */

        if (gains[istm] - gains[!istm] > stdn * (attl[!istm] != 0U)){
            break;
        }

        istm = !istm;
    }

    pieces[1] = rks; //restore original bitboards
    pieces[ao] |= sqbb;

    return (istm == toMove);
}

int Engine::quiesce(int alpha, int beta, int ply){
    //int failSoft = (scores[toMove] - scores[!toMove]);
    int score = evaluate();

    int bestScore = score;
    if (score >= beta){
        return score;
    }
    if (alpha < score){
        alpha = score;
    }

    int lply = 64 + ply;
    int nc = fullMoveGen(lply, 1);

    //MVVLVA
    /*
    for (int ii = 0; ii < nc; ii++){ //generate move priorities for all
        mprior[lply][ii] = -((moves[lply][ii] >> 16) & 7); //less valuable = better (-a)
        mprior[lply][ii] += (10 * ((moves[lply][ii] >> 13) & 7)); //10v
    }
    */

    //for (int ii = 0; ii < nc; ii++){
    //    mprior[lply][ii] = (1 << 20) + ((moves[lply][ii] >> 16) & 7U) - (((moves[lply][ii] >> 13) & 7U) << 9);
    //}
    

    sortMoves(nc, lply);

    for (int i = 0; i < nc; i++){
        if (!see(moves[lply][i])){
            continue;
        }

        makeMove(moves[lply][i], true);
        endHandle();
        if (isChecked(!toMove) or kingBare()){
            unmakeMove(moves[lply][i], true);
            continue;
        }

        score = -quiesce(-beta, -alpha, ply + 1);
        unmakeMove(moves[lply][i], true); //take back the move
        
        if (score >= beta){
            return score;
        }
        if (score > alpha){ //yay best move
            alpha = score; // alpha acts like max in MiniMax  
        }
        if (score > bestScore){
            bestScore = score;
        }   
    }
    return bestScore;    
}

double Engine::preciseQuiesce(double alpha, double beta, int ply){
    int mdiff = scores[toMove] + mobil[toMove] - scores[!toMove] - mobil[!toMove];
    int ediff = eScores[toMove] + emobil[toMove] - eScores[!toMove] - emobil[!toMove];

    double failSoft = (mdiff * inGamePhase + ediff * (Position::totalGamePhase - inGamePhase)) / ((double) Position::totalGamePhase);

    double score = -29000.0;
    if (failSoft >= beta){
        return beta;
    }
    if (alpha < failSoft){
        alpha = failSoft;
    }

    int lply = 64 + ply;
    int nc = fullMoveGen(lply, 1);    

    sortMoves(nc, lply);

    for (int i = 0; i < nc; i++){
        makeMove(moves[lply][i], true);
        endHandle();
        if (isChecked(!toMove) or kingBare()){
            unmakeMove(moves[lply][i], true);
            continue;
        }

        score = -preciseQuiesce(-beta, -alpha, ply + 1);
        unmakeMove(moves[lply][i], true); //take back the move
        
        if (score >= beta){
            return beta;
        }
        if (score > alpha){ //yay best move
            alpha = score; // alpha acts like max in MiniMax  
        }   
    }
    return alpha;    
}

int Engine::alphabeta(int alpha, int beta, int depth, int ply, bool nmp){
    //assert(ply < 64);
    int score = -29000;
    int bestScore = -29000;

    //3-Fold Check
    int reps = countReps(ply);
    if (reps > 2){
        return 0;
    }

    //70-move rule check
    if (chm[thm] >= 140){
        return 0;
    }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
    }

    bool isPV = (beta - alpha > 1);

    //Mate Distance Pruning
    if (!isPV){
        int malpha = std::max(alpha, -29000 + ply);
        int mbeta = std::min(beta, 29000 - ply - 1);

        if (malpha >= mbeta){
            return malpha;
        }
    }
                                                                                                                                                                                                                                                                                                        
    //Transposition Table Probing
    int ttindex = zhist[thm] & 0xFFFFF;
    int ttdepth = ttable[ttindex].eDepth;
    Move ttmove = 0U;

    if (ttable[ttindex].eHash == zhist[thm]){
        score = ttable[ttindex].eScore;
        ttmove = ttable[ttindex].eMove;

        int ntype = ttable[ttindex].enType;
        if ((ttdepth >= depth) and !(isPV and (ply < 2)) and (reps == 1)){
            if (ntype == 1){
                return score;
            }
            if ((ntype == 2) and (score >= beta)){
                return score;
            }
            if ((ntype == 3) and (score <= alpha)){
                return score;
            }
        }
        
    }

    /*
    if ((zhist[thm] == ttable[ttindex].eHash) and (ttable[ttindex].eDepth >= depth)
        and !(isPV and (ply < 2)) and (ply > 0) and (reps == 1)){
        
        score = ttable[ttindex].eScore;
        // if (special){ 
        //    std::cout << "Consulting TT...\n";
        //    ttable[ttindex].print();
        //}

        if (ttable[ttindex].enType == 1){ //PV Node
            // if (special){ std::cout << "TT PV Return @ depth " << depth << '\n';} // " from " << moveToAlgebraic(thisLine[ply - 1]) << '\n'; }
            return score;
        }
        if ((ttable[ttindex].enType == 2) and (score >= beta)){ //Cut
            // if (special){ std::cout << "TT Beta Return @ depth " << depth << '\n';} //" from " << moveToAlgebraic(thisLine[ply - 1]) << '\n'; }
            return score;
        }
        if ((ttable[ttindex].enType == 3) and (score <= alpha)){ //All
            // if (special){ std::cout << "TT Alpha Return @ depth " << depth << '\n';} // << " from " << moveToAlgebraic(thisLine[ply - 1]) << '\n'; }
            return score;
        }
    }
    */

    bool inCheck = isChecked(toMove);

    double margin, fmargin;

    //Reverse Futility Pruning
    //position is so good that there is presumably a move that does not crash eval 
    int presentEval = evaluate();
    margin = rfpCoef[0] + rfpCoef[1] * depth;
    if ((ply > 0) and (abs(beta) < 28000) and (presentEval - margin >= beta) and !inCheck){
        return presentEval - margin;
        //return (presentEval + beta) / 2;
        //return beta;
    }
        
    //Null - Move Pruning
    nmp = ((scores[1] > 1000) and (scores[0] > 1000)) and !nmp;
    //nmp enabled if both sides have material and there was no nmp before
    //nmp disabled if both sides dont have material or there was nmp before

    if (nmp and (ply != 0) and (depth > 1) and !inCheck /*and (presentEval >= beta)*/){
        passMove();
        score = -alphabeta(-beta, -alpha, std::max(0, (7 * depth / 10) - 1), ply + 1, nmp);
        unpassMove();
        if (score >= beta){
            return beta;
        }
    }

    int numMoves = fullMoveGen(ply, 0);
    
    //move scoring
    for (int ll = 0; ll < numMoves; ll++){
        if (moves[ply][ll] == ttmove){
            mprior[ply][ll] = (1 << 30);
            continue;
        }
        
        /*
        if (moves[ply][ll] == killers[ply][1]){
            mprior[ply][ll] = (1 << 18);
            continue;
        }

        if (moves[ply][ll] == killers[ply][0]){
            mprior[ply][ll] = (1 << 16);
        }
        */
        
        if ((moves[ply][ll] == killers[ply][1]) or (moves[ply][ll] == killers[ply][0])){
            mprior[ply][ll] = (1 << 16);
        }  
                
    }

    sortMoves(numMoves, ply);

    Move localBestMove = 0; //for TT updating
    bool isAllNode = true;
    //bool extended = false;

    int lmrReduce;
    int searchedQuiets = 0;
    bool boringMove;
    bool capturing, promoting;

    fmargin = fpCoef[0] + fpCoef[1] * depth;

    for (int i = 0; i < numMoves; i++){
        //pickMove(i, numMoves, ply);
        //if (ply == 0) { std::cout << "considering: " << moveToAlgebraic(moves[ply][i]) << '\t' << mprior[ply][i] << '\n';};
        //printMoveAsBinary(moves[ply][i]);

        // if (special){
        /*
            if (ply == 0){ std::cout << "\n == ROOT CALL MOVE == \n"; }
            std::cout << "Ply: " << ply << "    DepthAt: " << depth << "    Move: " << moveToAlgebraic(moves[ply][i]);
            std::cout << "    isAllNode: " << isAllNode;
        }
        */
        
        promoting = (moves[ply][i] >> 19) & 1U;
        capturing = (moves[ply][i] >> 12) & 1U;

        makeMove(moves[ply][i], true); 
        //thisLine[ply] = moves[ply][i];
        endHandle();

        if (isChecked(!toMove) or kingBare()){
            //std::cout << "check: " << moveToAlgebraic(moves[ply][i]) << '\n';
            // if (special) { std::cout << "\nLegality Prune\n"; } 
            unmakeMove(moves[ply][i], true);
            continue;
        }

        if ((ply > 0) and !capturing and (bestScore > -28000) and !inCheck){
            //LMP goes here
            if (!isPV and (depth < 4) and 
                (searchedQuiets >= lmpCoef[0] + lmpCoef[1] * depth * depth)){
                unmakeMove(moves[ply][i], true);
                // if (special) { std::cout << "\nLate Move Prune\n"; } 
                continue;
            }

            //Futility Pruning
            if ((alpha < 28000) and (-evaluate() + fmargin < alpha)){
                unmakeMove(moves[ply][i], true);
                // if (special) { std::cout << "\nFutility Prune\n"; } 
                continue;
            }

            searchedQuiets++;
        }
        
        //score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);

        //boringMove = !(capturing or promoting or inCheck);
        //quietMoves += boringMove; // quiet move if not boring move
        //quietMoves += !(inCheck or capturing);

        // PVS, LMR
        if (isAllNode){
            // { std::cout << "    NewDepth (PV): " << depth - 1 << '\n'; }
            score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
        } else {
            //score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp); bare PVS
            
            lmrReduce = lmrReduces[depth][i];
            boringMove = (depth > 1) and !(capturing or promoting or isChecked(toMove) or inCheck);
            lmrReduce *= boringMove;

            // if (special){ std::cout << "    NewDepth (LMR): " << std::max(depth - 1 - lmrReduce, 0) << '\n'; }
            
            score = -alphabeta(-alpha-1, -alpha, std::max(depth - 1 - lmrReduce, 0), ply + 1, nmp);

            if (score > alpha and lmrReduce > 0) {
                // if (special){ std::cout << "LMR Re-Search @ depth " << depth - 1 << '\n'; } 
                score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp);
            }
            if (score > alpha){
                // if (special){ std::cout << "LMR Re-Search Full Window\n"; } 
                score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
            }
        }
        
        //std::cout << "score: " << score << '\t' << moveToAlgebraic(moves[ply][i]) << '\n';
        unmakeMove(moves[ply][i], true);

        if (score >= beta){ //beta cutoff, cut node (2)
            ttable[ttindex].update(score, 2, depth, moves[ply][i], thm);
            // if (special){ std::cout << "Fail-High: " << score << '\n'; }
            //std::cout << "cut " << moveToAlgebraic(moves[ply][i]) << '\n';
            
            //Killer Move Updating and History Updating
            if ((moves[ply][i] & 4096U) ^ (4096U)){
                killers[ply][0] = killers[ply][1];
                killers[ply][1] = moves[ply][i];

                uint8_t pstp = (moves[ply][i] >> 16) & 7U;
                uint8_t edsq = (moves[ply][i] >> 6) & 63U;

                bool hexc = historyTable[toMove][pstp][edsq] > 0;
                historyTable[toMove][pstp][edsq] += (depth * depth * !hexc);
                //historyTable[toMove][pstp][edsq] += depth * depth * (-historyTable[toMove][pstp][edsq] / (double) historyCap);
            }            

            return score; // fail-hard beta cutoff
        }
        if (score > alpha){
            //std::cout << "PV " << moveToAlgebraic(moves[ply][i]) << '\n';
            isAllNode = false;
            alpha = score;

            // if (special){ std::cout << "PV: " << alpha << '\n'; }
        }
        if (score > bestScore){
            if (ply == 0){
                bestMove = moves[ply][i];
            }
            //PV - Nodes (1)
            localBestMove = moves[ply][i];
            bestScore = score;
        }
    }

    //std::cout << "Alpha: " << alpha << "\tScore: " << score << '\n';

    if (score == -29000){ //if no legal moves -> mate
        //std::cout << "no legal moves\n";sea
        score += ply;
        return score;
    }
    
    // TT-updating
    /*
    if (isAllNode){
        ttable[ttindex].update(alpha, 3, depth, 0, thm);
    } else {
        ttable[ttindex].update(alpha, 1, depth, localBestMove, thm);
    }
    */

    ttable[ttindex].update(bestScore, 1 + 2 * isAllNode, depth, localBestMove, thm);

    //if (special and ply == 0){ std::cout << "Best Move: " << moveToAlgebraic(localBestMove) << '\n'; }

    return bestScore;
}

int Engine::search(uint32_t thinkTime, int mdepth, uint64_t maxNodes, bool output){
    Bitboard oPos[8] = {sides[0], sides[1], pieces[0], pieces[1], pieces[2], pieces[3], pieces[4], pieces[5]};
    bool oMove = toMove; int othm = thm;
    timeKept = ~thinkTime;

    int alpha = -50000;
    int beta = 50000;

    Move cbMove = 0; //best move yielded from latest complete search
    int cbEval = 0; //eval of latest complete search
    uint64_t prevNodes; //nodes consumed in total

    eraseHistoryTable();
    evaluateScratch();
    
    beginMobility(true);
    beginMobility(false);

    int prevEval;
    bool windowFail;

    thinkLimit = thinkTime;
    nodes = 0;
    mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    try {
        prevNodes = nodes;
        cbEval = alphabeta(alpha, beta, 0, 0, false);
        if (output){ std::cout << "info depth 0 nodes " << nodes << " score cp " << cbEval << '\n';}
        
        for (int i = 1; i < mdepth + 1; i++){
            windowFail = true;
            prevNodes = nodes;
            prevEval = cbEval;
            int aspa = 0, aspb = 0;

            // if (special) { std::cout << "\n\n === BEGINNING DEPTH " << i << " SEARCH === \n\n"; }
            while (windowFail){
                alpha = prevEval - aspWins[aspa];
                beta = prevEval + aspWins[aspb];
                
                // if (special){ std::cout << '[' << alpha << " , " << beta << "]\n"; }
                cbEval = alphabeta(alpha, beta, i, 0, false); //ensure nmp = 1 when ply = 0

                if (cbEval <= alpha){
                    aspa++;
                } else if (cbEval >= beta){
                    aspb++;
                } else {
                    windowFail = false;
                }
            }

            // if (special) { std::cout << "\n\n === FINISHED DEPTH " << i << " SEARCH === \n\n"; }

            cbMove = bestMove;
            if (output){ std::cout << "info depth " << i << " nodes " <<
                nodes - prevNodes << " score cp " << cbEval << '\n'; }
        }
    } catch (const char* e){
        bestMove = cbMove;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - moment).count();

    if (output){
        std::cout << "info nodes " << nodes << " nps ";
        std::cout << ((dur == 0) ? 0 : ((int) ((float) nodes * 1000 / dur))) << '\n';
        std::cout << "bestmove " << moveToAlgebraic(bestMove) << '\n';
    }

    nodesForever += nodes;

    sides[0] = oPos[0]; sides[1] = oPos[1];
    pieces[0] = oPos[2]; pieces[1] = oPos[3]; pieces[2] = oPos[4]; 
    pieces[3] = oPos[5]; pieces[4] = oPos[6]; pieces[5] = oPos[7];

    thm = othm; toMove = oMove;
    nodes = 0ULL;
    thinkLimit = 0xFFFFFFFF;
    mnodes = ~0ULL;

    /*
    int extraMoves = fullMoveGen(0, 0);
    for (int aaa = 0; aaa < extraMoves; aaa++){
        std::cout << moveToAlgebraic(moves[0][aaa])<< ":    ";
        std::cout << historyTable[toMove][(moves[0][aaa] >> 16U) & 7U][(moves[0][aaa] >> 6U) & 63U] << '\n';
    }
    */

    return cbEval;
}

void Engine::bench(){
    std::string marks[] = 
        {   
            "rnbkqbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBKQBNR w - - 0 1",
            "r1bkq2r/2p2ppp/ppnp1n1b/4p3/8/PPNPQNPB/2P1PP1P/R1BK3R b - - 0 7",
            "3rr3/2kq1p2/1pp1nnpb/p3p3/P6P/BPNBP3/1KPN1Q2/4R1R1 w - - 3 34",
            "r4r2/p3k3/b2p2pb/1qn1p1N1/4P3/1P1PQ1P1/P1K4P/R4R2 w - - 2 25",
            "r2kr3/2p4p/bpnpqn1b/pN3pN1/P2Q4/BP4PB/2PK1P1P/2R1R3 b - - 3 19",
            "3rn3/2p1k3/1p1pb2b/p4qN1/Pn1Q3P/BPN2PP1/2PK4/4R3 b - - 0 28",
            "8/1r6/7R/4QQK1/8/2k2Q2/8/8 w - - 0 1",
            "2r5/p2k4/b5pb/3qp3/1P6/P2PQ1PP/3K4/4R3 b - - 0 33",
            "8/8/8/3q2Q1/2bk4/5KP1/8/8 w - - 18 67",
            "6b1/5kP1/2q2Q2/5K2/8/8/8/8 w - - 21 90"            
        };

    auto benchStart = std::chrono::steady_clock::now();

    for (std::string op : marks){
        newGame();
        readFen(op);
        search(0xFFFFFFFFU, 11, 0x3FFFFFFFULL, false);
    }

    auto benchEnd = std::chrono::steady_clock::now();
    auto benchDur = std::chrono::duration_cast<std::chrono::milliseconds>(benchEnd - benchStart).count();

    //((dur == 0) ? 0 : ((int) ((float) nodes * 1000 / dur))) << '\n';


    std::cout << nodesForever << " nodes " << ((int) ((float) nodesForever * 1000 / benchDur)) << " nps\n";    
}


