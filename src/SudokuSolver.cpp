#include "SudokuSolver.h"

/*
// Box is each of the 9 3x3 blocks that make up the 9x9 board. Row/Col are the overall row/column on the board in [0,8]
// 0 1 2
// 3 4 5
// 6 7 8
// given row+col, return box # in board

// Square is each of the 9 cells that make up each box. Row/Col are the overall row/column on the board in [0,8]
// 0 1 2
// 3 4 5
// 6 7 8
// given row+col, return square # in box

 ____ ____ ____
|    |    |    |
|    |    |    |
|____|____|____|
|    |    |    |
|    |    |    |
|____|____|____|
|    |    |    |
|    |    |    |
|____|____|____|

*/



SudokuSolver::SudokuSolver(void)
{
    Populate_Lookup_Tables();  //populate lookup tables. lookUp is unnecessary, but the reverse (upLookBitMap) is more efficient to precalculate. Although with only 10 options and map overhead, it's actually probably slower...
    //Initialize();  //i think unecessary/unlogical - going to initialize on Solve anyway///
}

//i think only useful for guess and check, so maybe remove some of this excess crap
SudokuSolver::SudokuSolver(const SudokuSolver& other)
{
    Populate_Lookup_Tables();
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            solutionBoard               = other.solutionBoard;
            methodBoard                 = other.methodBoard;
            boxIndex[rowNo][colNo]      = other.boxIndex[rowNo][colNo];
            sqrIndex[rowNo][colNo]      = other.sqrIndex[rowNo][colNo];
            rowIndex[rowNo][colNo]      = other.rowIndex[rowNo][colNo];
            colIndex[rowNo][colNo]      = other.colIndex[rowNo][colNo];
            solveOrder[9*rowNo+colNo][0]= other.solveOrder[9*rowNo+colNo][0]; //questionable
            solveOrder[9*rowNo+colNo][1]= other.solveOrder[9*rowNo+colNo][1]; //questionable
        }

    solveStep = other.solveStep;
    solveCount = other.solveCount;
    perfStatsSquareCount = other.perfStatsSquareCount;
    perfStatsIndexCount = other.perfStatsIndexCount;
    conflictEncountered = false;
}

void SudokuSolver::Populate_Lookup_Tables(void)
{
    lookUp[0] = 0;
    for (uint16_t bit=0; bit<9; bit++)
    {
        lookUp[bit+1] = 1<<bit;
        upLookBitMap[1<<bit] = bit+1;
    }
}

// 511 = (1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256)
// i.e. every number is possible in that square
void SudokuSolver::Initialize(void)
{
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            solutionBoard.Clear_Board();
            methodBoard.Clear_Board();
            boxIndex[rowNo][colNo]=511;
            sqrIndex[rowNo][colNo]=511;
            rowIndex[rowNo][colNo]=511;
            colIndex[rowNo][colNo]=511;
        }
    }

    solveStep = 0;
    solveCount = 0;
    perfStatsSquareCount = 0;
    perfStatsIndexCount = 0;
    conflictEncountered = false;
}

