#ifndef SUDOKUBOARD_H
#define SUDOKUBOARD_H

#include <ostream>

//probably should have constructor accept these values and reserve memory dynamically, but enh, not today
#define BOARD_ROWS 9
#define BOARD_COLS 9
#define BOARD_UNKNOWN 0

class SudokuBoard
{
private:
    uint16_t board[BOARD_ROWS][BOARD_COLS];

public:
    SudokuBoard();
    SudokuBoard(const SudokuBoard& other);
    SudokuBoard& operator=(const SudokuBoard& other);
    std::ostream& operator<<(std::ostream& xout);  // std::cout << myBoard;  //prints the board
    uint16_t& operator()(uint16_t row, uint16_t col);
    void Clear_Board(void);
    bool is_Square_Known(uint16_t row, uint16_t col);
};

#endif // SUDOKUBOARD_H
