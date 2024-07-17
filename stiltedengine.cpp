/*
Declaration and Methods of an Engine instance
TheTilted096, 5-25-2024
*/

/*
This is the base code of Tilted refactoring.
Tuning and Testing is very difficult without an Object-Oriented design,
so this is a necessary step in maintainability.

TheTilted096, 5-25-2024
*/

#include <cassert>
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

typedef uint64_t Bitboard;
typedef uint32_t Move;

class Bitboards{
    public:
        static constexpr Bitboard plt[2][64] =
        {{0x200ULL, 0x500ULL, 0xA00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xA000ULL, 0x4000ULL, 
        0x20000ULL, 0x50000ULL, 0xA0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xA00000ULL, 0x400000ULL, 
        0x2000000ULL, 0x5000000ULL, 0xA000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xA0000000ULL, 0x40000000ULL, 
        0x200000000ULL, 0x500000000ULL, 0xA00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xA000000000ULL, 0x4000000000ULL, 
        0x20000000000ULL, 0x50000000000ULL, 0xA0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xA00000000000ULL, 0x400000000000ULL, 
        0x2000000000000ULL, 0x5000000000000ULL, 0xA000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xA0000000000000ULL, 0x40000000000000ULL, 
        0x200000000000000ULL, 0x500000000000000ULL, 0xA00000000000000ULL, 0x1400000000000000ULL, 0x2800000000000000ULL, 0x5000000000000000ULL, 0xA000000000000000ULL, 0x4000000000000000ULL, 
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}, {
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 
        0x2ULL, 0x5ULL, 0xAULL, 0x14ULL, 0x28ULL, 0x50ULL, 0xA0ULL, 0x40ULL, 
        0x200ULL, 0x500ULL, 0xA00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xA000ULL, 0x4000ULL, 
        0x20000ULL, 0x50000ULL, 0xA0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xA00000ULL, 0x400000ULL, 
        0x2000000ULL, 0x5000000ULL, 0xA000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xA0000000ULL, 0x40000000ULL, 
        0x200000000ULL, 0x500000000ULL, 0xA00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xA000000000ULL, 0x4000000000ULL, 
        0x20000000000ULL, 0x50000000000ULL, 0xA0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xA00000000000ULL, 0x400000000000ULL, 
        0x2000000000000ULL, 0x5000000000000ULL, 0xA000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xA0000000000000ULL, 0x40000000000000ULL}};

