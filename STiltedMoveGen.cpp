/*
This is take 2 of the Tilted Shatranj Engine,
since git decided it was a great idea to delete everybody.

Anyways, this is a library to do movegen.

TheTilted096, 3-8-2024.
*/

#include <iostream>
#include <cstdint>
#include <bitset>
#include <string>
#include <chrono>
#include <sstream>
#include <random>
#include <thread>

// #include <bits/stdc++.h>
#pragma GCC target("popcnt")

#define RANK0 255ULL               // 8th rank
#define FILE0 72340172838076673ULL // A File

#define LDIAG 9241421688590303745ULL // Long Diagonal (Left)
#define RDIAG 72624976668147840ULL   // long diagonal (right)

/*
The plan:

Each side is still represented as 7 bitboards.

Order of Pieces; King, Rook, Knight, Ferz, Alfil, Pawn, ALL
*/

uint64_t* white;
uint64_t* black;
bool toMove;
uint32_t bestMove;
uint64_t*** tables;
int boardEval;

uint32_t moves[64][128];

int wtotal, btotal;
volatile bool timeExpired;
uint64_t mnodes = 1000000000;

//int evalIncr[33];

uint64_t nodes;

int mps[6][64] = 
{
{-50, -40, -30, -20, -20, -30, -40, -50, /*King*/
-40, -10, 0, 0, 0, 0, -10, -40,
-30, 5, 10, 15, 15, 10, 5, -30,
-20, 10, 15, 15, 15, 15, 10, -20,
-10, 10, 15, 15, 15, 15, 10, -10,
-10, 5, 15, 15, 10, 5, 5, -10,
-20, 0, 25, 20, 0, 0, 0, -20,
-40, -30, -20, -10, -10, -20, -30, -40},

{0, 0, 0, 0, 0, 0, 0, 0, /*Rooks*/
5, 10, 10, 10, 10, 10, 10, 5,
0, 0, 0, 0, 0, 0, 0, 0,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-10, 0, 5, 10, 10, 5, 0, -10},

{-50, -40, -30, -30, -30, -30, -40, -50, /*Knights*/
-40, -20, -10, 0, 0, -10, -20, -40,
-30, 5, 10, 10, 10, 10, 5, -30,
-20, 5, 15, 20, 20, 15, 5, -20,
-20, 5, 15, 20, 20, 15, 5, -20,
-30, 0, 10, 10, 10, 10, 0, -30, 
-40, -20, -10, 0, 0, -10, -20, -40,
-50, -40, -30, -30, -30, -30, -40, -50},

{-40, -35, -30, -25, -25, -30, -35, -40, /*Ferz*/
-30, 0, 5, 10, 10, 5, 0, -30,
-20, 5, 10, 20, 20, 20, 5, -20,
-15, 10, 10, 20, 20, 20, 15, -15,
-10, 5, 10, 15, 15, 10, 5, -10,
-20, 0, 5, 10, 10, 10, 5, -20, 
-30, -10, 0, 5, 5, 0, -10, -30,
-40, -35, -30, -25, -25, -30, -35, -40},

{0, 0, 0, 0, 0, 0, 0, 0, /*Alfils*/
5, 0, 0, 10, 10, 0, 0, 5,
0, 0, 0, 0, 0, 0, 0, 0,
0, 20, 30, 0, 0, 30, 20, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 15, 15, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, -30, -30, 0, 0, -30, -30, 0},

{0, 0, 0, 0, 0, 0, 0, 0, /* Pawns */
20, 20, 20, 25, 25, 20, 20, 20,
15, 15, 15, 20, 20, 15, 15, 15,
9, 10, 11, 15, 15, 10, 9, 8,
7, 8, 9, 10, 10, 8, 7, 6, 
5, 5, 5, 0, 0, 5, 5, 5,
-5, -5, -10, -15, -15, -10, -5, -5,
0, 0, 0, 0, 0, 0, 0, 0}
};

