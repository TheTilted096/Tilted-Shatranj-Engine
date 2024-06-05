/*
This is the base code of Tilted refactoring.
Tuning and Testing is very difficult without an Object-Oriented design,
so this is a necessary step in maintainability.

TheTilted096, 5-25-2024
*/

#include <chrono>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#pragma GCC target("popcnt")
#pragma GCC optimize("O2")

#define RANK0 0xFFULL                // 8th rank
#define RANK7 0xFF00000000000000ULL  // 1st rank

#define FILE0 0x0101010101010101ULL  // A File

#define LDIAG 0x8040201008040201ULL  // Long Diagonal (Left)
#define RDIAG 0x0102040810204080ULL  // long diagonal (right)

//Movegen Tables
uint64_t llt[5][64];
uint64_t plt[2][64];
uint8_t hlt[8][64];

//Zobrist Hashing
uint64_t zpk[2][6][64];
uint64_t ztk;

//Elementary Eval Constants
int phases[6] = {0, 6, 4, 2, 1, 1};
int matVals[6] = {0, 650, 400, 200, 150, 100};
int totalPhase = 64;

void printAsBitboard(uint64_t board){
    for (int i = 0; i < 64; i++) {
        std::cout << (board & 1ULL);
        if ((i & 7) == 7) {
            std::cout << '\n';
        }
        board >>= 1;
    }
    std::cout << '\n';
}

void genLeaperLookup(){
    // Kings
    llt[0][0] = 0x302ULL;
    llt[0][7] = 0xC040ULL;
    for (int i = 0; i < 6; i++) {
        llt[0][i + 1] = (0x705ULL << i);
    }

    for (int i = 0; i < 41; i += 8) {
        llt[0][i + 8] = 0x30203ULL << i;
        llt[0][i + 15] = 0xC040C0ULL << i;
        for (int j = 0; j < 6; j++) {
            llt[0][i + j + 9] = 0x70507ULL << (i + j);
        }
    }

    llt[0][56] = 0x0203000000000000ULL;
    llt[0][63] = 0x40C0000000000000ULL;
    for (int i = 0; i < 6; i++) {
        llt[0][i + 57] = 0x0507000000000000ULL << i;
    }

    // Knights
    llt[2][0] = 0x020400ULL;
    llt[2][1] = 0x050800ULL;
    llt[2][6] = 0xA01000ULL;
    llt[2][7] = 0x402000ULL;

    llt[2][8] = 0x02040004ULL;
    llt[2][9] = 0x05080008ULL;
    llt[2][14] = 0xA0100010ULL;
    llt[2][15] = 0x40200020ULL;

    llt[2][48] = 0x400040200000000ULL;
    llt[2][49] = 0x800080500000000ULL;
    llt[2][54] = 0x100010A000000000ULL;
    llt[2][55] = 0x2000204000000000ULL;

    llt[2][56] = 0x4020000000000ULL;
    llt[2][57] = 0x8050000000000ULL;
    llt[2][62] = 0x10A00000000000ULL;
    llt[2][63] = 0x20400000000000ULL;

    for (int i = 0; i < 4; i++) {
        llt[2][i + 58] = 0x110A0000000000ULL << i;
        llt[2][i + 50] = 0x1100110A00000000ULL << i;
        llt[2][i + 2] = 0xA1100ULL << i;
        llt[2][i + 10] = 0xA110011ULL << i;
    }

    for (int i = 0; i < 25; i += 8) {
        llt[2][i + 16] = 0x204000402ULL << i;
        llt[2][i + 17] = 0x508000805ULL << i;
        llt[2][i + 22] = 0xA0100010A0ULL << i;
        llt[2][i + 23] = 0x4020002040ULL << i;
    }

    for (int i = 0; i < 25; i += 8) {
        for (int j = 0; j < 4; j++) {
            llt[2][i + j + 18] = 0xA1100110AULL << (i + j);
        }
    }

    // Ferz
    llt[3][0] = 0x200ULL;
    llt[3][7] = 0x4000ULL;
    llt[3][56] = 0x2000000000000ULL;
    llt[3][63] = 0x40000000000000ULL;

    for (int i = 0; i < 41; i += 8) {
        llt[3][i + 8] = 0x20002ULL << i;
        llt[3][i + 15] = 0x400040ULL << i;
    }

    for (int i = 0; i < 6; i++) {
        llt[3][i + 1] = 0x500ULL << i;
        llt[3][i + 57] = 0x5000000000000ULL << i;
    }

    for (int i = 0; i < 41; i += 8) {
        for (int j = 0; j < 6; j++) {
            llt[3][i + j + 9] = 0x50005ULL << (i + j);
        }
    }

    // alfils
    llt[4][0] = 0x40000ULL;
    llt[4][1] = 0x80000ULL;
    llt[4][8] = 0x4000000ULL;
    llt[4][9] = 0x8000000ULL;

    llt[4][6] = 0x100000ULL;
    llt[4][7] = 0x200000ULL;
    llt[4][14] = 0x10000000ULL;
    llt[4][15] = 0x20000000ULL;

    llt[4][48] = 0x400000000ULL;
    llt[4][49] = 0x800000000ULL;
    llt[4][56] = 0x40000000000ULL;
    llt[4][57] = 0x80000000000ULL;

    llt[4][54] = 0x1000000000ULL;
    llt[4][55] = 0x2000000000ULL;
    llt[4][62] = 0x100000000000ULL;
    llt[4][63] = 0x200000000000ULL;

    for (int i = 0; i < 25; i += 8) {
        llt[4][i + 16] = 0x400000004ULL << i;
        llt[4][i + 17] = 0x800000008ULL << i;
        llt[4][i + 22] = 0x1000000010ULL << i;
        llt[4][i + 23] = 0x2000000020ULL << i;

        for (int j = 0; j < 4; j++) {
            llt[4][i + j + 18] = 0x1100000011ULL << (i + j);
        }
    }

    for (int i = 0; i < 4; i++) {
        llt[4][i + 2] = 0x110000ULL << i;
        llt[4][i + 10] = 0x11000000ULL << i;
        llt[4][i + 50] = 0x1100000000ULL << i;
        llt[4][i + 58] = 0x110000000000ULL << i;
    }
}

