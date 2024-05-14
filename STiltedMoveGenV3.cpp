/*
3rd Version of Tilted MoveGen
4-20-2024
TheTilted096
*/

#include <chrono>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#pragma GCC target("popcnt")
#pragma GCC optimize("O2")

#define RANK0 0xFFULL                // 8th rank
#define RANK7 0xFF00000000000000ULL  // 1st rank

#define FILE0 0x0101010101010101ULL  // A File

#define LDIAG 0x8040201008040201ULL  // Long Diagonal (Left)
#define RDIAG 0x0102040810204080ULL  // long diagonal (right)

uint64_t pieces[14];
bool toMove;

uint64_t llt[5][64];
uint64_t plt[2][64];
uint8_t hlt[8][64];

uint32_t moves[96][128];
int mprior[96][128];

uint64_t nodes;
uint64_t mnodes = 0x3FFFFFFFULL;
volatile bool timeExpired;

int scores[2];
int eScores[2];

int thm;        // total half moves
int chm[1024];  // current half move array

uint64_t zpk[2][6][64];
uint64_t ztk;
uint64_t zhist[1024];

int phases[6] = {0, 6, 4, 2, 1, 1};
int totalPhase = 64;
int inGamePhase;
// 0 = king, 1 = rook, 2 = knight, 3 = ferz, 4 = alfil, 5 = pawn, 6 = total
// 2 * 6 + 2 * 4 + 2 * 2 + 2 * 1 + 8 * 1 = 32

int lmrReduces[64][128];