void printAsBitboard(uint64_t board)
{
    for (int i = 0; i < 64; i++)
    {
        std::cout << (board & 1ULL);
        if ((i & 7) == 7)
        {
            std::cout << '\n';
        }
        board >>= 1;
    }
    std::cout << '\n';
}

void printSidesBitboard(uint64_t *side)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            for (int k = 8 * i; k < 8 * i + 8; k++)
            {
                std::cout << ((side[j] >> k) & 1);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }
}

void printAllBitboards(){
    std::cout << "\nWhite's Pieces\n";
    printSidesBitboard(white);
    std::cout << "\nBlack's Pieces\n";
    printSidesBitboard(black);
}

void setStartPos(){
    black[0] = 8ULL;
    black[1] = 129ULL;
    black[2] = 66ULL;
    black[3] = 16ULL;
    black[4] = 36ULL;
    black[5] = RANK0 << 8;
    black[6] = RANK0 | (RANK0 << 8);

    for (int i = 0; i < 5; i++)
    {
        white[i] = black[i] << 56;
    }

    white[5] = (RANK0 << 48);
    white[6] = (RANK0 << 48) | (RANK0 << 56);

    toMove = true;
}

void loadPos(uint64_t*& wo, uint64_t*& bo, bool& to, bool forward){
    if (forward){
        for (int i = 0; i < 7; i++){
            wo[i] = white[i];
            bo[i] = black[i];
        }
        to = toMove;
    } else {
        for (int i = 0; i < 7; i++){
            white[i] = wo[i];
            black[i] = bo[i];
        }
        toMove = to;
    }
}

uint64_t genEmptyStraight(uint8_t start, uint8_t len){
    uint64_t res = 0;

    uint8_t r = start >> 3;
    uint8_t c = start & 7;

    uint64_t st = 1;
    st <<= start;

    uint64_t set = ((RANK0 << (8 * r)) ^ (FILE0 << c));

    res |= (st << 8 * len);
    res |= (st >> 8 * len);
    res |= (st >> len);
    res |= (st << len);

    res &= set;

    return res;
}

uint64_t genEmptyDiagonal(uint8_t start, uint8_t distance){
    uint64_t result = 0;
    uint8_t r = start >> 3;
    uint8_t c = start & 7;

    uint64_t set = 0;

    if (r >= c)
    {
        set |= (LDIAG << (8 * (r - c)));
    }
    if (r < c)
    {
        set |= (LDIAG >> (8 * (c - r)));
    }

    if (r + c <= 7)
    {
        set |= (RDIAG >> (8 * (7 - r - c)));
    }
    if (r + c > 7)
    {
        set |= (RDIAG << (8 * (r + c - 7)));
    }

    uint64_t st = 1;
    st <<= start;

    result |= (st >> (9 * distance));
    result |= (st << (9 * distance));
    result |= (st >> (7 * distance));
    result |= (st << (7 * distance));

    result &= set;

    return result;
}

uint64_t emptyBoardMoves(uint8_t type, uint8_t start){ // white = 1, black = 0
    uint64_t st = 0;
    uint64_t temp;

    //uint8_t r = start >> 3;
    uint8_t c = start & 7;

    switch (type){
        case 0: // King
            st = (genEmptyStraight(start, 1) | genEmptyDiagonal(start, 1));
            break;
        case 1: // Rook
            break;
        case 2: // Knight
            temp = 1ULL << start;
            if (c >= 2)
            {
                st |= (temp >> 10);
                st |= (temp << 6);
            }
            if (c >= 1)
            {
                st |= (temp >> 17);
                st |= (temp << 15);
            }
            if (c <= 5)
            {
                st |= (temp >> 6);
                st |= (temp << 10);
            }
            if (c <= 6)
            {
                st |= (temp >> 15);
                st |= (temp << 17);
            }
            break;
        case 3: // Ferz
            st = genEmptyDiagonal(start, 1);
            break;
        case 4: // Alfil
            st = genEmptyDiagonal(start, 2);
            break;
        case 5: // Pawn (shatranj)
            break;
        default:
            break;
        }

    return st;
}