        static constexpr Bitboard llt[5][64] =
        {{0x302ULL, 0x705ULL, 0xE0AULL, 0x1C14ULL, 0x3828ULL, 0x7050ULL, 0xE0A0ULL, 0xC040ULL, 
        0x30203ULL, 0x70507ULL, 0xE0A0EULL, 0x1C141CULL, 0x382838ULL, 0x705070ULL, 0xE0A0E0ULL, 0xC040C0ULL, 
        0x3020300ULL, 0x7050700ULL, 0xE0A0E00ULL, 0x1C141C00ULL, 0x38283800ULL, 0x70507000ULL, 0xE0A0E000ULL, 0xC040C000ULL, 
        0x302030000ULL, 0x705070000ULL, 0xE0A0E0000ULL, 0x1C141C0000ULL, 0x3828380000ULL, 0x7050700000ULL, 0xE0A0E00000ULL, 0xC040C00000ULL, 
        0x30203000000ULL, 0x70507000000ULL, 0xE0A0E000000ULL, 0x1C141C000000ULL, 0x382838000000ULL, 0x705070000000ULL, 0xE0A0E0000000ULL, 0xC040C0000000ULL, 
        0x3020300000000ULL, 0x7050700000000ULL, 0xE0A0E00000000ULL, 0x1C141C00000000ULL, 0x38283800000000ULL, 0x70507000000000ULL, 0xE0A0E000000000ULL, 0xC040C000000000ULL, 
        0x302030000000000ULL, 0x705070000000000ULL, 0xE0A0E0000000000ULL, 0x1C141C0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL, 0xE0A0E00000000000ULL, 0xC040C00000000000ULL, 
        0x203000000000000ULL, 0x507000000000000ULL, 0xA0E000000000000ULL, 0x141C000000000000ULL, 0x2838000000000000ULL, 0x5070000000000000ULL, 0xA0E0000000000000ULL, 0x40C0000000000000ULL}, {
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 
        0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL}, {
        0x20400ULL, 0x50800ULL, 0xA1100ULL, 0x142200ULL, 0x284400ULL, 0x508800ULL, 0xA01000ULL, 0x402000ULL, 
        0x2040004ULL, 0x5080008ULL, 0xA110011ULL, 0x14220022ULL, 0x28440044ULL, 0x50880088ULL, 0xA0100010ULL, 0x40200020ULL, 
        0x204000402ULL, 0x508000805ULL, 0xA1100110AULL, 0x1422002214ULL, 0x2844004428ULL, 0x5088008850ULL, 0xA0100010A0ULL, 0x4020002040ULL, 
        0x20400040200ULL, 0x50800080500ULL, 0xA1100110A00ULL, 0x142200221400ULL, 0x284400442800ULL, 0x508800885000ULL, 0xA0100010A000ULL, 0x402000204000ULL, 
        0x2040004020000ULL, 0x5080008050000ULL, 0xA1100110A0000ULL, 0x14220022140000ULL, 0x28440044280000ULL, 0x50880088500000ULL, 0xA0100010A00000ULL, 0x40200020400000ULL, 
        0x204000402000000ULL, 0x508000805000000ULL, 0xA1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL, 0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL, 
        0x400040200000000ULL, 0x800080500000000ULL, 0x1100110A00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL, 
        0x4020000000000ULL, 0x8050000000000ULL, 0x110A0000000000ULL, 0x22140000000000ULL, 0x44280000000000ULL, 0x88500000000000ULL, 0x10A00000000000ULL, 0x20400000000000ULL}, {
        0x200ULL, 0x500ULL, 0xA00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xA000ULL, 0x4000ULL, 
        0x20002ULL, 0x50005ULL, 0xA000AULL, 0x140014ULL, 0x280028ULL, 0x500050ULL, 0xA000A0ULL, 0x400040ULL, 
        0x2000200ULL, 0x5000500ULL, 0xA000A00ULL, 0x14001400ULL, 0x28002800ULL, 0x50005000ULL, 0xA000A000ULL, 0x40004000ULL, 
        0x200020000ULL, 0x500050000ULL, 0xA000A0000ULL, 0x1400140000ULL, 0x2800280000ULL, 0x5000500000ULL, 0xA000A00000ULL, 0x4000400000ULL, 
        0x20002000000ULL, 0x50005000000ULL, 0xA000A000000ULL, 0x140014000000ULL, 0x280028000000ULL, 0x500050000000ULL, 0xA000A0000000ULL, 0x400040000000ULL, 
        0x2000200000000ULL, 0x5000500000000ULL, 0xA000A00000000ULL, 0x14001400000000ULL, 0x28002800000000ULL, 0x50005000000000ULL, 0xA000A000000000ULL, 0x40004000000000ULL, 
        0x200020000000000ULL, 0x500050000000000ULL, 0xA000A0000000000ULL, 0x1400140000000000ULL, 0x2800280000000000ULL, 0x5000500000000000ULL, 0xA000A00000000000ULL, 0x4000400000000000ULL, 
        0x2000000000000ULL, 0x5000000000000ULL, 0xA000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xA0000000000000ULL, 0x40000000000000ULL}, {
        0x40000ULL, 0x80000ULL, 0x110000ULL, 0x220000ULL, 0x440000ULL, 0x880000ULL, 0x100000ULL, 0x200000ULL, 
        0x4000000ULL, 0x8000000ULL, 0x11000000ULL, 0x22000000ULL, 0x44000000ULL, 0x88000000ULL, 0x10000000ULL, 0x20000000ULL, 
        0x400000004ULL, 0x800000008ULL, 0x1100000011ULL, 0x2200000022ULL, 0x4400000044ULL, 0x8800000088ULL, 0x1000000010ULL, 0x2000000020ULL, 
        0x40000000400ULL, 0x80000000800ULL, 0x110000001100ULL, 0x220000002200ULL, 0x440000004400ULL, 0x880000008800ULL, 0x100000001000ULL, 0x200000002000ULL, 
        0x4000000040000ULL, 0x8000000080000ULL, 0x11000000110000ULL, 0x22000000220000ULL, 0x44000000440000ULL, 0x88000000880000ULL, 0x10000000100000ULL, 0x20000000200000ULL, 
        0x400000004000000ULL, 0x800000008000000ULL, 0x1100000011000000ULL, 0x2200000022000000ULL, 0x4400000044000000ULL, 0x8800000088000000ULL, 0x1000000010000000ULL, 0x2000000020000000ULL, 
        0x400000000ULL, 0x800000000ULL, 0x1100000000ULL, 0x2200000000ULL, 0x4400000000ULL, 0x8800000000ULL, 0x1000000000ULL, 0x2000000000ULL, 
        0x40000000000ULL, 0x80000000000ULL, 0x110000000000ULL, 0x220000000000ULL, 0x440000000000ULL, 0x880000000000ULL, 0x100000000000ULL, 0x200000000000ULL}};