/* Original Piece Square Tables
int mps[6][64] =
{
{-50, -40, -30, -20, -20, -30, -40, -50,
-40, -10, 0, 0, 0, 0, -10, -40,
-30, 5, 10, 15, 15, 10, 5, -30,
-25, 10, 15, 15, 15, 15, 10, -25,
-20, 10, 15, 15, 15, 15, 10, -20,
-25, 5, 15, 15, 10, 5, 5, -25,
-30, 0, 25, 20, 0, 0, 0, -30,
-40, -30, -20, -10, -10, -20, -30, -40},

{0, 0, 0, 0, 0, 0, 0, 0,
15, 30, 30, 30, 30, 30, 30, 15,
0, 0, 0, 0, 0, 0, 0, 0,
-15, 0, 0, 0, 0, 0, 0, -15,
-15, 0, 0, 0, 0, 0, 0, -15,
-15, 0, 0, 0, 0, 0, 0, -15,
-15, 0, 0, 0, 0, 0, 0, -15,
-30, 0, 15, 30, 30, 15, 0, -30},

{-50, -40, -30, -30, -30, -30, -40, -50,
-40, -20, -10, 0, 0, -10, -20, -40,
-30, 5, 10, 10, 10, 10, 5, -30,
-20, 5, 15, 20, 20, 15, 5, -20,
-20, 5, 15, 20, 20, 15, 5, -20,
-30, 0, 10, 10, 10, 10, 0, -30,
-40, -20, -10, 0, 0, -10, -20, -40,
-50, -40, -30, -30, -30, -30, -40, -50},

{-40, -35, -30, -25, -25, -30, -35, -40,
-30, 0, 5, 10, 10, 5, 0, -30,
-20, 5, 10, 20, 20, 20, 5, -20,
-15, 10, 10, 30, 30, 20, 15, -15,
-10, 5, 15, 30, 30, 15, 5, -10,
-20, 0, 5, 20, 20, 20, 5, -20,
-30, -10, 0, 15, 15, 0, -10, -30,
-40, -35, -30, -25, -25, -30, -35, -40},

{0, 0, 0, 0, 0, 0, 0, 0,
5, 0, 0, 10, 10, 0, 0, 5,
0, 0, 0, 0, 0, 0, 0, 0,
0, 20, 30, 0, 0, 30, 20, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 10, 10, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, -20, -20, 0, 0, -20, -20, 0},

{0, 0, 0, 0, 0, 0, 0, 0,
20, 20, 25, 30, 30, 25, 25, 20,
15, 19, 18, 28, 28, 19, 18, 15,
9, 10, 11, 25, 25, 10, 9, 8,
12, 12, 20, 25, 25, 20, 12, 12,
15, 16, 17, 20, 20, 16, 15, 14,
-5, -10, -20, -30, -30, -20, -10, -5,
0, 0, 0, 0, 0, 0, 0, 0}
};

int eps[6][64] =
{
{-100, -90, -80, -70, -70, -80, -90, -100,
-90, -40, -30, -20, -20, -30, -40, -90,
-80, -30, 15, 35, 35, 15, -30, -80,
-70, -20, 35, 70, 70, 35, -20, -70,
-70, -20, 35, 70, 70, 35, -20, -70,
-80, -30, 15, 35, 35, 15, -30, -80,
-90, -40, -30, -20, -20, -30, -40, -90,
-100, -90, -80, -70, -70, -80, -90, -100},

{0, 0, 0, 0, 0, 0, 0, 0,
5, 10, 10, 10, 10, 10, 10, 5,
0, 0, 0, 0, 0, 0, 0, 0,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-5, 0, 0, 0, 0, 0, 0, -5,
-10, 0, 5, 10, 10, 5, 0, -10},

{-50, -40, -30, -30, -30, -30, -40, -50,
-40, -20, -10, 0, 0, -10, -20, -40,
-30, 5, 10, 10, 10, 10, 5, -30,
-20, 5, 15, 20, 20, 15, 5, -20,
-20, 5, 15, 20, 20, 15, 5, -20,
-30, 0, 10, 10, 10, 10, 0, -30,
-40, -20, -10, 0, 0, -10, -20, -40,
-50, -40, -30, -30, -30, -30, -40, -50},

{-40, -35, -30, -25, -25, -30, -35, -40,
-30, 0, 5, 10, 10, 5, 0, -30,
-20, 5, 10, 20, 20, 20, 5, -20,
-15, 10, 10, 20, 20, 20, 15, -15,
-10, 5, 15, 20, 20, 15, 5, -10,
-20, 0, 5, 20, 20, 20, 5, -20,
-30, -10, 0, 15, 15, 0, -10, -30,
-40, -35, -30, -25, -25, -30, -35, -40},

{0, 0, 0, 0, 0, 0, 0, 0,
5, 0, 0, 10, 10, 0, 0, 5,
0, 0, 0, 0, 0, 0, 0, 0,
0, 20, 30, 0, 0, 30, 20, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 10, 10, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, -20, -20, 0, 0, -20, -20, 0},

{0, 0, 0, 0, 0, 0, 0, 0,
20, 20, 25, 30, 30, 25, 25, 20,
15, 19, 18, 28, 28, 19, 18, 15,
9, 10, 11, 25, 25, 10, 9, 8,
12, 12, 20, 25, 25, 20, 12, 12,
15, 16, 17, 20, 20, 16, 15, 14,
-5, -10, -20, -30, -30, -20, -10, -5,
0, 0, 0, 0, 0, 0, 0, 0}
};
*/