//this goes through all the steps/logic of solving the board, starting from the input board
SudokuBoard& SudokuSolver::Solve_Board(SudokuBoard& inputBoard)
{
    Initialize();
    uint16_t v;
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            v = inputBoard(rowNo,colNo);
            if ( v != 0 )
                Update_Board(v,rowNo,colNo);
        }
    }

    //check if successfully solved (no 0's left)
    bool wasSolved = Is_Solved();
    int tryCount=0;

    if (!wasSolved)
    {
        std::cout << "After simple elimination:" << std::endl;  //debugging ###
        Print_Remainders(sqrIndex);
    }


    //if haven't solved puzzle via straightforward elimination, try the more expensive, more complicated logic (val forced 1 row/col in box, N vals in N sqrs 'pairs', guess n check)
    while (!wasSolved && ++tryCount<5)
    {
        Check_Box_Exclusions();
        if ( !Is_Solved() )
        {
            std::cout << "After box exclusions (round " << tryCount << ")" << std::endl;    //debugging ###
            Print_Remainders(sqrIndex);                                                     //debugging ###
            Check_Row_Pairs();
            Check_Col_Pairs();
        }
        if ( !Is_Solved() )
        {
            std::cout << "After pairs (round " << tryCount << ")" << std::endl;             //debugging ###
            Print_Remainders(sqrIndex);                                                     //debugging ###
            Guess_And_Check();
        }

        wasSolved = Is_Solved(); //debugging ###
        if (!wasSolved)
        {
            std::cout << "After guess and check (round " << tryCount << ")" << std::endl;
            Print_Remainders(sqrIndex);
        }
    }

    //debugging ### this whole block is unneeded for solving
    if (!wasSolved)
    {
        std::cout << "FAILED TO SOLVE! Here's what we got:" << std::endl;
        std::cout << std::endl << std::endl << "Solution Board - [row v][col >] = vals";
        Print_Remainders(sqrIndex);

        std::cout << std::endl << std::endl << "Row Index - [row v][val >] = cols";
        Print_Remainders(rowIndex);

        std::cout << std::endl << std::endl << "Col Index - [col v][val >] = rows";
        Print_Remainders(colIndex);

        std::cout << std::endl << std::endl << "Box Index - [box v][val >] = sqrs";
        Print_Remainders(boxIndex);
    }

    return solutionBoard;
}

//called every time we gain knowledge of another square. Updates our indicies to eliminate possibilities based on knowledge of new number
//it's recursive-ish, so if eliminating options determines another square, we'll call Update_Board again with the new info
// val = known number to enter into the puzzle [1,9]
// row/column = position ot place the number [0,8]
// discoverMethod = keeps track of how we determined the number we're setting. When inputting givens leave blank or pass given. Other values used internally
//return value doesn't work at the moment ### top level will always be true, but there may be conflicts during the recursion that get ignored. Check conflictEncountered for now to see if any ssignment conflicted.
bool SudokuSolver::Update_Board(uint16_t val, uint16_t row, uint16_t col, DISCOVERY_METHOD discoverMethod)
{
    //debugging ###
    std::cout << "[" << row << "," << col << "] = " << val+1 << "  ( " << discoverMethod << " )" << std::endl;

    if (discoverMethod == DISCOVERY_METHOD::GIVEN)
        val -= 1;

    //don't think this is a problem but ###debug just incase
    if (val > 9)  //actually checking <0, but would overflow to 2^16-1
    {
        std::cout << "ERROR?: Assigning 0?" << std::endl;
        return false;
    }

    //this shouldn't be happening but it is happening... ### debugging until resolved. Some errors probably superficial from the val-1 offset, but it's not all
    if ( solutionBoard(row,col) != 0 )
    {
        if ( solutionBoard(row,col) != (val+1) )
        {
            std::cout << "ERROR?: Already have [" << solutionBoard(row,col) << "]  -  now receive (" << (val+1) << ") ???" << std::endl;
            conflictEncountered = true;
        }
        std::cout << "ERROR?: Duplicate Update" << std::endl;
        return false;   //already got this square, return
    }

    ++solveCount;
    solutionBoard(row,col) = val+1;              //enter the value into the solution board.
    methodBoard(row,col) += discoverMethod;     // keep track of how we figured out

    //clean up indicies. I think only for debugging and printing remainders
    sqrIndex[row][col] = Look_Up(val);
    rowIndex[row][val] = Look_Up(col);
    colIndex[col][val] = Look_Up(row);
    boxIndex[3*(row/3) + col/3][val] = Look_Up(3*(row%3) + col%3);

    //keep track of the order we solved everything. We start counting after all the givens
    //because we enter givens 1by1 and may make conclusions before entering all the givens, we don't count the givens (givens should be available via other sources).
    if (discoverMethod != DISCOVERY_METHOD::GIVEN)
    {
        solveOrder[solveStep][0] = row;
        solveOrder[solveStep][1] = col;
        ++solveStep;
    }

    //we just found the value for this square. we know no other value can go in this square
    //we also know the value can't go anywhere else in this row, column, or box
    //in each case we can skip the exact square+value because we already solved it

    //remove other values from found square.
    //We know the square isn't one of these other value, so skip Update_Square_Exclusions? Not necesarily more efficient...
    for (uint16_t tmpVal=0; tmpVal<9; ++tmpVal)
        if ( tmpVal != val )
            Update_Indices(tmpVal, row, col);

    //remove found value from other rows in same column
    for (uint16_t tmpRow=0; tmpRow<9; ++tmpRow)
        if ( tmpRow != row )
            Update_Square_Exclusions(val, tmpRow, col);

    //remove found value from other columns in same row
    for (uint16_t tmpCol=0; tmpCol<9; ++tmpCol)
        if ( tmpCol != col )
            Update_Square_Exclusions(val, row, tmpCol);

    //remove found value from other squares in same box. Note: we already hit therow and column, so we can skip tmpRow==row and tmpCol==col (equivalent to modulo magic here - so efficient!)
    uint16_t rowStart = row-row%3;  //first row in box
    uint16_t colStart = col-col%3;  //first col in box
    for (uint16_t rInc=1; rInc<3; ++rInc)
        for (uint16_t cInc=1; cInc<3; ++cInc)
            Update_Square_Exclusions( val, rowStart + (row+rInc)%3, colStart + (col+cInc)%3 );  //just loop over the rows/cols in box not equal to row and col themselves. math checks out, promise.

    return true;
}

