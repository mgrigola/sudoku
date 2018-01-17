#ifndef SUDOKU_BOARD_H
#define SUDOKU_BOARD_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>

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

class SudokuBoard
{
private:    
    uint_fast16_t solveCount;         //keeps track of how many squares known. Done when ==81
    uint_fast16_t solveStep;          //keeps track of the step in the solution (for use with solve order) [0,80]
    uint_fast16_t sqrIndex[9][9];     //dim1: row, dim2:col   - each element is bit representation of possible values in a given square. If element=pow2, then log2(element) goes in that square
    uint_fast16_t rowIndex[9][9];     //dim1: row, dim2:value - each element is bit representation of possible locations(columns) in a given row for a given value. If element=pow2, then that value goes in that row in column log2(element)
    uint_fast16_t colIndex[9][9];     //dim1: col, dim2:value - each element is bit representation of possible locations(rows) in a given col for a given value. If element=pow2, then that value goes in that row in column log2(element)
    uint_fast16_t boxIndex[9][9];     //dim1: row, dim2:col   - each element is bit representation of possible values in a given square. If element=pow2, then log2(element) goes in that
    //uint_fast16_t boxIndexExcl[9][9]; //not used? I;ve left out some stuff from the original MUMPS code...

    // uint_fast16_t  //so on 32/64 bit architecture. It might be faster to use 64bit integers instead of 16!
    // uint_fast16_t will use whatever this C++ implementation thinks is fastest. Actually, as writing this it's just defined as uint_fast16_t anyway but wouold generally depend on compiler?

    //could be static...
    uint_fast16_t lookUp[10];  //given number [0,9] return its bit representation -> lookUp[index] = 2^index
    std::unordered_map<uint_fast16_t, uint_fast16_t> upLookBitMap;   // upLook[2^index] = index... Up_Look(lookUp[x]) = x | if x is pow2. Else 0.

    //given a value from sqrIndex or whatever, returns the corresponding number from bitwise storage. i.e. 1->1, 16->4, 256->9. Otherwise returns 0
    //in practice, we use this to check if we've eliminated all but one option for a square. If so it's an even power of 2 and in the map. If not, it won't be in the map and 0 is returned (i.e. 0 = "square not known yet!")
    void Populate_Lookup_Tables(void);
    uint_fast16_t Up_Look(uint_fast16_t xBit);
    uint_fast16_t Look_Up(uint_fast16_t x);
    uint_fast16_t Read_Board_Character(char cVal);
    uint_fast16_t Count_Square_Opts(uint_fast16_t xBit);

    bool Is_Solved(void);
    bool Check_Indices(uint_fast16_t val, uint_fast16_t row, uint_fast16_t col);
    bool Check_Square(uint_fast16_t val, uint_fast16_t row, uint_fast16_t col);
    void Check_Box_Exclusions(void);
    void Check_Other_Squares_In_Row(uint_fast16_t val, uint_fast16_t row, uint_fast16_t colStart);
    void Check_Other_Squares_In_Col(uint_fast16_t val, uint_fast16_t rowStart, uint_fast16_t col);
    void Check_Row_Pairs(void);
    void Check_Col_Pairs(void);

    uint_fast16_t Up_Look_Full(uint_fast16_t xBit, std::string& retStr);
    bool Up_Look_Pair(uint_fast16_t xBits, uint_fast16_t& val1, uint_fast16_t& val2);
    bool Up_Look_Triplet(uint_fast16_t xBits, uint_fast16_t& val1, uint_fast16_t& val2, uint_fast16_t& val3);

    bool Guess_And_Check(void);

public:
    enum DISCOVERY_METHOD : uint_fast16_t {
        GIVEN,              //given in puzzle definition
        LAST_IN_SQUARE,     //no other number possible in a square
        LAST_IN_ROW,        //only remaining place to put a number in a row
        LAST_IN_COLUMN,     //only remaining place to put a number in a column
        LAST_IN_BOX,        //only remaining place to put a number in a box
        GUESS_N_CHECK,      //had to guess
        UNKNOWN
    };

    uint_fast16_t inputBoard[9][9];    //stores the initial/known values of each square. 0=blank, 9="9"
    uint_fast16_t solutionBoard[9][9]; //stores the known/final values of each square. 0=blank, 9="9"
    uint_fast16_t methodBoard[9][9];   //
    uint_fast16_t solveOrder[81][2];   //stores row+column of solved squares in the order they were found/solved. 81 squares, so every solution is 81 steps

    int perfStatsIndexCount, perfStatsSquareCount;

    SudokuBoard(void);
    SudokuBoard(const SudokuBoard& other);
    bool Read_Board(const std::string& fileName, uint_fast16_t puzzleNo=0);
    void Clear_Board(void);
    bool Solve_Board(void);
    bool Update_Board(uint_fast16_t val, uint_fast16_t row, uint_fast16_t col, SudokuBoard::DISCOVERY_METHOD discoverMethod = DISCOVERY_METHOD::GIVEN);

    void Print_Board(void);
    bool Print_Remainders(uint_fast16_t target[][9]);

    bool is_Square_Known(uint_fast16_t row, uint_fast16_t col);
    uint_fast16_t Get_Solution_Step_Count(void);

};

#endif