int mps[6][64] = 
{{-38, -55, -15, -5, -5, -15, -25, -35, 
-55, -18, 15, 15, 15, 15, 5, -25, 
-45, -8, 18, 11, 0, 25, 12, -34, 
-40, -5, 4, 0, 0, 22, 25, -10, 
-6, -5, 2, 0, 0, 0, 14, -33, 
-10, 15, 9, 15, 25, 7, 20, -12, 
-16, 15, 10, 9, 4, 15, 15, -21, 
-26, -45, -5, 2, 5, -5, -15, -32},

{15, 5, -15, -15, -15, -15, -14, 15, 
2, 15, 39, 15, 17, 26, 15, 28, 
-5, 3, -15, -15, -15, 9, 1, 11, 
-4, -8, 15, 15, -15, 15, -6, -11, 
0, 15, -15, 1, -15, 14, 15, -2, 
0, 15, 15, -15, 15, -8, 14, -4, 
-30, -15, -15, -15, -15, 12, -2, -30, 
-15, -15, 23, 15, 15, 16, 15, -15},

{-65, -50, -15, -15, -45, -31, -46, -64, 
-55, -5, -25, -15, 15, -21, -5, -55, 
-32, -10, 20, -5, -3, 21, 2, -39, 
-5, 10, 0, 5, 31, 0, 19, -5, 
-5, 19, 0, 21, 34, 19, -10, -15, 
-35, 15, -5, 25, 22, 0, 15, -45, 
-28, -35, -20, -11, 10, -14, -35, -41, 
-38, -25, -15, -41, -43, -21, -25, -65},

{-30, -50, -23, -34, -15, -45, -50, -55, 
-18, 12, -10, -5, -5, -9, -15, -42, 
-29, 9, 8, 5, 7, 5, 14, -5, 
-30, 1, 25, 15, 44, 5, 15, -30, 
-14, -10, 0, 43, 15, 29, 19, -25, 
-5, -6, -5, 5, 19, 5, -10, -18, 
-35, -25, -7, 0, 0, -13, -17, -45, 
-38, -35, -15, -39, -25, -27, -50, -40},

{0, 0, 0, 0, 0, 0, 0, 0, 
-10, 0, 0, 15, -5, 0, 0, -10, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 5, 15, 0, 0, 19, 5, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
-15, 0, 0, -5, -5, 0, 0, 7, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, -35, -32, 0, 0, -13, -35, 0},

{0, 0, 0, 0, 0, 0, 0, 0, 
6, 5, 12, 15, 15, 10, 10, 5, 
0, 4, 3, 13, 13, 4, 3, 0, 
10, -5, -3, 36, 10, 16, -6, -7, 
-3, -3, 17, 31, 30, 19, -3, -3, 
0, 4, 27, 35, 19, 15, 1, -1, 
-20, -25, -10, -15, -15, -19, -25, -20, 
0, 0, 0, 0, 0, 0, 0, 0}};

int eps[6][64] = 
{{-85, -77, -65, -55, -55, -65, -75, -85, 
-83, -25, -15, -5, -5, -15, -25, -75, 
-65, -15, 30, 20, 20, 30, -15, -65, 
-55, -5, 23, 55, 55, 24, -5, -55, 
-63, -5, 20, 55, 55, 33, -5, -55, 
-65, -15, 23, 22, 43, 21, -15, -65, 
-75, -25, -15, -5, -5, -15, -25, -75, 
-85, -92, -65, -56, -55, -65, -75, -85},

{15, 6, -14, -12, 10, -13, -15, 15, 
-10, -5, -2, -4, -3, 1, -5, 11, 
-15, -15, -15, -15, -15, -15, -13, -15, 
10, 15, -9, 5, -15, -5, -15, -6, 
10, 15, -15, -15, -15, -4, 7, 10, 
10, -13, 2, 12, 15, -5, 13, 10, 
-10, 8, -15, -15, -14, -15, -14, -20, 
5, 0, 20, -2, 18, -5, 3, 5},

{-65, -27, -15, -16, -45, -19, -28, -36, 
-54, -5, -25, -15, 15, -12, -8, -55, 
-45, 12, 25, -5, -4, -4, -8, -45, 
-5, -6, 21, 5, 6, 0, 20, -34, 
-6, 19, 1, 5, 6, 10, 8, -13, 
-45, 15, -5, 14, -5, 19, 11, -15, 
-42, -31, -24, 0, 14, -14, -16, -25, 
-36, -25, -31, -40, -15, -15, -25, -56},

{-25, -50, -37, -40, -40, -45, -50, -55, 
-16, 12, -10, -5, -5, -10, -15, -40, 
-19, -10, -5, 5, 5, 5, 16, -5, 
-25, 1, -5, 5, 5, 5, 12, -30, 
-25, -10, 0, 5, 5, 0, 19, -25, 
-5, -9, -10, 5, 5, 5, -10, -5, 
-45, -17, -15, 0, 0, -15, -25, -16, 
-26, -35, -15, -40, -38, -45, -50, -40},

{0, 0, 0, 0, 0, 0, 0, 0, 
-10, 0, 0, -2, -5, 0, 0, -10, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, 5, 15, 0, 0, 15, 5, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 
-4, 0, 0, -5, -5, 0, 0, 3, 
0, 0, 0, 0, 0, 0, 0, 0, 
0, -27, -26, 0, 0, -29, -35, 0},

{0, 0, 0, 0, 0, 0, 0, 0, 
27, 5, 11, 15, 15, 10, 10, 5, 
15, 4, 3, 13, 13, 4, 3, 0, 
-4, -5, 6, 10, 10, -5, -6, -7, 
-3, -3, 5, 10, 10, 5, -3, -3, 
0, 1, 2, 23, 28, 1, 0, -1, 
-20, -25, -8, -15, -15, -5, -25, -18, 
0, 0, 0, 0, 0, 0, 0, 0}};