void generateHorizontalLookup(){
    /*
    table is a u8[8][64];

    */
    std::bitset<8> middle;
    std::bitset<8> rMoves(0);

    tables[0][0][0] = 254; // edge case

    // suppose i = 0
    for (int j = 1; j < 64; j++)
    { // for each of the 63 remaining 6 bit combos
        for (int k = 0; k < __builtin_ctz(j << 1); k++)
        {                      // for each trailing unset bit and 1 after
            rMoves.set(k + 1); // set the in the rank move
        }
        tables[0][0][j] = rMoves.to_ullong();

        // std::cout << rMoves << '\t' << std::bitset<8>(j << 1) << '\n';

        rMoves.reset(); // reset our bitset
    }

    tables[0][7][0] = 127;
    // suppose i = 7
    for (int j = 1; j < 64; j++)
    {
        for (int k = 0; k < __builtin_clz(j << 1) - 24; k++)
        {
            rMoves.set(6 - k);
        }
        tables[0][7][j] = rMoves.to_ullong();

        // std::cout << rMoves << '\t' << std::bitset<8>(j << 1) << '\n';

        rMoves.reset();
    }

    int index;
    for (int i = 1; i < 7; i++)
    { // i = rookFile in the middle
        for (int j = 0; j < 64; j++)
        { // j = middle 6 bits
            middle = std::bitset<8>(j << 1);
            if (!middle[i])
            { // if ith bit is not set
                // no need to generate because ith bit must be set in occupied board.
                continue;
            }
            index = i - 1; // going left
            while (true)
            {
                rMoves.set(index);
                if (middle[index] or (index == 0))
                { // if we encounter a set bit or reach the end
                    break;
                }
                index--;
            }
            index = i + 1;
            while (true)
            {
                rMoves.set(index);
                if (middle[index] or (index == 7))
                { // same
                    break;
                }
                index++;
            }
            tables[0][i][j] = rMoves.to_ullong();

            //std::cout << i << '\t' << std::bitset<8>(j << 1) << '\t' << rMoves << '\n';

            rMoves.reset();
        }
    }
}

uint64_t hyperbolaQuintessence(uint8_t square){
    uint64_t forward;
    uint64_t reverse;
    forward = (white[6] | black[6]) & (FILE0 << (square & 7));

    // printAsBitboard(forward);

    reverse = __builtin_bswap64(forward);

    // uint64_t squarebb = 1ULL << square;
    forward -= 2 * (1ULL << square);
    reverse -= 2 * (1ULL << (56 ^ square));

    forward ^= __builtin_bswap64(reverse);
    forward &= (FILE0 << (square & 7));

    return forward;
}

void generateLeaperLookup(){
    //indexed u64[5][64]
    //i = 0 (King)
    for (int j = 0; j < 64; j++){
        tables[1][0][j] = emptyBoardMoves(0, j);
    }
    //i = 2 - 4 (knight, ferz, alfil)
    for (int i = 2; i < 5; i++){
        for (int j = 0; j < 64; j++){
            tables[1][i][j] = emptyBoardMoves(i, j);
        }
    }
}

void generatePawnLookup(){
    //table [2][48] 
    for (int j = 0; j < 63; j++){
        tables[2][0][j] = (5ULL << (j + 7)) & (RANK0 << ((j & 56) + 8));
        tables[2][1][j] = (5ULL << (j - 9)) & (RANK0 << ((j & 56) - 8));
    }
}

void generateLookupTables(){
    tables = new uint64_t**[3];

    tables[0] = new uint64_t*[8]; //rooks
    tables[1] = new uint64_t*[5]; //leapers
    tables[2] = new uint64_t*[2]; //pawns

    for (int i = 0; i < 8; i++){
        tables[0][i] = new uint64_t[64];
    }

    for (int i = 0; i < 5; i++){
        tables[1][i] = new uint64_t[64];
    }

    for (int i = 0; i < 2; i++){
        tables[2][i] = new uint64_t[64];
    }

    generateHorizontalLookup();
    generateLeaperLookup();
    generatePawnLookup();
}