//calling this function indicates val cannot be at this square [row][col]
bool SudokuSolver::Update_Square_Exclusions(uint16_t val, uint16_t row, uint16_t col)
{
    ++perfStatsSquareCount;
    uint16_t remainder;

    //(1) if val was previously possible in square, we remove it and chceck if there's now only 1 option left for the square (if so, then we've determined the square).
    uint16_t valBit = Look_Up(val);        //val = [1-9] so don't +1 (
    if ( sqrIndex[row][col] & valBit )
    {
        sqrIndex[row][col] -= valBit;                                              //remove val from possibilities at square
        remainder = Up_Look(sqrIndex[row][col]);                                   //check if only one option left (sqrIndex would be even pow2)
        if ( remainder != 0 )                                                       //if remianingVal is not pow2, Up_Look returns 0
            Update_Board(remainder-1, row, col, DISCOVERY_METHOD::LAST_IN_SQUARE);    //there was only 1 option left for square. We can fill it in.

         Update_Indices(val,row,col);
         return true;
    }
    return false;
}

//given a value in a square. That value can't be anywhere else in the row, column, or box.
bool SudokuSolver::Update_Indices(uint16_t val, uint16_t row, uint16_t col)
{
    ++perfStatsIndexCount;
    uint16_t remainder;
    bool madeChange=false;

    //(2) for each row, we look for values that can only be at one column/square in the row
    //we use the bitwise representation of columns, so 511 means a value can be at any column in a row.
    //As we eliminate possible locations for a number, we know there is exactly 1 location left when rowIndex element == power of 2 (exactly matches a key in upLookBitMap / Up_Look() )
    uint16_t colBit = Look_Up(col);      //col = [0-8] so add +1
    if ( rowIndex[row][val] & colBit )
    {
        rowIndex[row][val] -= colBit;
        remainder = Up_Look(rowIndex[row][val]);           //remaining column in row
        if ( remainder != 0 )
            Update_Board(val, row, remainder-1, DISCOVERY_METHOD::LAST_IN_ROW);

        madeChange = true;
    }

    //(3) same as 3 but with row/col roles reversed.
    uint16_t rowBit = Look_Up(row);
    if ( colIndex[col][val] & rowBit )
    {
        colIndex[col][val] -= rowBit;
        remainder = Up_Look(colIndex[col][val]);           //remaining row in column
        if ( remainder != 0 )
            Update_Board(val, remainder-1, col, DISCOVERY_METHOD::LAST_IN_COLUMN);

        madeChange = true;
    }

    //(4) same as 2/3 but looking at squares in the box.
    //doesn't actually help solving at all?
    uint16_t box = 3*(row/3) + col/3;                  //box is which 3x3 block [0-8]. upper left is 0, lower right is 8
    uint16_t sqrBit = Look_Up( 3*(row%3) + col%3 ); //sqr is which square within block [0-8]. upper left is 0, lower right is 8. sqr = [0-8] so add +1
    if ( boxIndex[box][val] & sqrBit )
    {
        boxIndex[box][val] -= sqrBit;
        remainder = Up_Look(boxIndex[box][val]);           //remaining square in box
        if ( remainder != 0 )
            Update_Board(val, (3*(box/3) + (remainder-1)/3), (3*(box%3) + (remainder-1)%3), DISCOVERY_METHOD::LAST_IN_BOX);

        madeChange = true;
    }

    return madeChange;
}

