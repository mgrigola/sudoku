#ifndef SUDOKU_SOLVER_H
#define SUDOKU_SOLVER_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "SudokuBoard.h"

//would probably be swell if there was a class SudokuBoard, that stored the 9x9 grid and some access functions
//

enum DISCOVERY_METHOD : uint16_t {
    GIVEN,              //given in puzzle definition
    LAST_IN_SQUARE,     //no other number possible in a square
    LAST_IN_ROW,        //only remaining place to put a number in a row
    LAST_IN_COLUMN,     //only remaining place to put a number in a column
    LAST_IN_BOX,        //only remaining place to put a number in a box
    GUESS_N_CHECK,      //had to guess
    UNKNOWN
};

// "Board"  = the whole 9x9 grid
// "Box"    = each of the nine 3x3 blocks/chunks that make up the grid [0-8]!
// "Square" = each of the eighty-one cells/squares in the full board [0-8][0-8]
// "Row"    = each of the 9 rows in the puzzle [0-8]
// "Col"    = each of the 9 columns in the puzzle [0-8]
// "Value"  = the character in one of the squares [0-8]
// ?"bit"   = bitwise representation of ?: the position of the set bit represents the number. e.g. 1->1, 4->3, 16->5, etc.
//              Most efficient way to represent some combination of some numbers. Ex: 1 or 3 or 5 = 21. to remove X: val ~& lookUp[X]. The offset has varied over time. maybe 256->8, or maybe 256->7. maybe 1->0, maybe 1->1 (0 might mean 'none of the above' not 1, etc.)

//careful with 0 or 1 first index. Is a "1" represented as [0] or [1]? It varies and I might change it (rows/cols are [0-8], values are [0-8].
//the lookup to decode the bitwise representation stuff needs to return a value for 'not anything/no match'. I'm choosing 0 for that, which would conflict with 0 indexing...

class SudokuSolver
{
private:    
    SudokuBoard solutionBoard, methodBoard;  //###make private + access func?  //method board only explains the immediate method - guess and check ro whatever might reveal like 20 squares, but only the first guess gets method::guess. Everything else probably simple elimination. maybe need a bit thing - like (guess | pair | row elim) i dunno
    bool conflictEncountered;
    uint16_t solveCount;         //keeps track of how many squares known. Done when ==81
    uint16_t solveStep;          //keeps track of the step in the solution (for use with solve order) [0,80]
    uint16_t sqrIndex[9][9];     //dim1: row, dim2:col   - each element is bit representation of possible values in a given square. If element=pow2, then log2(element) goes in that square
    uint16_t rowIndex[9][9];     //dim1: row, dim2:value - each element is bit representation of possible locations(columns) in a given row for a given value. If element=pow2, then that value goes in that row in column log2(element)
    uint16_t colIndex[9][9];     //dim1: col, dim2:value - each element is bit representation of possible locations(rows) in a given col for a given value. If element=pow2, then that value goes in that row in column log2(element)
    uint16_t boxIndex[9][9];     //dim1: row, dim2:col   - each element is bit representation of possible values in a given square. If element=pow2, then log2(element) goes in that

    // uint_fast16_t  //so on 32/64 bit architecture. It might be faster to use 64bit integers instead of 16!
    // uint_fast16_t will use whatever this C++ implementation thinks is fastest. Actually, as writing this it's just defined as uint16_t anyway but wouold generally depend on compiler?

    //could be static...
    uint16_t lookUp[10];  //given number [0,9] return its bit representation -> lookUp[index] = 2^index
    std::unordered_map<uint16_t, uint16_t> upLookBitMap;   // upLook[2^index] = index... Up_Look(lookUp[x]) = x | if x is pow2. Else 0.

    //given a value from sqrIndex or whatever, returns the corresponding number from bitwise storage. i.e. 1->1, 16->4, 256->9. Otherwise returns 0
    //in practice, we use this to check if we've eliminated all but one option for a square. If so it's an even power of 2 and in the map. If not, it won't be in the map and 0 is returned (i.e. 0 = "square not known yet!")
    void Initialize(void);
    void Populate_Lookup_Tables(void);
    uint16_t Up_Look(uint16_t xBit);
    uint16_t Look_Up(uint16_t x);
    uint16_t Count_Square_Opts(uint16_t xBit);
    uint16_t Up_Look_Full(uint16_t xBit, std::string& retStr);
    bool Up_Look_Pair(uint16_t xBits, uint16_t& val1, uint16_t& val2);
    bool Up_Look_Triplet(uint16_t xBits, uint16_t& val1, uint16_t& val2, uint16_t& val3);

    bool Update_Board(uint16_t val, uint16_t row, uint16_t col, DISCOVERY_METHOD discoverMethod = DISCOVERY_METHOD::GIVEN);
    bool Update_Indices(uint16_t val, uint16_t row, uint16_t col);
    bool Update_Square_Exclusions(uint16_t val, uint16_t row, uint16_t col);        //### i think can/should merge with Update Indicies but needs testing
    void Check_Box_Exclusions(void);
    void Check_Other_Squares_In_Row(uint16_t val, uint16_t row, uint16_t colStart);
    void Check_Other_Squares_In_Col(uint16_t val, uint16_t rowStart, uint16_t col);
    void Check_Row_Pairs(void);
    void Check_Col_Pairs(void);
    bool Guess_And_Check(void);

public:
    uint16_t solveOrder[81][2];   //stores row+column of solved squares in the order they were found/solved. 81 squares, so every solution is 81 steps  //### make access function
    int perfStatsIndexCount, perfStatsSquareCount; //kinda debugging. Otherwise should be private + access function ###

    SudokuSolver(void);
    SudokuSolver(const SudokuSolver& other); //maybe private? used for guess and check ###

    SudokuBoard& Solve_Board(SudokuBoard& _inputBoard);
    bool Is_Solved(void);
    bool Print_Remainders(uint16_t target[][9]);
    uint16_t Get_Solution_Step_Count(void);
    uint16_t Get_Solution_Method(uint16_t row, uint16_t col);

};

#endif
