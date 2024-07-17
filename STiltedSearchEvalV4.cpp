/*
Function Definitions related to Evaluation

Contains Engine and some Position Methods
*/

#include "STiltedEngine.h"

TTentry::TTentry(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
    zref = nullptr;
}

void TTentry::setReference(uint64_t* zh){ zref = zh; }

void TTentry::update(int& sc, int nt, int& d, Move dm, int thm){
    if ((d >= eDepth) or (zref[thm] != eHash)){
        eScore = sc;
        enType = nt;
        eDepth = d;
        eHash = zref[thm];
        eMove = dm;
    }
}

void TTentry::print(std::ostream& ou){
    ou << "Index: " << (eHash & 0xFFFFF) << "     Score: " << eScore
        << "\tdepthAt: " << eDepth << "     nodeType: " << enType << "     Move: " 
            << Position::moveToAlgebraic(eMove) << '\n';
}

void TTentry::reset(){
    eScore = -29501;
    eHash = 0; enType = -1;
    eDepth = -1; eMove = 0;
}

int Position::evaluate(){
    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[!toMove];

    return (mdiff * gamePhase + ediff * (Position::totalGamePhase - gamePhase)) / 64;
}

int Position::evaluateScratch(){
    scores[1] = 0; scores[0] = 0;
    eScores[1] = 0; eScores[0] = 0;
    Bitboard tpbd; int f;
    
    //kings
    scores[1] += mps[0][__builtin_ctzll(sides[1] & pieces[0])];
    eScores[1] += eps[0][__builtin_ctzll(sides[1] & pieces[0])];

    scores[0] += mps[0][56 ^ __builtin_ctzll(sides[0] & pieces[0])];
    eScores[0] += eps[0][56 ^ __builtin_ctzll(sides[0] & pieces[0])];

    for (int i = 1; i < 6; i++){
        tpbd = sides[1] & pieces[i];
        while (tpbd){
            f = __builtin_ctzll(tpbd);
            scores[1] += mps[i][f];
            eScores[1] += eps[i][f];
            tpbd ^= (1ULL << f);
        }
        tpbd = sides[0] & pieces[i];
        while (tpbd){
            f = __builtin_ctzll(tpbd);
            scores[0] += mps[i][56 ^ f];
            eScores[0] += mps[i][56 ^ f];
            tpbd ^= (1ULL << f);
        }

    }

    int mdiff = scores[toMove] - scores[!toMove];
    int ediff = eScores[toMove] - eScores[toMove];

    return (mdiff * gamePhase + ediff * (Position::totalGamePhase - gamePhase)) / Position::totalGamePhase;
}