//we don't know where a number goes in a box, but we can at least determine it's in single row/column.
//thus we can eliminate the value from said row/column outside of the box we're considering
void SudokuSolver::Check_Box_Exclusions(void)
{
    uint16_t bitVal;
    for (uint16_t boxNo=0; boxNo<9; ++boxNo)
    {
        for (uint16_t valNo=0; valNo<9; ++valNo)
        {
            bitVal = boxIndex[boxNo][valNo];
            if (Up_Look(bitVal) != 0)  //if already solved value in this box, Up_Look will return the square #. Don't need to investigate further
                continue;

            //now we check for bit combinations that are a single srow/column
            //there are only 6 options: 123,456,789,147,258,369. we can do bitwise and to check if bitVal lies entirely within one of these combos
            //i think slightly more efficient if we do an unordered map (hash table), but could also check all 6
            if ( (bitVal & 7) == bitVal)        // 123 = 7      //top row
                Check_Other_Squares_In_Row( valNo, (boxNo/3)*3+0, (boxNo%3)*3 );

            else if ( (bitVal & 56) == bitVal)  // 456 = 56     //middle row
                Check_Other_Squares_In_Row( valNo, (boxNo/3)*3+1, (boxNo%3)*3 );

            else if ( (bitVal & 448) == bitVal) // 789 = 448    //bottom row
                Check_Other_Squares_In_Row( valNo, (boxNo/3)*3+2, (boxNo%3)*3 );

            else if ( (bitVal & 73) == bitVal)  // 147 = 73     //left column
                Check_Other_Squares_In_Col( valNo, (boxNo/3)*3 , (boxNo%3)*3+0);

            else if ( (bitVal & 146) == bitVal) // 258 = 146    //middle column
                Check_Other_Squares_In_Col( valNo, (boxNo/3)*3 , (boxNo%3)*3+1);

            else if ( (bitVal & 292) == bitVal) // 369 = 292    //right column
                Check_Other_Squares_In_Col( valNo, (boxNo/3)*3 , (boxNo%3)*3+2);
        }
    }
}

void SudokuSolver::Check_Other_Squares_In_Row(uint16_t val, uint16_t row, uint16_t colStart)
{
    for(uint16_t colInc=3; colInc<9; ++colInc)
    {
        uint16_t col = (colStart + colInc)%9;
        Update_Square_Exclusions(val,row,col);
    }
}

void SudokuSolver::Check_Other_Squares_In_Col(uint16_t val, uint16_t rowStart, uint16_t col)
{
    for(uint16_t rowInc=3; rowInc<9; ++rowInc)
    {
        uint16_t row = (rowStart + rowInc)%9;
        Update_Square_Exclusions(val,row,col);
    }
}