void genPawnLookup(){
    for (int i = 0; i < 49; i += 8) {
        plt[0][i] = 0x200ULL << i;
        plt[0][i + 7] = 0x4000ULL << i;

        plt[1][i + 8] = 0x2ULL << i;
        plt[1][i + 15] = 0x40ULL << i;

        for (int j = 0; j < 6; j++) {
            plt[0][i + j + 1] = 0x500ULL << (i + j);
            plt[1][i + j + 9] = 0x5ULL << (i + j);
        }
    }
}

void genHorizontalLookup(){
    // i = 0
    /*
    1 - - - - - - -
    */
    hlt[0][0] = 254;
    for (int j = 1; j < 64; j++) {
        hlt[0][j] = ((1 << (__builtin_ctz(j) + 1)) - 1) << 1;
    }

    // i = 7
    /*

    - | - - - 1 - - | 1
    j = 110100 lsb -> msb
    temp = 0110 1000
    ans = 0000 1110

    */

    hlt[7][0] = 127;
    uint8_t temp;
    for (int j = 1; j < 64; j++) {
        hlt[7][j] = 0;
        temp = (j << 1);
        for (int k = 6; !(temp & (1 << (k + 1))); k--) {
            hlt[7][j] |= (1 << k);
        }
    }

    for (int i = 1; i < 7; i++) {
        for (int j = 0; j < 64; j++) {
            hlt[i][j] = 0;
            temp = (j << 1);
            if (!((temp >> i) & 1)) {
                continue;
            }
            temp ^= (1 << i);
            for (int k = i - 1; !(temp & (1 << (k + 1))) and (k >= 0); k--) {
                hlt[i][j] |= (1 << k);
            }
            for (int k = i + 1; !(temp & (1 << (k - 1))) and (k < 8); k++) {
                hlt[i][j] |= (1 << k);
            }
        }
    }
}

void initializeTables(){
    genLeaperLookup();
    genPawnLookup();
    genHorizontalLookup();

    //Zobrist Hash Initialize
    std::mt19937_64 mt{0xDE92F5AD5A5EAD57};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 64; k++) {
                zpk[i][j][k] = mt();
            }
        }
    }
    ztk = mt();
}

void printMoveAsBinary(uint32_t move){
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

std::string moveToAlgebraic(uint32_t& move){
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

//Transposition Table Entry
class TTentry{
    public:
        int eScore;
        uint64_t eHash;
        int enType;
        int eDepth;
        uint32_t eMove;
        uint64_t* zhist;

        TTentry();
        TTentry(uint64_t[]);
        void update(int&, int, int&, uint32_t, int);
        void reset();
        void print();
};

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
        << "\tdepthAt: " << eDepth << "     nodeType: " << enType << "     debugMove: " 
            << moveToAlgebraic(eMove) << '\n';
}

struct EvalVars{
    int* rc;
    int* aw;
};