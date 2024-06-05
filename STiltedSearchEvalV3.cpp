/*
3rd generation Search-Eval file fitted to OOP design
TheTilted096, 5-25-2024

*/

#include "STiltedMoveGenV4.cpp"

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

int Engine::halfMoveCount(){
    return chm[thm];
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

int Engine::evaluateScratch(){
    scores[1] = 0; scores[0] = 0;
    eScores[1] = 0; eScores[0] = 0;

    uint64_t wtb, btb;
    int f, g;

    for (int i = 0; i < 6; i++){
        g = -1;
        wtb = pieces[i + 7];
        while (wtb){
            f = __builtin_ctzll(wtb);
            g += (f + 1);

            scores[1] += mps[i][g];
            eScores[1] += eps[i][g];

            wtb >>= f;
            wtb >>= 1;
        }

        g = -1;
        btb = pieces[i];
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

int Engine::evaluate(){
    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[!toMove];

    return (mdiff * inGamePhase + ediff * (64 - inGamePhase)) / 64;
}

bool Engine::kingBare(){ //returns true if 'toMove' king is bare, false otherwise, and false when both bare.
    bool b = (pieces[6] == pieces[0]);
    bool w = (pieces[13] == pieces[7]);

    return ((toMove and !w and b) or (!toMove and w and !b));
    // (black tM and white not bare and black bare) OR (white tM and white bare and black not bare);
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
        makeMove(moves[64 + lply][i], 1, 1);
        endHandle();
        if (isChecked() or kingBare()){
            makeMove(moves[64 + lply][i], 0, 1);
            continue;
        }

        score = -quiesce(-beta, -alpha, lply + 1);
        makeMove(moves[64 + lply][i], 0, 1); //take back the move
        
        if (score >= beta){
            return beta;
        }
        if (score > alpha){ //yay best move
            alpha = score; // alpha acts like max in MiniMax  
        }   
    }
    return alpha;    
}

int Engine::countReps(){
    int rind = thm;
    int reps = 0;

    while (chm[rind] and (rind >= 0)){ //search until the latest reset
        if (zhist[thm] == zhist[rind]){
            reps++;
        }
        rind -= 2;
    }
    return reps;
}

int Engine::alphabeta(int alpha, int beta, int depth, int ply, bool nmp){
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
    int margin = rfpCoef[0] + rfpCoef[1] * depth;
    if ((ply > 0) and (abs(beta) < 28000) and (be - margin >= beta) and !inCheck){
        return be - margin;
    }
        
    //Null - Move Pruning
    nmp = ((scores[1] > 1000) and (scores[0] > 1000)) and !nmp;
    //nmp enabled if both sides have material and there was no nmp before
    //nmp disabled if both sides dont have material or there was nmp before

    if (nmp and (ply != 0) and (depth > 1)){
        makeMove(0, 1, 1);
        if (!isChecked()){
            score = -alphabeta(-beta, -alpha, depth - 2, ply + 1, nmp);
        }
        makeMove(0, 0, 1);
        if (score >= beta){
            return beta;
        }
    }

    int numMoves = fullMoveGen(ply, 0);
    
    bool ttMoveFound = false;
    //Find TT-Move
    for (int ll = 0; ll < numMoves; ll++){
        if (moves[ply][ll] == ttable[ttindex].eMove){
            std::swap(moves[ply][ll], moves[ply][0]);
            ttMoveFound = true;
            break;
        }
    }

    //MVV-LVA
    for (int ii = ttMoveFound; ii < numMoves; ii++){
        mprior[ply][ii] = ((moves[ply][ii] >> 16) & 7) * (-1); //less valuable = better (-a)
        if ((moves[ply][ii] >> 12) & 1){ //if capturing
            mprior[ply][ii] += (10 * ((moves[ply][ii] >> 13) & 7)); //10v
        } else {
            mprior[ply][ii] += 100;
        }
    }
    for (int aa = 1 + ttMoveFound; aa < numMoves; aa++){
        for (int bb = aa; (mprior[ply][bb - 1] > mprior[ply][bb]) and (bb > ttMoveFound); bb--){
            std::swap(moves[ply][bb], moves[ply][bb - 1]);
            std::swap(mprior[ply][bb], mprior[ply][bb - 1]);
        }
    }
    

    uint32_t localBestMove = 0; //for TT updating
    bool isAllNode = true;
    bool searchPv = true;

    int lmrReduce;
    bool boringMove;

    for (int i = 0; i < numMoves; i++){
        //std::cout << "considering: " << moveToAlgebraic(moves[ply][i]) << '\n';
        makeMove(moves[ply][i], 1, 1); 
        endHandle();

        if (isChecked() or kingBare()){
            //std::cout << "check: " << moveToAlgebraic(moves[ply][i]) << '\n';
            makeMove(moves[ply][i], 0, 1);
            continue;
        }

        //score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);

        // PVS, LMR
        if (searchPv){
            score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
        } else {
            //score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp); bare PVS
            
            //lmrReduce = lmrReduces[depth][i];
            lmrReduce = 1;
            boringMove = (i > 3) and (depth > 1) and !isInteresting(moves[ply][i], inCheck);
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
        makeMove(moves[ply][i], 0, 1);

        if (score >= beta){ //beta cutoff, cut node (2)
            ttable[ttindex].update(score, 2, depth, moves[ply][i], thm);
            //std::cout << "cut " << moveToAlgebraic(moves[ply][i]) << '\n';
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
    uint64_t oPos[14]; bool oMove = toMove; int othm = thm;
    for (int i = 0; i < 14; i++){
        oPos[i] = pieces[i];
    }

    int alpha = -50000;
    int beta = 50000;

    uint32_t cbMove; //best move yielded from latest complete search
    int cbEval = 0; //eval of latest complete search
    uint64_t prevNodes; //nodes consumed in total

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
                //sstd::cout << '[' << alpha << " , " << beta << "]\n";

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

    for (int i = 0; i < 14; i++){
        pieces[i] = oPos[i];
    }
    thm = othm; toMove = oMove;
    nodes = 0;
    thinkLimit = 0xFFFFFFFF;
    mnodes=~0ULL;

    return cbEval;
}

uint32_t Engine::getMove(){
    return bestMove;
}