void deleteLookupTables(){
    //0 = ranks, 1 = pawns, 2 = leaper
    for (int i = 0; i < 8; i++){
        delete[] tables[0][i];
    }
    delete[] tables[0];
    
    for (int i = 0; i < 5; i++){
        delete[] tables[1][i];
    }
    delete[] tables[1];

    delete[] tables[2][0];
    delete[] tables[2][1];
    delete[] tables[2];
}

uint8_t *bitboardToList(uint64_t board){
    int bits = __builtin_popcountll(board);
    uint8_t *list = new uint8_t[bits + 1];
    list[0] = bits;
    int b = 0;
    int index = 1;

    while (board)
    {
        if (board & 1)
        {
            list[index] = b;
            index++;
        }
        b++;
        board >>= 1;
    }

    return list;
}

void initializeAll(){
    white = new uint64_t[7];
    black = new uint64_t[7];
    setStartPos();
    nodes = 0;
    wtotal = 0;

    uint8_t* wlist;
    int values[6] = {0, 650, 400, 200, 150, 100};
    for (int i = 0; i < 6; i++){
        wlist = bitboardToList(white[i]);
        wtotal += (__builtin_popcountll(white[i]) * values[i]);
        for (int j = 0; j < wlist[0]; j++){
            wtotal += mps[i][wlist[j + 1]];
        }
        delete[] wlist;
    }

    btotal = wtotal;

    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            mps[i][j] += values[i];
        }
    }

    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 128; j++){
            moves[i][j] = 0;
        }
    }
    
    /*
    for (int i = 0; i < 33; i++){
        evalIncr[i] = 0;
    }
    */

    generateLookupTables();
}

void cleanupAll(){
    delete[] white;
    delete[] black;
    deleteLookupTables();
}

uint64_t fsinglemoveset(uint8_t start, bool piececolor, uint8_t type){
    uint64_t result;

    uint64_t friends, enemy; // temp variables to store friend/enemy bitboards.
    if (piececolor){ // assign white/black to friend/enemy, 1 = white.
        friends = white[6];
        enemy = black[6];
    } else {
        friends = black[6];
        enemy = white[6];
    }
    uint64_t occupied = white[6] | black[6];

    if (type == 1){ //rook
        result = hyperbolaQuintessence(start);
        uint64_t rankMask = RANK0 << ((start & 56));
        /*
        printAsBitboard(rankMask);

        printAsBitboard(occupied);

        uint64_t key = rankMask & occupied;
        printAsBitboard(key);

        uint64_t key2 = key >> (start & 56);
        printAsBitboard(key2);

        uint64_t key3 = (key2 >> 1) & 63;
        printAsBitboard(key3);


        uint64_t key4 = rlookup[start & 7][key3];
        std::cout << '\n' << std::bitset<8>(rlookup[start & 7][key3]) << "\n\n";

        uint64_t key5 = key4 << (start & 56);
        printAsBitboard(key5);
        */

        result |= ((uint64_t) tables[0][start & 7][(((rankMask & occupied) >> (start & 56)) >> 1) & 63]) << (start & 56);
        result &= ~friends;
    } else if (type == 5){
        result = tables[2][piececolor][start] & enemy;
        if (piececolor){
            result |= ((1ULL << (start - 8)) & ~occupied);
        } else {
            result |= ((1ULL << (start + 8)) & ~occupied);
        }
        
        
    } else {
        result = tables[1][type][start];
        result &= ~friends;
    }

    return result;
}

