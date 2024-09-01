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

void Engine::endHandle(){
    if ((nodes % 2000 == 0) and timeKept and
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

int Engine::quiesce(int alpha, int beta, int ply){
    //int failSoft = (scores[toMove] - scores[!toMove]);
    int failSoft = evaluate();

    int score = -29000;
    if (failSoft >= beta){
        return beta;
    }
    if (alpha < failSoft){
        alpha = failSoft;
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
        makeMove(moves[lply][i], true);
        endHandle();
        if (isChecked(!toMove) or kingBare()){
            unmakeMove(moves[lply][i], true);
            continue;
        }

        score = -quiesce(-beta, -alpha, ply + 1);
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

    //3-Fold Check
    int reps = countReps();
    if (reps > 2){
        return 0;
    }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
    }

    //Transposition Table Probing
    int ttindex = zhist[thm] & 0xFFFFF;
    if ((zhist[thm] == ttable[ttindex].eHash) and (ttable[ttindex].eDepth >= depth)
        and (ply > 0) and (reps == 1)){
        
        score = ttable[ttindex].eScore;

        if (ttable[ttindex].enType == 1){ //PV Node
            return score;
        }
        if ((ttable[ttindex].enType == 2) and (score >= beta)){ //Cut
            return score;
        }
        if ((ttable[ttindex].enType == 3) and (score <= alpha)){ //All
            return score;
        }
    }

    bool inCheck = isChecked(toMove);

    //Reverse Futility Pruning
    int be = evaluate();
    double margin = rfpCoef[0] + rfpCoef[1] * depth;
    if ((ply > 0) and (abs(beta) < 28000) and (be - margin >= beta) and !inCheck){
        return be - margin;
    }
        
    //Null - Move Pruning
    nmp = ((scores[1] > 1000) and (scores[0] > 1000)) and !nmp;
    //nmp enabled if both sides have material and there was no nmp before
    //nmp disabled if both sides dont have material or there was nmp before

    if (nmp and (ply != 0) and (depth > 1) and !inCheck){
        passMove();
        score = -alphabeta(-beta, -alpha, depth - 2, ply + 1, nmp);
        unpassMove();
        if (score >= beta){
            return beta;
        }
    }

    int numMoves = fullMoveGen(ply, 0);
    
    //stilted-26-zfix
    /*
    for (int ll = 0; ll < numMoves; ll++){ //for each move, score it
        if (moves[ply][ll] == ttable[ttindex].eMove){ //TT-move has score 0
            mprior[ply][ll] = 0;
        } else if ((moves[ply][ll] >> 12U) & 1U){ //captures have value 512 * victim - aggressor
            mprior[ply][ll] = (((moves[ply][ll] >> 13U) & 7U) << 9U) - ((moves[ply][ll] >> 16U) & 7U);
        } else if ((moves[ply][ll] == killers[ply][0]) or (moves[ply][ll] == killers[ply][1])){
            mprior[ply][ll] = 10000; //killers have value 10000
        } else {
            mprior[ply][ll] = 100000000 - ((moves[ply][ll] >> 16U) & 7U) -
                 historyTable[toMove][(moves[ply][ll] >> 16U) & 7U][(moves[ply][ll] >> 6U) & 63U]; //non-captures have value 20000
        }
    }
    */

    /*
    for (int ll = 0; ll < numMoves; ll++){ //for each move, score it
        if (moves[ply][ll] == ttable[ttindex].eMove){ //TT-move has score 0
            mprior[ply][ll] = (1 << 30);
        } else if ((moves[ply][ll] >> 12U) & 1U){ //captures have value 512 * victim - aggressor
            mprior[ply][ll] = (1 << 20) + ((moves[ply][ll] >> 16) & 7U) - (((moves[ply][ll] >> 13) & 7U) << 9);
        } else if ((moves[ply][ll] == killers[ply][0]) or (moves[ply][ll] == killers[ply][1])){
            mprior[ply][ll] = (1 << 16); //killers have value 10000
        } else {
            mprior[ply][ll] = ((moves[ply][ll] >> 16U) & 7U) +
                 historyTable[toMove][(moves[ply][ll] >> 16U) & 7U][(moves[ply][ll] >> 6U) & 63U]; //non-captures have value 20000
        }
    }
    */

    for (int ll = 0; ll < numMoves; ll++){
        if (moves[ply][ll] == ttable[ttindex].eMove){
            mprior[ply][ll] = (1 << 30);
            continue;
        }
        if ((moves[ply][ll] == killers[ply][0]) or (moves[ply][ll] == killers[ply][1])){
            mprior[ply][ll] = (1 << 16);
        }
    }

    
    sortMoves(numMoves, ply);

    Move localBestMove = 0; //for TT updating
    //bool isAllNode = true;
    bool searchPv = true;

    int lmrReduce;
    bool boringMove;

    for (int i = 0; i < numMoves; i++){
        //pickMove(i, numMoves, ply);
        //if (ply == 0) { std::cout << "considering: " << moveToAlgebraic(moves[ply][i]) << '\t' << mprior[ply][i] << '\n';};
        //printMoveAsBinary(moves[ply][i]);
        makeMove(moves[ply][i], true); 
        endHandle();

        if (isChecked(!toMove) or kingBare()){
            //std::cout << "check: " << moveToAlgebraic(moves[ply][i]) << '\n';
            unmakeMove(moves[ply][i], true);
            continue;
        }

        //score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);

        //quietMove = !isInteresting(moves[ply][i], inCheck);
        // PVS, LMR
        if (searchPv){
            score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
        } else {
            //score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp); bare PVS
            
            lmrReduce = lmrReduces[depth][i];
            boringMove = (depth > 1) and !(((moves[ply][i] >> 12) & 1U) or ((moves[ply][i] >> 19) & 1U) or isChecked(toMove) or inCheck);
            lmrReduce *= boringMove;
            
            score = -alphabeta(-alpha-1, -alpha, std::max(depth-1-lmrReduce, 0), ply + 1, nmp);

            if (score > alpha and lmrReduce > 0) { 
                score = -alphabeta(-alpha-1, -alpha, depth-1, ply+1, nmp);
            }
            if (score > alpha){
                score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
            }
        }
        
        //std::cout << "score: " << score << '\t' << moveToAlgebraic(moves[ply][i]) << '\n';
        unmakeMove(moves[ply][i], true);

        if (score >= beta){ //beta cutoff, cut node (2)
            ttable[ttindex].update(score, 2, depth, moves[ply][i], thm);
            //std::cout << "cut " << moveToAlgebraic(moves[ply][i]) << '\n';
            
            //Killer Move Updating and History Updating
            if ((moves[ply][i] & 4096U) ^ (4096U)){
                killers[ply][0] = killers[ply][1];
                killers[ply][1] = moves[ply][i];

                uint8_t pstp = (moves[ply][i] >> 16) & 7U;
                uint8_t edsq = (moves[ply][i] >> 6) & 63U;

                bool hexc = historyTable[toMove][pstp][edsq] > 0;
                historyTable[toMove][pstp][edsq] += (depth * depth * !hexc);
            }            

            return beta; // fail-hard beta cutoff
        }
        if (score > alpha){
            //std::cout << "PV " << moveToAlgebraic(moves[ply][i]) << '\n';
            if (ply == 0){
                bestMove = moves[ply][i];
            }
            //PV - Nodes (1)
            localBestMove = moves[ply][i];
            searchPv = false;
            alpha = score;
        }
    }

    //std::cout << "Alpha: " << alpha << "\tScore: " << score << '\n';

    if (score == -29000){ //if no legal moves -> mate
        //std::cout << "no legal moves\n";sea
        score += ply;
        return score;
    }
    
    // TT-updating
    if (searchPv){
        ttable[ttindex].update(alpha, 3, depth, 0, thm);
    } else {
        ttable[ttindex].update(alpha, 1, depth, localBestMove, thm);
    }

    return alpha;
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

            while (windowFail){
                alpha = prevEval - aspWins[aspa];
                beta = prevEval + aspWins[aspb];
                
                cbEval = alphabeta(alpha, beta, i, 0, false); //ensure nmp = 1 when ply = 0
                //std::cout << '[' << alpha << " , " << beta << "]\n";

                if (cbEval <= alpha){
                    aspa++;
                } else if (cbEval >= beta){
                    aspb++;
                } else {
                    windowFail = false;
                }
            }

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