        static constexpr uint8_t hlt[8][64] =
        {{0xFE, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x3E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x7E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x3E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2}, {
        0x0, 0xFD, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x3D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x7D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x3D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5}, {
        0x0, 0x0, 0xFB, 0xFA, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x3B, 0x3A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x7B, 0x7A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x3B, 0x3A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA}, {
        0x0, 0x0, 0x0, 0x0, 0xF7, 0xF6, 0xF4, 0xF4, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x37, 0x36, 0x34, 0x34, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x77, 0x76, 0x74, 0x74, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x37, 0x36, 0x34, 0x34, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xEF, 0xEE, 0xEC, 0xEC, 0xE8, 0xE8, 0xE8, 0xE8, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x2F, 0x2E, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x6F, 0x6E, 0x6C, 0x6C, 0x68, 0x68, 0x68, 0x68, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x2F, 0x2E, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xDF, 0xDE, 0xDC, 0xDC, 0xD8, 0xD8, 0xD8, 0xD8, 
        0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x5F, 0x5E, 0x5C, 0x5C, 0x58, 0x58, 0x58, 0x58, 
        0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xBF, 0xBE, 0xBC, 0xBC, 0xB8, 0xB8, 0xB8, 0xB8, 
        0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 
        0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 
        0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0}, {
        0x7F, 0x7E, 0x7C, 0x7C, 0x78, 0x78, 0x78, 0x78, 
        0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 
        0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
        0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40}};

        uint64_t pieces[14];
        bool toMove;

        bool isChecked();
        bool kingBare();
        uint64_t hyperbolaQuintessence(int&);
        bool ownKingBare();

        bool getSide();
        void printAllBitboards();

        static std::string moveToAlgebraic(Move&);
        static void printMoveAsBinary(Move);
        static void printAsBitboard(Bitboard);
};

class Position : public Bitboards{
    public:
        uint32_t moves[96][128];
        int mprior[96][128];

        int scores[2], eScores[2];
        int thm, chm[1024];
        uint64_t zhist[1024];
        
        int historyTable[2][6][64];
        int inGamePhase;

        uint64_t nodes;

        static constexpr int totalGamePhase = 64;
        static constexpr int phases[6] = {0, 6, 4, 2, 1, 1};
        static constexpr int matVals[6] = {0, 650, 400, 200, 150, 100};

        //Zobrist Hashing
        static uint64_t zpk[2][6][64];
        static uint64_t ztk;

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

        int countReps();
        int evaluate();
        int evaluateScratch();

        void setStartPos();
        int fullMoveGen(int, bool);
        uint64_t perft(int, int);

        void makeMove(uint32_t, bool, bool);
        void sendMove(std::string);

        void eraseHistoryTable();
        void beginZobristHash();
        
        int halfMoveCount();
        void readFen(std::string);
        std::string makeFen();

        std::string makeRandMoves(int);

        static void initZobristKeys();
};

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

struct EvalVars{
    double* rc;
    double* aw;
};

class Engine : public Position{
    public:
    Move bestMove;

    uint64_t mnodes;

    TTentry* ttable;
    uint32_t killers[64][2];
    int numKillers[64];

    int64_t thinkLimit; //think time limit
    std::chrono::_V2::steady_clock::time_point moment;
    
    void endHandle();

    bool isInteresting(uint32_t&, bool);
    void eraseTransposeTable();
    int alphabeta(int, int, int, int, bool);

        double rfpCoef[2] = {3.0, 67.0};
        double aspWins[5] = {73.5, 120.0, 240.0, 480.0, 40000.0};

        int lmrReduces[64][128]; 
        double lmrCoef[2] = {-0.1, 0.4};

        Engine();

        Move getMove();

        void showZobrist();
        void newGame();
        void copyEval(EvalVars);

        int quiesce(int, int, int);
        int search(uint32_t, int, uint64_t, bool);

        ~Engine();
};

