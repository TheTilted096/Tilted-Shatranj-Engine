/*
3rd generation Search-Eval file fitted to OOP design
TheTilted096, 5-25-2024

*/

#include "stiltedmvgnv4.cpp"


int Position::evaluateScratch(){
    scores[1] = 0; scores[0] = 0;
    eScores[1] = 0; eScores[0] = 0;

    uint64_t wtb, btb;
    int f, g;

    for (int i = 0; i < 6; i++){
        g = -1;
        wtb = pieces[i] & sides[1];
        while (wtb){
            f = __builtin_ctzll(wtb);
            g += (f + 1);

            scores[1] += mps[i][g];
            eScores[1] += eps[i][g];

            wtb >>= f;
            wtb >>= 1;
        }

        g = -1;
        btb = pieces[i] & sides[0];
        while (btb){
            f = __builtin_ctzll(btb);
            g += (f + 1);

            scores[0] += mps[i][56 ^ g];
            eScores[0] += eps[i][56 ^ g];

            btb >>= f;
            btb >>= 1;
        }
    }

    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[!toMove];

    return (mdiff * inGamePhase + ediff * (64 - inGamePhase)) / 64;
}

int Position::evaluate(){
    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[!toMove];

    return (mdiff * inGamePhase + ediff * (Position::totalGamePhase - inGamePhase)) / Position::totalGamePhase;
}




TTentry::TTentry(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
    zhist = nullptr;
}

void TTentry::reset(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
}

TTentry::TTentry(uint64_t t[]){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
    zhist = t;
}

void TTentry::update(int& sc, int nt, int& d, uint32_t dm, int thm){
    if ((d >= eDepth) or (zhist[thm] != eHash)){
        eScore = sc;
        enType = nt;
        eDepth = d;
        eHash = zhist[thm];
        eMove = dm;
    }
}

void TTentry::print(){
    std::cout << "Index: " << (eHash & 0xFFFFF) << "     Score: " << eScore
        << "\tdepthAt: " << eDepth << "     nodeType: " << enType << "     Move: " 
            << Bitboards::moveToAlgebraic(eMove) << '\n';
}



Engine::Engine(){
    mnodes= ~0ULL;
    
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
    if ((nodes % 2000 == 0) and 
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

void Engine::copyEval(EvalVars e){
    if (e.rc != nullptr){
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
}

bool Engine::isInteresting(uint32_t& move, bool checked){
    toMove = !toMove;
    bool giveCheck = isChecked();
    toMove = !toMove;

    return ((move >> 12) & 1U) or ((move >> 19) & 1U) or giveCheck or checked;
}

int Engine::quiesce(int alpha, int beta, int lply){
    //int failSoft = (scores[toMove] - scores[!toMove]);
    int failSoft = evaluate();

    int score = -29000;
    if (failSoft >= beta){
        return beta;
    }
    if (alpha < failSoft){
        alpha = failSoft;
    }

    int nc = fullMoveGen(64 + lply, 1);

    //MVVLVA
    for (int ii = 0; ii < nc; ii++){ //generate move priorities for all
        mprior[64 + lply][ii] = ((moves[64 + lply][ii] >> 16) & 7) * (-1); //less valuable = better (-a)
        mprior[64 + lply][ii] += (10 * ((moves[64 + lply][ii] >> 13) & 7)); //10v
    }
    for (int aa = 1; aa < nc; aa++){ //insertion sort
        for (int bb = aa; (bb > 0) and (mprior[64 + lply][bb - 1] > mprior[64 + lply][bb]); bb--){
            std::swap(moves[64 + lply][bb - 1], moves[64 + lply][bb]);
            std::swap(mprior[64 + lply][bb - 1], mprior[64 + lply][bb]);
        }
    }

    for (int i = 0; i < nc; i++){
        makeMove(moves[64 + lply][i], true);
        endHandle();
        if (isChecked() or kingBare()){
            unmakeMove(moves[64 + lply][i], true);
            continue;
        }

        score = -quiesce(-beta, -alpha, lply + 1);
        unmakeMove(moves[64 + lply][i], true); //take back the move
        
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

    toMove = !toMove;
    bool inCheck = isChecked();
    toMove = !toMove;

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

    if (nmp and (ply != 0) and (depth > 1)){
        passMove();
        if (!isChecked()){
            score = -alphabeta(-beta, -alpha, depth - 2, ply + 1, nmp);
        }
        unpassMove();
        if (score >= beta){
            return beta;
        }
    }

    int numMoves = fullMoveGen(ply, 0);

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
    
    for (int ii = 1; ii < numMoves; ii++){
        for (int jj = ii; (jj > 0) and (mprior[ply][jj - 1] > mprior[ply][jj]); jj--){
            std::swap(moves[ply][jj], moves[ply][jj - 1]);
            std::swap(mprior[ply][jj], mprior[ply][jj - 1]);
        }
    }

    for (int kk = ply + 1; kk < 64; kk++){
        numKillers[kk] = 0;
    }

    uint32_t localBestMove = 0; //for TT updating
    bool isAllNode = true;
    bool searchPv = true;

    int lmrReduce;
    bool boringMove, quietMove;

    for (int i = 0; i < numMoves; i++){
        //std::cout << "considering: " << moveToAlgebraic(moves[ply][i]) << '\t' << mprior[ply][i] << '\n';
        //printMoveAsBinary(moves[ply][i]);
        makeMove(moves[ply][i], true); 
        endHandle();

        if (isChecked() or kingBare()){
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
            boringMove = (depth > 1) and !isInteresting(moves[ply][i], inCheck);
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
            if ((moves[ply][i] & 4096U) ^ (4096U) and (ply > 0)){
                if (numKillers[ply] = 2){
                    std::swap(killers[ply][0], killers[ply][1]);
                    killers[ply][1] = moves[ply][i];
                } else {
                    killers[ply][numKillers[ply]] = moves[ply][i];
                    numKillers[ply]++;
                }

                historyTable[toMove][(moves[ply][i] >> 16U) & 7U][(moves[ply][i] >> 6U) & 63U] += (depth * depth);
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
            isAllNode = false;
            searchPv = false;
            alpha = score;
        }
    }

    //std::cout << "Alpha: " << alpha << "\tScore: " << score << '\n';

    if (score == -29000){ //if no legal moves -> mate
        //std::cout << "no legal moves\n";
        score += ply;
        return score;
    }
    
    // TT-updating
    if (isAllNode){
        ttable[ttindex].update(alpha, 3, depth, 0, thm);
    } else {
        ttable[ttindex].update(alpha, 1, depth, localBestMove, thm);
    }

    return alpha;
}

int Engine::search(uint32_t thinkTime, int mdepth, uint64_t maxNodes, bool output){
    Bitboard oPos[8] = {sides[0], sides[1], pieces[0], pieces[1], pieces[2], pieces[3], pieces[4], pieces[5]};
    bool oMove = toMove; int othm = thm;

    int alpha = -50000;
    int beta = 50000;

    uint32_t cbMove; //best move yielded from latest complete search
    int cbEval = 0; //eval of latest complete search
    uint64_t prevNodes; //nodes consumed in total

    eraseHistoryTable();
    evaluateScratch();

    int prevEval;
    bool windowFail;

    thinkLimit = thinkTime;
    nodes = 0;
    mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    try {
        prevNodes = nodes;
        cbEval = alphabeta(alpha, beta, 0, 0, false);
        if (output){ std::cout << "info depth 0 nodes 0 score cp " << cbEval << '\n';}
        
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