void Position::sortMoves(int nc, int ply){
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

Engine::Engine(){
    mnodes = ~0ULL;
    nodes = 0ULL;

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

void Engine::eraseTransposeTable(){
    for (int i = 0; i < 0x100000; i++){
        ttable[i].reset();
    }
}

Move Engine::getMove(){ return bestMove; }

void Engine::newGame(){
    setStartPos();
    eraseTransposeTable();
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

void Engine::showZobrist(std::ostream& ou){
    std::cout << "Tranposition Table:\n";
    for (int k = 1; k < 15; k++){
        for (int i = 0; i < 0xFFFFF; i++){
            if (ttable[i].eDepth == k){
                ttable[i].print(ou);
            }
        }
    }
    std::cout << "\nZobrist History + Last 20 Bits\n";
    for (int j = 0; j < thm + 1; j++){
        std::cout << "ZH " << j << ": " << zhist[j] << "\tIndex: " << (zhist[j] & 0xFFFFF) << '\n';
    }
}

void Engine::endHandle(){
    if (nodes > mnodes){
        throw "Nodes Exceeded\n";
    }
    if (((nodes & 0xFFFULL) == 0ULL) and timeKept and 
        std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - moment).count() >= thinkLimit){
        throw "Time Expired\n";
    }
}

int Engine::quiesce(int alpha, int beta, int ply){
    int failSoft = evaluate();
    //std::cout << failSoft << '\n';
    int score = -29000;
    if (failSoft >= beta){
        return beta;
    }
    if (alpha < failSoft){
        alpha = failSoft;
    }

    int lply = ply + 64;
    int nc = fullMoveGen(lply, true);

    sortMoves(nc, lply);

    for (int i = 0; i < nc; i++){
        makeMove(moves[lply][i], true); nodes++;
        endHandle();
        if (isChecked(!toMove) or kingBare()){
            unmakeMove(moves[lply][i], true);
            continue;
        }

        score = -quiesce(-beta, -alpha, ply + 1);
        unmakeMove(moves[lply][i], true);

        if (score >= beta){
            return beta;
        }
        if (score > alpha){
            alpha = score;
        }
    }
    return alpha;
}

int Engine::alphabeta(int alpha, int beta, int depth, int ply, bool nmp){
    int score = -29000;
    int reps = countReps(); //3fold check
    if (reps > 2){ return 0; }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
    }

    //Tranposition Table Probing
    int ttindex = zhist[thm] & 0xFFFFF;
    if ((zhist[thm] == ttable[ttindex].eHash) and (ttable[ttindex].eDepth >= depth)
        and (ply > 0) and (reps == 1)){
        
        score = ttable[ttindex].eScore;

        if (ttable[ttindex].enType == 1){ return score; } //PV node
        if ((ttable[ttindex].enType == 2) and (score >= beta)){ return score; } //Cut Node
        if ((ttable[ttindex].enType == 3) and (score <= alpha)){ return score; } //All Node
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
    //nmp enabled if both sides have sufficient material and no nmp on previous ply
    //nmp disabled if both sides have insufficient material or nmp on previous ply

    if (nmp and (ply != 0) and (depth > 1) and !inCheck){
        passMove();
        score = -alphabeta(-beta, -alpha, depth - 2, ply + 1, nmp);
        unpassMove();
        if (score >= beta){
            return beta;
        }
    }

    int numMoves = fullMoveGen(ply, false);

    //additional move scoring (TT and KILLERS)
    bool ttmf = false;
    for (int a = 0; a < numMoves; a++){
        if (!ttmf and (moves[ply][a] == ttable[ttindex].eMove)){
            mprior[ply][a] = 0x40000000U;
            ttmf = true;
            continue;         
        }
        if ((moves[ply][a] == killers[ply][0]) or (moves[ply][a] == killers[ply][1])){
            mprior[ply][a] = 0x20000U;
        }
    }

    sortMoves(numMoves, ply);

    Move localBestMove;
    bool searchPv = true;

    int lmrReduce; 
    bool boringMove;

    for (int i = 0; i < numMoves; i++){
        makeMove(moves[ply][i], true); nodes++;
        endHandle();

        if (isChecked(!toMove) or kingBare()){
            unmakeMove(moves[ply][i], true);
            continue;
        }

        //score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp); //plain alphabeta

        //PVS, LMR
        if (searchPv){
            score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
        } else {
            //score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp); //bare PVS

            lmrReduce = lmrReduces[depth][i];
            boringMove = (depth > 1) and 
                !(((moves[ply][i] >> 12) & 1U) or ((moves[ply][i] >> 19) & 1U) 
                    or isChecked(toMove) or inCheck); 
            //boring if depth > 1 and no capture, no promotion, or no checks
            lmrReduce *= boringMove; //dont LMR if the move is interesting


            score = -alphabeta(-alpha-1, -alpha, std::max(depth - 1 - lmrReduce, 0), ply + 1, nmp); //do a null window reduced search 

            if ((score > alpha) and (lmrReduce > 0)){ //move exceeds expectation
                score = -alphabeta(-alpha-1, -alpha, depth - 1, ply + 1, nmp); //do a full depth null window
            }
            if (score > alpha){ //do a full depth search if still exceeds expectation
                score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
            }
        }

        unmakeMove(moves[ply][i], true);

        if (score >= beta){ //beta cutoff, cut node (2)
            ttable[ttindex].update(score, 2, depth, moves[ply][i], thm);

            //Killer Move Updating and History Updating
            if (((moves[ply][i] & 0x1000U) ^ (0x1000U)) and (ply > 0)){
                if (moves[ply][i] != killers[ply][0]){
                    killers[ply][1] = killers[ply][0];
                    killers[ply][0] = moves[ply][i];
                }

                uint8_t pstp = (moves[ply][i] >> 16) & 7U;
                uint8_t edsq = (moves[ply][i] >> 6) & 63U;

                bool historyExceeds = (historyTable[toMove][pstp][edsq] > 0);
                historyTable[toMove][pstp][edsq] += (depth * depth * !historyExceeds);
            }

            return beta;
        }
        if (score > alpha){ //PV-Nodes (1)
            if (ply == 0){
                bestMove = moves[ply][i];
            }
            localBestMove = moves[ply][i];
            searchPv = false;
            alpha = score;            
        }
        
    }

    if (score == -29000){
        score += ply;
        return score;
    }

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

    int alpha = -50000;
    int beta = 50000;

    Move cbMove; //best move from previous depth search
    int cbEval = 0; //eval of previous depth search
    uint64_t prevNodes; //nodes total

    eraseHistoryTable();
    evaluateScratch();

    int prevEval; //temporary holder for cbEval
    bool windowFail; //aspiration windows

    thinkLimit = thinkTime; timeKept = ~thinkTime;
    nodes = 0ULL; mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    try { //search is halted by an exception
        prevNodes = nodes;
        cbEval = alphabeta(alpha, beta, 0, 0, false);
        if (output){ std::cout << "info depth 0 nodes " << nodes << " score cp " << cbEval << '\n'; }

        for (int i = 1; i < mdepth + 1; i++){
            windowFail = true; //assume window fails
            prevNodes = nodes; //save current node count for this depth
            prevEval = cbEval; //save previous depth eval for aspiration windows
            int aspa = 0, aspb = 0; //aspiration window values (above and below)

            while (windowFail){
                alpha = prevEval - aspWins[aspa]; //set alpha and beta
                beta = prevEval + aspWins[aspb];

                cbEval = alphabeta(alpha, beta, i, 0, false); 
                //std::cout << '[' << alpha << " , " << beta << "]\n";
                //ensures nmp = true when ply = 0
                //attempt to set current best eval to successful search

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
        std::cout << "info nodes "  << nodes << " nps ";
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

