#include "SudokuBoard.h"

SudokuBoard::SudokuBoard()
{
    //probably not necessary, but initialize with all zeros... doesn't matter much i guess
    this->Clear_Board();
}

//copy constructor
SudokuBoard::SudokuBoard(const SudokuBoard& other)
{
    *this = other;
}

//copy operator
SudokuBoard& SudokuBoard::operator=(const SudokuBoard& other)
{
    for (uint16_t row=0; row<BOARD_ROWS; ++row)
    {
        for (uint16_t col=0; col<BOARD_COLS; ++col)
            this->board[row][col] = other.board[row][col];
    }
    return *this;
}

//print board
std::ostream& SudokuBoard::operator<<(std::ostream& xout)
{
    xout << " ----------------------------- " << std::endl;
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        xout << "|";
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            xout << " " << board[rowNo][colNo] << "  ";
            if (colNo%3==2)
                xout << "|";
        }
        if (rowNo%3==2)
            xout << " ----------------------------- " << std::endl;
    }
    return xout;
}

uint16_t& SudokuBoard::operator()(uint16_t row, uint16_t col)
{
    return board[row][col];
}


void SudokuBoard::Clear_Board(void)
{
    for (uint16_t row=0; row<BOARD_ROWS; ++row)
    {
        for (uint16_t col=0; col<BOARD_COLS; ++col)
            board[row][col] = BOARD_UNKNOWN;
    }
}

//maybe this doesn't belong here... SudokuBoard doesn't need to understand the meaning of its contents, nothing else
//but where else would this go? It's more on the solver to understand this maybe, but I like if(board.is_Known(x,y))...
bool SudokuBoard::is_Square_Known(uint16_t row, uint16_t col)
{
    return board[row][col] != BOARD_UNKNOWN;
}


//std::ostream& SudokuBoard::operator<<(std::ostream& xout, const SudokuBoard& obj)
//{
//    for (uint16_t row=0; row<BOARD_ROWS; ++row)
//    {
//        for (uint16_t col=0; col<BOARD_COLS; ++col)
//            xout << board[row][col] << "\t";

//        xout << std::endl; //this is potentially a problem... not sure, my case is simple
//    }
//    return xout;
//}