//check rows for a pair of 2 squares that both have the same (exactly) 2 possible values. If so those 2 values can't go anywhere else in the row.
//there's no good index to make this efficient, so we have to step square by square and compare each square to every other square in the row
//we can save some work by only considering squares with exactly 2 possible values - and so this function becomes a complex mess
void SudokuSolver::Check_Row_Pairs(void)
{
    uint16_t candidateCount, bitVal;
    uint16_t candidateAry[9];
    uint16_t candidateBitAry[9];

    //check every row
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        candidateCount=0;
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            if (solutionBoard(rowNo,colNo) != 0)   //already solved square, skip
                continue;

            bitVal = sqrIndex[rowNo][colNo];
            if (Count_Square_Opts(bitVal) != 2)     //not 2 options, skip
                continue;

            //compare to other options we've already covered in the row. Look for same bitVal.
            bool foundMatch = false;
            for (uint16_t candidateNo=0; candidateNo<candidateCount; ++candidateNo)
            {
                uint16_t candidateBitVal = candidateBitAry[candidateNo];;
                if ( bitVal == candidateBitVal )  //same 2 options in 2 squares in this row - success! - means these 2 options can't go elsewhere in the row
                {
                    uint16_t candidateCol = candidateAry[candidateNo];
                    uint16_t v1,v2;
                    Up_Look_Pair(bitVal, v1, v2);

                    //###We also technically need to have a Check_Box_Pairs that looks for pairs in different rows/cols but same box, same idea as ###above
                    if (colNo/3 == candidateCol/3)
                    {
                        std::cout << "pair: rows: same box. Need to implement code here!" << std::endl; //###needs work. if implement box check this can be deleted i think (can be deleted regardless i guess)
                        //same box
                    }

                    //now remove the 2 values elsewhere in row
                    //###We should also check if [candidateCol and colNo] are in same box and address box as well!
                    for (uint16_t checkCol=0; checkCol<9; ++checkCol)
                    {
                        if(checkCol==candidateCol || checkCol==colNo)
                            continue;

                        Update_Square_Exclusions(v1,rowNo,checkCol);
                        Update_Square_Exclusions(v2,rowNo,checkCol);
                    }
                    foundMatch = true;
                    break;  // break out of candidateNo loop
                }
            }

            //because finding 1 pair will alter the rest of the row. It seems not valid to continue editing row.
            //if anything, we probably need to start over this row with the updated sqrIndex data, but probably do easier work first
            if (foundMatch)  break;  //break out of column loop (go to next row)

            candidateBitAry[candidateCount] = bitVal;
            candidateAry[candidateCount] = colNo;
            ++candidateCount;
        }
    }
}

//pretty much same as Check_Row_Pairs, but flip row and column, plus all the related paperwork. Maybe can combine more... Need a Check_Box_Pairs, too
void SudokuSolver::Check_Col_Pairs(void)
{
    uint16_t candidateCount, bitVal;
    uint16_t candidateAry[9];
    uint16_t candidateBitAry[9];

    //check every row
    for (uint16_t colNo=0; colNo<9; ++colNo)
    {
        candidateCount=0;
        for (uint16_t rowNo=0; rowNo<9; ++rowNo)
        {
            if (solutionBoard(rowNo,colNo) != 0)   //already solved square, skip
                continue;

            bitVal = sqrIndex[rowNo][colNo];
            if (Count_Square_Opts(bitVal) != 2)     //not 2 options, skip
                continue;

            //compare to other options we've already covered in the row. Look for same bitVal.
            bool foundMatch = false;
            for (uint16_t candidateNo=0; candidateNo<candidateCount; ++candidateNo)
            {
                uint16_t candidateBitVal = candidateBitAry[candidateNo];;
                if ( bitVal == candidateBitVal )  //same 2 options in 2 squares in this row - success! - means these 2 options can't go elsewhere in the row
                {
                    uint16_t candidateRow = candidateAry[candidateNo];
                    uint16_t v1,v2;
                    Up_Look_Pair(bitVal, v1, v2);

                    //###We also technically need to have a Check_Box_Pairs that looks for pairs in different rows/cols but same box, same idea as ###above
                    if (rowNo/3 == candidateRow/3)
                    {
                        std::cout << "pair: columns: same box. Need to implement code here!" << std::endl;
                        //same box
                    }

                    //now remove the 2 values elsewhere in row
                    //###We should also check if [candidateCol and colNo] are in same box and address box as well!
                    for (uint16_t checkRow=0; checkRow<9; ++checkRow)
                    {
                        if(checkRow==candidateRow || checkRow==rowNo)
                            continue;

                        Update_Square_Exclusions(v1,checkRow,colNo);
                        Update_Square_Exclusions(v2,checkRow,colNo);
                    }
                    foundMatch = true;
                    break;  // break out of candidateNo loop
                }
            }

            //because finding 1 pair will alter the rest of the row. It seems not valid to continue editing row.
            //if anything, we probably need to start over this row with the updated sqrIndex data, but probably do easier work first
            if (foundMatch)  break;  //break out of column loop (go to next row)

            candidateBitAry[candidateCount] = bitVal;
            candidateAry[candidateCount] = rowNo;
            ++candidateCount;
        }
    }
}

