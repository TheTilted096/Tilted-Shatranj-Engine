/*
2nd Version of Tilted Search/Eval
Compatible with STiltedMoveGenV3
4-24-2024
TheTilted096
*/

#include "STiltedMoveGenV3.cpp"

class TTentry{
    public:
        int eScore;
        uint64_t eHash;
        int enType;
        int eDepth;

        uint32_t eMove;

        TTentry(){
            eScore = -29501;
            eHash = 0; enType = -1;
            eDepth = -1; eMove = 0;
        }

        void update(int& sc, int nt, int& d, uint32_t dm){
            if ((d >= eDepth) or (zhist[thm] != eHash)){
                eScore = sc;
                enType = nt;
                eDepth = d;
                eHash = zhist[thm];
                eMove = dm;
            }
        }

        void print(){
            std::cout << "Index: " << (eHash & 0xFFFFF) << "     Score: " << eScore
                << "\tdepthAt: " << eDepth << "     nodeType: " << enType << "     debugMove: " 
                    << moveToAlgebraic(eMove) << '\n';
        }

        
};

TTentry ttable[0x100000];

uint32_t bestMove;

int evaluateScratch(){
    scores[1] = 0; scores[0] = 0;

    uint64_t wtb, btb;
    int f, g;

    for (int i = 0; i < 6; i++){
        g = -1;
        wtb = pieces[i + 7];
        while (wtb){
            f = __builtin_ctzll(wtb);
            g += (f + 1);

            scores[1] += mps[i][g];

            wtb >>= f;
            wtb >>= 1;
        }

        g = -1;
        btb = pieces[i];
        while (btb){
            f = __builtin_ctzll(btb);
            g += (f + 1);
            scores[0] += mps[i][56 ^ g];

            btb >>= f;
            btb >>= 1;
        }
    }

    return (scores[toMove] - scores[!toMove]);
}

bool kingBare(){ //returns true if 'toMove' king is bare, false otherwise, and false when both bare.
    bool b = (pieces[6] == pieces[0]);
    bool w = (pieces[13] == pieces[7]);

    return ((toMove and !w and b) or (!toMove and w and !b));
    // (black tM and white not bare and black bare) OR (white tM and white bare and black not bare);
}

int quiesce(int alpha, int beta, int lply){
    int failSoft = (scores[toMove] - scores[!toMove]);
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

int alphabeta(int alpha, int beta, int depth, int ply, bool nmp){
    int score = -29000;

    //3-Fold Check
    int rind = thm;
    int reps = 0;

    while (chm[rind]){ //search until the latest reset
        if (zhist[thm] == zhist[rind]){
            reps++;
        }
        rind -= 2;
    }
    if (reps > 2){
        return 0;
    }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
        //return scores[toMove] - scores[!toMove];
        //return evaluateScratch();
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

    for (int i = 0; i < numMoves; i++){
        //std::cout << "considering: " << moveToAlgebraic(moves[ply][i]) << '\n';
        makeMove(moves[ply][i], 1, 1); 
        endHandle();

        if (isChecked() or kingBare()){
            //std::cout << "check: " << moveToAlgebraic(moves[ply][i]) << '\n';
            makeMove(moves[ply][i], 0, 1);
            continue;
        }

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1, nmp);
        //std::cout << "score: " << score << '\t' << moveToAlgebraic(moves[ply][i]) << '\n';
        makeMove(moves[ply][i], 0, 1);

        if (score >= beta){ //beta cutoff, cut node (2)
            ttable[ttindex].update(score, 2, depth, moves[ply][i]);
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
            alpha = score;
        }
    }

    //std::cout << "Alpha: " << alpha << "\tScore: " << score << '\n';

    if (score == -29000){ //if no legal moves -> mate
        //std::cout << "no legal moves\n";
        score += ply;
        return score;
    }
    
    if (isAllNode){
        ttable[ttindex].update(alpha, 3, depth, 0);
    } else {
        ttable[ttindex].update(alpha, 1, depth, localBestMove);
    }

    return alpha;
}

void moveTimer(int wait){
    timeExpired = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    timeExpired = true;
}

void iterativeDeepening(uint32_t thinkTime, int mdepth){
    int alpha = -30000;
    int beta = 30000;

    std::thread myTimer;
    if (~thinkTime){
        myTimer = std::thread(moveTimer, thinkTime);
    }

    uint32_t cbMove; //best move yielded from latest complete search
    int cbEval; //eval of latest complete search
    uint64_t prevNodes; //nodes consumed in total

    evaluateScratch();

    auto start = std::chrono::steady_clock::now();
    try {
        for (int i = 0; i < mdepth + 1; i++){
            prevNodes = nodes;
            cbEval = alphabeta(alpha, beta, i, 0, false); //ensure nmp = 1 when ply = 0

            //std::cout << cbEval << " eval in id\n";

            cbMove = bestMove;
            std::cout << "info depth " << i << " nodes " <<
                nodes - prevNodes << " score cp " << cbEval << '\n';
        }
    } catch (const char* e){
        timeExpired = false; //reset time expire flag
        bestMove = cbMove;
    }
    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    if (~thinkTime){
        myTimer.detach();
    }
    
    std::cout << "info nodes " << nodes << " nps ";
    std::cout << ((dur == 0) ? 0 : ((int) ((float) nodes * 1000 / dur))) << '\n';

    std::cout << "bestmove " << moveToAlgebraic(bestMove) << '\n';
}