/* Some Piece Info
King = 0 (L)
Rook = 1 (H)
Knight = 2 (L)
Ferz = 3 (L)
Alfil = 4 (L)
Pawn = 5 (P)
*/

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

void printSidesBitboard(bool side){
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 7; j++) {
            for (int k = 8 * i; k < 8 * i + 8; k++) {
                std::cout << ((pieces[side * 7 + j] >> k) & 1);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }
}

void printAllBitboards(){
    std::cout << "\nWhite's Pieces\n";
    printSidesBitboard(1);
    std::cout << "\nBlack's Pieces\n";
    printSidesBitboard(0);
    std::cout << "toMove: " << toMove << "\n\n";
}

void setStartPos(){
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

    // PSQT initialize
    int values[6] = {0, 650, 400, 200, 150, 100};
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 64; j++) {
            mps[i][j] += values[i];
            eps[i][j] += values[i];
        }
    }

    // Zobrist Hash Initialize
    std::mt19937_64 mt{0xDE92F5AD5A5EAD57};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            for (int k = 0; k < 64; k++) {
                zpk[i][j][k] = mt();
            }
        }
    }
    ztk = mt();

    // LMR Table Initialize
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 128; j++){
            lmrReduces[i][j] = (int) (0.4 + log(i + 1) * log(j + 1) * 0.31);
        }
    }
}

void beginZobristHash(){
    // confirm zhist and chm initializations
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

uint64_t hyperbolaQuintessence(int& square){
    uint64_t forward = (pieces[6] | pieces[13]) & (FILE0 << (square & 7));
    uint64_t reverse = __builtin_bswap64(forward);

    forward -= 2 * (1ULL << square);
    reverse -= 2 * (1ULL << (56 ^ square));

    forward ^= __builtin_bswap64(reverse);
    forward &= (FILE0 << (square & 7));

    return forward;
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

int fullMoveGen(int ply, bool capsOnly){
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

void makeMove(uint32_t move, bool forward, bool ev){
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

bool isChecked(){  // checks if the opposing side is left in check; called after makeMove
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

void endHandle(){
    if (timeExpired) {
        throw "Time Expired\n";
    }
    if (nodes > mnodes) {
        throw "Nodes Exceeded\n";
    }
}

uint64_t perft(int depth, int ply){
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

// Example FEN: rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b - - 1 2 moves a8a8 a8a8

void readFen(std::string fen) {
    for (int i = 0; i < 14; i++) {
        pieces[i] = 0;
    }

    uint8_t sq = 0;
    int ind = -1;
    uint8_t p;

    char posChars[] = {'/', '1', '2', '3', '4', '5', '6', '7', '8',
                       'k', 'r', 'n', 'q', 'b', 'p', 'K', 'R', 'N', 'Q', 'B', 'P'};
    while (sq < 64) {
        ind++;
        for (int i = 0; i < 21; i++) {  // quantify the character
            if (fen[ind] == posChars[i]) {
                p = i;
                break;
            }
        }
        if ((0 < p) and (p < 9)) {
            sq += p;
            continue;
        }
        if ((p >= 9) and (p <= 14)) {
            pieces[p - 9] |= (1ULL << sq);
            pieces[6] |= (1ULL << sq);
            sq++;
        }
        if ((p >= 15) and (p <= 20)) {
            pieces[p - 8] |= (1ULL << sq);
            pieces[13] |= (1ULL << sq);
            sq++;
        }
    }

    inGamePhase = 0;               // calculate gamephase
    for (int i = 1; i < 6; i++) {  // note phases[0] = 0 (king)
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
        int nn = fullMoveGen(0, 0);
        for (int i = 0; i < nn; i++) {
            if (moveToAlgebraic(moves[0][i]) == fenTerm) {
                makeMove(moves[0][i], 1, 0);
                break;
            }
        }
    }
}