//create a copy of current board. make a guess at a resonable square. try to solve it.
//If it works we can plug in solution to current board. If not we can plug in the other
bool SudokuSolver::Guess_And_Check(void)
{
    uint16_t rowNo=0, colNo=0, guessToTry, otherOptIfGuessWrong;
    //find a square with 2 options, so we can guess the val and we know it's the otehr if it doesnt work out.
    while (true)
    {
        if (solutionBoard.is_Square_Known(rowNo,colNo))
            if (this->Up_Look_Pair(sqrIndex[rowNo][colNo], guessToTry, otherOptIfGuessWrong))
                break;

        if (++colNo == 9)  { ++rowNo; colNo=0; }

        if (rowNo==9)
            return false;  //failure...? No square with 2 options? something has gone horribly wrong
    }

    //guess 1st value on duplicateboard. If good (board is solved with no conflicts), we copy over.
    //if no conflicts found but board isn;t solved, we don't actually know if our guess correct or not, so can't do anything...
    //if we got a conflict we know out guess was wrong and therefore the option we didn't guess must be correct, so plug that in
    SudokuSolver copySolver = SudokuSolver(*this);  //copy the board. We guess. if we're wrong we have to revert to current state
    copySolver.Update_Board(guessToTry, rowNo, colNo); //this returns false if Update value conflicts with solution board

    if ( !copySolver.conflictEncountered && copySolver.Is_Solved() )
    {
        for (uint16_t row=0; row<9; ++row)
        {
            for (uint16_t col=0; col<9; ++col)
            {
                if ( !this->solutionBoard.is_Square_Known(row,col) )
                    this->Update_Board(copySolver.solutionBoard(row,col), row, col, DISCOVERY_METHOD::GUESS_N_CHECK);
            }
        }
    }
    else if (copySolver.conflictEncountered)
    {
        this->Update_Board(otherOptIfGuessWrong, rowNo, colNo, DISCOVERY_METHOD::GUESS_N_CHECK);
    }
    else
    {
        SudokuSolver copySolver2 = SudokuSolver(*this);
        copySolver.Update_Board(otherOptIfGuessWrong, rowNo, colNo);
        if ( !copySolver2.conflictEncountered && copySolver2.Is_Solved() )
        {
            for (uint16_t row=0; row<9; ++row)
            {
                for (uint16_t col=0; col<9; ++col)
                {
                    if ( !this->solutionBoard.is_Square_Known(row,col) )
                        this->Update_Board(copySolver2.solutionBoard(row,col), row, col, DISCOVERY_METHOD::GUESS_N_CHECK);
                }
            }
        }
        //else do nothing

        //we didn't gain any information :( maybe mark this square as unproductive so next iteration we guess on another square
        //Or try guessing the other number now?
        //currently doing nothing... we can try applying the other logic and hope for a miracle that that helps us
    }

    return true;
}







bool SudokuSolver::Is_Solved(void)
{
//    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
//        for (uint16_t colNo=0; colNo<9; ++colNo)
//            if (solutionBoard[rowNo][colNo] == 0)
//                return false;

//    return true;
    return (solveCount == 81);
}

uint16_t SudokuSolver::Get_Solution_Step_Count(void)
{
    return solveStep;
}