uint64_t *pseudolegal(){
    uint64_t *side = 0; // initialize pointers
    uint64_t *enemy = 0;

    if (toMove){ // depending on the color being generated, the friend and enemy pieces must be distinguished
        side = white;
        enemy = black;
    } else {
        side = black;
        enemy = white;
    }

    // std::cout << "Indicator 1\n";

    int total = __builtin_popcountll(side[6]); // total number of movesets = number of pieces

    uint64_t *result = new uint64_t[total + 1]; // initialize the result; the first element is how large the list is
    result[0] = total;                          // we fill that in now.

    int index = 1;       // initialize the index; this "points" to which element we are on
    uint8_t *pieces = 0; // placeholder variable to store the array generated from piece bitboards

    // std::cout << "Indicator 2\n\n";

    for (int i = 0; i < 6; i++)
    { // for each type of piece (excluding pawns)
        // std::cout << "Started Main Loop iter " << i << "\n\n";
        pieces = bitboardToList(side[i]); // convert piece bitboard to a list of starting squares
        for (int j = 0; j < pieces[0]; j++)
        { // iterate through the piece starting squares and generate moves for the specified piece type
            result[index] = fsinglemoveset(pieces[j + 1], toMove, i);
            // the f(inal)single(piece)moveset for each piece written into the set of movesets
            // parameters: the location, piececolor, white pieces, black pieces, and piece type
            index++;
        }
        delete[] pieces;
    }

    return result;
}

uint8_t *orderedStartingSquares(uint64_t *side)
{
    uint8_t *pieces = 0;

    int total = __builtin_popcountll(side[6]); // total number of movesets = number of pieces

    uint8_t *result = new uint8_t[total + 1];
    result[0] = total;

    int index = 1;

    for (int i = 0; i < 6; i++)
    {
        pieces = bitboardToList(side[i]);
        for (int j = 0; j < pieces[0]; j++)
        {
            result[index] = pieces[j + 1];
            index++;
        }
        delete[] pieces;
    }

    // std::cout << numPieces;

    return result;
}

uint8_t determineCapture(uint64_t *opp, uint8_t square)
{
    uint64_t asbitboard = 1ULL << square;
    if (asbitboard & opp[6])
    {
        for (int i = 1; i < 6; i++)
        {
            if (asbitboard & opp[i])
            {
                return i;
            }
        }
    }
    return 0;
}

void printMoveAsBinary(uint32_t move)
{
    int spaces[8] = {25, 19, 18, 15, 12, 11, 8, 7};
    uint32_t msb = 1 << 31;
    for (int i = 0; i < 32; i++)
    {
        std::cout << move / msb;
        move <<= 1;
        for (int j : spaces)
        {
            if (i == j)
            {
                std::cout << ' ';
                break;
            }
        }
    }
    std::cout << '\n';
}

uint32_t *movesetToMoves(uint8_t start, uint64_t set, uint8_t type)
{
    uint8_t *destinations = bitboardToList(set);
    uint32_t *resultMoves = new uint32_t[27];

    // uint8_t* typeToList = decidePieceList(mode);

    uint8_t captured;

    uint64_t *enemy;
    if (toMove)
    {
        enemy = black;
    }
    else
    {
        enemy = white;
    }

    resultMoves[0] = destinations[0];
    int index = 1;

    for (int i = 1; i < destinations[0] + 1; i++)
    {
        resultMoves[index] = (uint32_t)start;

        // printMoveAsBinary(resultMoves[index]);

        resultMoves[index] |= ((uint32_t)destinations[i] << 6);

        // printMoveAsBinary(resultMoves[index]);

        captured = determineCapture(enemy, destinations[i]);
        if (captured != 0)
        {
            resultMoves[index] |= (((uint32_t)1) << 12);
            resultMoves[index] |= (((uint32_t)captured) << 13);
        }
        resultMoves[index] |= ((uint32_t)type << 16);
        resultMoves[index] |= ((uint32_t)type << 20);

        // printMoveAsBinary(resultMoves[index]);

        if (type == 5)
        {
            bool isPromoting = (1ULL << destinations[i]) & ((RANK0) | (RANK0 << 56));
            if (isPromoting)
            {
                resultMoves[index] |= ((uint32_t)1) << 19;

                uint32_t endpiecereset = 0xFF8FFFFF;
                resultMoves[index] &= endpiecereset; // wipe the bits since they were set before.
                resultMoves[index] |= ((uint32_t)3) << 20;
            }
            else
            {
                resultMoves[index] |= ((uint32_t)5) << 20;
            }
            // printMoveAsBinary(resultMoves[index]);
        }

        resultMoves[index] |= ((uint32_t)toMove << 23);
        // printMoveAsBinary(resultMoves[index]);
        index++;
    }

    resultMoves[0] = index - 1;

    delete[] destinations;
    // delete[] typeToList;

    return resultMoves;
}

uint8_t *orderedPieceIndices(uint64_t *side)
{
    int bits = __builtin_popcountll(side[6]);
    uint8_t *result = new uint8_t[bits + 1];
    result[0] = bits;

    int index = 1;
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < __builtin_popcountll(side[i]); j++)
        {
            result[index] = i;
            index++;
        }
    }

    return result;
}

void fullMoveGen(int ply){
    uint64_t *moveSetList = pseudolegal(); // generate the pseudolegal moves
    //std::cout << "Pseudolegals Generated\n";
    uint64_t *fr;                          // assign friend and enemy pointers
    uint64_t *en;

    if (toMove){
        fr = white;
        en = black;
    } else {
        fr = black;
        en = white;
    }

    uint8_t *startSquares = orderedStartingSquares(fr); // find the starting squares of each piece in ORDER
    uint8_t *pieceIndices = orderedPieceIndices(fr);

    int bits = __builtin_popcountll(fr[6]);

    uint32_t **moveListList = new uint32_t *[bits]; // initialize list of move lists obtained from bboards
    for (int i = 0; i < moveSetList[0]; i++)
    {
        moveListList[i] = movesetToMoves(startSquares[i + 1], moveSetList[i + 1], pieceIndices[i + 1]);
    }

    int total = 0;
    for (int i = 0; i < bits; i++){
        total += moveListList[i][0];
    }

    moves[ply][0] = total;

    int index = 1;

    for (int i = 0; i < bits; i++){ // each piece has a movelist with it
        for (int j = 0; j < (moveListList[i][0]); j++){
            moves[ply][index] = moveListList[i][j + 1];
            index++;
        }
    }

    /*
    for (int i = 0; i < finalMoveList[0]; i++){
        printMoveAsBinary(finalMoveList[i + 1]);
    }
    std::cout << finalMoveList[0];
    */

    for (int i = 0; i < bits; i++)
    {
        delete[] moveListList[i];
    }
    delete[] moveListList;

    delete[] startSquares;
    delete[] pieceIndices;
    delete[] moveSetList;
}

/*
Move Representation:

0000 0000 0000 0000 0000 000000 000000

0-5: start square
6-11: end square

12: Capture
13-15: Captured Type
16-18: Piece Type Moved
19: Promotion
20-22: Piece End Type
23: Color

*/

std::string moveToAlgebraic(uint32_t move){
    uint8_t start = move & 63U;
    uint8_t end = (move >> 6) & 63U;

    std::string result;

    result += ((start & 7) + 97);
    result += (8 - (start >> 3)) + 48;

    result += ((end & 7) + 97);
    result += (8 - (end >> 3)) + 48;

    if ((move >> 19) & 1U)
    {
        result += 'q';
    }

    return result;
}

void evaluateMove(uint32_t& move, int& bonus, int& cap){
    int ept = (move >> 20) & 7;
    int esq = (move >> 6) & 63U;

    int ipt = (move >> 16) & 7;
    int isq = (move & 63U);

    bool col = move & (1U << 23);

    cap = 0;

    if ((move >> 12) & 1){ //if there is a capture
        if (col){ //if it is white's move (captured black piece), get psqt for that
            cap = mps[(move >> 13) & 7][esq^56];
        } else { //if it is black's move (captured white piece)< get psqt for that as well
            cap = mps[(move >> 13) & 7][esq];
        }
    }

    if (!col){ //now we compute our own bonus. If we have the black pieces, XOR with 56
        esq ^= 56; 
        isq ^= 56;
    }

    bonus = mps[ept][esq] - mps[ipt][isq]; //regardless, this will always be the formula. 
}