//SudokuSolver::DISCOVERY_METHOD - but could be bitwise-or at some point
uint16_t SudokuSolver::Get_Solution_Method(uint16_t row, uint16_t col)
{
    return methodBoard(row, col);   //need cast?
}

bool SudokuSolver::Print_Remainders(uint16_t target[][9])
{
    bool hasRemainder = false;
    std::string printStr;
    std::cout << " -----------------------------------------------------------------" << std::endl;
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        std::cout << "|";
        for (uint16_t colNo=0; colNo<9; ++colNo)
        {
            uint16_t cnt = Up_Look_Full(target[rowNo][colNo], printStr);
            if (cnt == 3)
                std::cout << " " << printStr << " ";
            else if (cnt == 2)
                std::cout << "  " << printStr << "  ";
            else if (cnt <= 1)
                std::cout << "   " << printStr << "   ";
            else
                std::cout << "   " << "." << "   ";

            if (colNo%3==2)
                std::cout << "|";

            if (cnt>0)
                hasRemainder = true;
        }
        std::cout << std::endl;
        if (rowNo%3==2)
            std::cout << " -----------------------------------------------------------------" << std::endl;
    }
    return hasRemainder;
}

//returns count of possible values based on the xBit bit pattern (also how many 1 bits in xBits)
uint16_t SudokuSolver::Count_Square_Opts(uint16_t xBit)
{
    return ((xBit&256)>0) + ((xBit&128)>0) + ((xBit&64)>0) + ((xBit&32)>0) + ((xBit&16)>0) + ((xBit&8)>0)  + ((xBit&4)>0) + ((xBit&2)>0) + ((xBit&1)>0);
}

// ?:0 | 1:1  | 2:2  | 4:3  | 8:4 | 16:5 | 32:6 | 64:7 | 128:8 | 256:9
uint_fast16_t SudokuSolver::Up_Look(uint_fast16_t xBit)
{
    if ( upLookBitMap.count(xBit) )
        return upLookBitMap[xBit];

    return 0;
}

uint_fast16_t SudokuSolver::Look_Up(uint_fast16_t x)
{
    return lookUp[x+1];
}

//assuming 2 bits set in xBits. find both of them. Return in val1/val2. 0-indexed
bool SudokuSolver::Up_Look_Pair(uint16_t xBits, uint16_t& val1, uint16_t& val2)
{
    bool foundFirst = false;
    uint16_t valBit;
    for (uint16_t bit=0; bit<9; ++bit)
    {
        valBit = Look_Up(bit);
        if (xBits & valBit )
        {
            if (foundFirst)
            {
                val2 = bit;
                return ((xBits-valBit) == 0);
            }
            else
            {
                val1 = bit;
                xBits -= valBit;
                foundFirst = true;
            }
        }
    }
    return false;
}

//assuming 3 bits set in xBits. find all of them. Return in val1/val2/val3. 0-indexed
bool SudokuSolver::Up_Look_Triplet(uint16_t xBits, uint16_t& val1, uint16_t& val2, uint16_t& val3)
{
    bool foundFirst = false, foundSecond = false;
    for (uint16_t bit=0; bit<16; ++bit)
    {
        if (xBits & Look_Up(bit) )
        {
            if (foundSecond)
            {
                val3 = bit;
                return true;
            }
            else if (foundFirst)
            {
                val2 = bit;
                foundSecond=true;
            }
            else
            {
                val1 = bit;
                foundFirst=true;
            }
        }
    }
    return false;
}

//translates the bit pattern in val into string representation of which bits are set or which numbers are possible in a square
// e.x. 7=1+2+4 = "1,2,3"
uint16_t SudokuSolver::Up_Look_Full(uint16_t val, std::string& retStr)
{
    retStr = "";
    uint16_t optCount=0;

    //no remaining options?
    if (val == 0)
        return 0;

    for (uint16_t dig=0; dig<9; ++dig)
    {
        if (Look_Up(dig) & val)  //dig is an option
        {
            ++optCount;
            retStr = (retStr == "") ? std::to_string(dig+1) : retStr + "," + std::to_string(dig+1);
        }
    }

    return optCount;
}