void makeMove(uint32_t move, bool forward, bool ev){
    // true -> make, false -> unmake

    uint8_t startsquare = move & (63U);
    uint8_t endsquare = (move >> 6) & (63U);
    // std::cout << (int) startsquare << ' ' << (int) endsquare << '\n';

    bool capturing = (move >> 12) & (1U);
    uint8_t captureType = (move >> 13) & (7U);

    uint8_t typeMoved = (move >> 16) & (7U);

    bool promoting = (move >> 19) & (1U);

    uint8_t typeEnded = (move >> 20) & (7U);

    bool color = move >> 23;

    /*
    std::cout << color << ' ' << (int) typeEnded << ' ' << promoting
        << ' ' << (int) typeMoved << ' ' << (int) captureType << ' ' << capturing
        << ' ' << (int) endsquare << ' ' << (int) startsquare << '\n';
    */

    uint64_t *fr;
    uint64_t *en;

    if (color){
        fr = white;
        en = black;
    } else {
        fr = black;
        en = white;
    }

    fr[6] ^= ((1ULL << startsquare) | (1ULL << endsquare)); // update location of own pieces
    fr[typeMoved] ^= (1ULL << startsquare);                 // remove piece its starting square & type
    fr[typeEnded] ^= (1ULL << endsquare);                   // place the piece on its ending square & type

    if (capturing){                                           // if capturing
        en[captureType] ^= (1ULL << endsquare); // remove the captured piece from its square
        en[6] ^= (1ULL << endsquare);           // update the location of opp's pieces by removing the victim's bit
    }

    //std::cout << moveToAlgebraic(move) << '\n';

    if (forward){
        nodes++;
    }

    if (ev){
        int b, c;
        evaluateMove(move, b, c);
        if (forward){
            if (toMove){
                wtotal += b; btotal -= c;
            } else {
                wtotal -= c; btotal += b;
            }
        } else {
            if (!toMove){
                wtotal -= b; btotal += c;
            } else {
                wtotal += c; btotal -= b;
            }
        }
    }

    toMove = !toMove;
}

bool isChecked(){
    // uint8_t kingsquare = 0;
    uint64_t *fr;
    uint64_t *en;
    //std::cout << '\n' << toMove << '\n';
    if (!toMove){
        fr = white;
        en = black;
    } else {
        en = white;
        fr = black;
    }
    /*
    uint64_t kingbb = fr[0];
    // printAsBitboard(kingbb);
    while (kingbb > 1ULL)
    {
        kingbb >>= 1;
        kingsquare++;
    }
    */
    uint8_t kingsquare = __builtin_ctzll(fr[0]);

    uint64_t phantombitboard;
    for (int i = 0; i < 5; i++){ // all except pawns
        phantombitboard = fsinglemoveset(kingsquare, !toMove, i);
        //printAsBitboard(phantombitboard);
        if (__builtin_popcountll(phantombitboard & en[i])){ // if the phantom piece can capture an opposing piece of the same type
            //std::cout << "\nCheck Received from piece type: " << i << '\n';
            return true;
        }
    }

    uint64_t possibleCaptures = tables[2][!toMove][kingsquare];
    /*
    genEmptyDiagonal(kingsquare, 1); // initialize
    int rank = kingsquare >> 3;
    if (!toMove)
    {
        possibleCaptures &= (RANK0 << (8 * (rank - 1))); // ensure captures are in correct direction
    }
    else
    {
        possibleCaptures &= (RANK0 << (8 * (rank + 1)));
    }
    */
    if (en[5] & possibleCaptures){
        return true;
    }
    return false;
}

void endHandle(){
    if (timeExpired){
        throw "Time Expired\n";
    }
    if (nodes > mnodes){
        throw "Nodes Exceeded\n";
    }
}

uint64_t perft(int depth, int ply){
    uint64_t nodes = 0; // initialize
    if (depth == 0){ //
        return 1ULL;
    }

    uint64_t additional = 0;

    fullMoveGen(ply); //generate for whoever's move it is
    for (int i = 0; i < moves[ply][0]; i++){
        makeMove(moves[ply][i + 1], 1, 0);

        endHandle();
        
        if (isChecked()){                                            // if is checked
            makeMove(moves[ply][i + 1], 0, 0); // unmake the move
            // printMoveAsBinary(moves[ply][i + 1]);
            // std::cout << moveToAlgebraic(moves[ply][i + 1]) << " rejected by check on " << color << '\n';
            // printSidesBitboard(black);

            continue; // continue to the next iteration.
        }
        additional = perft(depth - 1, ply + 1);

        if (ply == 0)
        {
            std::cout << moveToAlgebraic(moves[ply][i + 1]) << ": " << additional << '\n';
            //printMoveAsBinary(moves[ply][i + 1]);
        }

        nodes += additional;
        makeMove(moves[ply][i + 1], 0, 0);
    }
    return nodes;
}

int pieceFenIndex(char p){
    if (p > 64 and p < 91){
        p += 32;
    }
    char orderedpieces[6] = {'k', 'r', 'n', 'q', 'b', 'p'};
    for (int i = 0; i < 6; i++){
        if (p == orderedpieces[i]){
            return i;
        }
    }
    return -1;
}

void placeFenPiece(char p, int square){
    int pieceindex = pieceFenIndex(p);
    uint64_t piecebb = 1ULL << square;

    if (p > 96){
        black[pieceindex] |= piecebb;
        black[6] |= piecebb;
    }
    if (p < 91){
        white[pieceindex] |= piecebb;
        white[6] |= piecebb;
    }
}

/*
Fen Format
[board setup] [w/b to move] [castling] [en passant] [half move - 50 moves] [full move]

Example: BqNN4/3b4/pN1b4/1b1BN3/3b1PP1/Pr1NP1b1/4P1P1/qkB3KQ w - - 0 1
*/

void emptyBoard(){
    for (int i = 0; i < 7; i++){
        white[i] = 0;
        black[i] = 0;
    }
}

uint8_t readFen(std::string fen){
    emptyBoard();

    int index = 0; // char by char reader
    int currSquare = 0;
    char currChar;

    while (currSquare < 64){
        currChar = fen[index];
        if (currChar == '/')
        { // if there is a slash, ignore it
            index++;
            continue;
        }
        if (currChar > 48 and currChar < 57)
        { // if we encounter a number, move forward that amount
            index++;
            currSquare += (currChar - 48);
            continue;
        }
        placeFenPiece(fen[index], currSquare); // otherwise, assume its a piece and place it
        // std::cout << fen[index] << '\t' << (int) fen[index] << '\t' << currSquare << '\t' << index << '\n';
        currSquare++; // update the square as needed.
        index++;      // and the index, too.
    }

    index++;                 // skip over a space
    toMove = fen[index] & 1; //'w' = 119, and 'b' = 98.
 
    index += 6;
    // std::cout << fen[index - 1];

    if (index > fen.length()){ // some FEN's don't contain the move counters
        return 0;
    }

    std::string hmovestring = "";
    while (fen[index] != ' '){
        hmovestring += fen[index];
        index++;
    }
    uint8_t halfmoves = std::stoi(hmovestring);

    index += 9;

    if (index > fen.length()){
        return halfmoves;
    }


    std::string movesstring = fen.substr(index);
    std::stringstream movestream(movesstring);

    int extraMoves = 1;
    for (char c : movesstring){
        if (c == ' ')
        {
            extraMoves++;
        }
    }

    std::string extraMoveList[extraMoves];
    for (int i = 0; i < extraMoves; i++){
        movestream >> extraMoveList[i];
        // std::cout << extraMoveList[i] << '\n';
    }
    //bool color = 1;

    for (std::string m : extraMoveList)
    {                                                 // for each move found
        fullMoveGen(0); // generate all the moves
        for (int i = 0; i < moves[0][0]; i++)
        { // for each of the moves generated
            if (moveToAlgebraic(moves[0][i + 1]) == m){     // get their alg representation and compare
                makeMove(moves[0][i + 1], 1, 0); // if so, make the move
            }
        }
        //color = !color;
    }

    return halfmoves;
}


