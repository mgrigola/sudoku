#include "TableWidgetSudoku.h"

TableWidgetSudoku::TableWidgetSudoku(uint16_t _nRows, uint16_t _nCols, QWidget* _parent) :
    nRows(_nRows),
    nCols(_nCols)
{
    this->setParent(_parent);
    //QTableWidget::QTableWidget(_nRows, _nCols, _parent);              //should be to use this instead of set cols/row and the loop creating all the QTableWidgetItems, but something is wrong...
    this->setColumnCount(nCols);
    this->setRowCount(nRows);
    this->setSelectionMode(QAbstractItemView::NoSelection);             //don't allow selecting/highlighting cells (like ctrl+a). looks super weird
    //this->setSelectionBehvior(QAbstractItemView::SingleSelection);
    this->horizontalHeader()->setDefaultSectionSize(60);
    this->verticalHeader()->setDefaultSectionSize(60);
    this->verticalScrollBar()->setEnabled(false);
    this->horizontalScrollBar()->setEnabled(false);
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);
    this->setShowGrid(false);                                           //we draw our own grid with the borderDrawer
    //this->setStyleSheet("QTableWidget::item { padding: 20px }");

    //store fonts for numbers. Puzzle givens are bold, guesses/solutions not bold. colors variable
    guessFont = this->font();
    guessFont.setPointSize(28);
    givenFont = guessFont;
    givenFont.setBold(true);
    this->setFont(guessFont);

    //this draws the borders of the cells in the table so it looks like a sudoku grid. neat
    borderDrawer = new DrawBorderDelegate(this);
    this->setItemDelegate( borderDrawer );


    for (int rowNo=0; rowNo<nRows; ++rowNo)
    {
        for (int colNo=0; colNo<nCols; ++colNo)
        {
            QTableWidgetItem* pItem =  new QTableWidgetItem("");
            pItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            this->setItem(rowNo, colNo, pItem);
        }
    }
}

//Load a board from file (fileName) and display it.
//puzzleNo: I have single files with many multiple puzzles in them. puzzleNo selects which puzzle in the file to use
//most test puzzles originally from Project Euler file (with 50 sudokus, i use same format)
void TableWidgetSudoku::Read_Board_File(const std::string& fileName, uint16_t puzzleNo)
{
    Clear_Table();
    sReader.Read_Board_File(inputBoard, fileName, puzzleNo);

    uint16_t v;
    for (uint16_t rowNo=0; rowNo<nRows; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<nCols; ++colNo)
        {
            v = inputBoard(rowNo,colNo);
            if ( v != 0)
            {
                QTableWidgetItem* pItem = this->item(rowNo, colNo);
                pItem->setText(QString::number(v));                     //set value to given number
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);  //givens non-editable
                pItem->setFont(givenFont);
            }
        }
    }
    this->repaint();
}

//this
void TableWidgetSudoku::Solve_Board(void)
{
    //actually solve the board (SudokuBoard does that)
    solutionBoard = sSolver.Solve_Board(inputBoard);

    //performance debugging. pretty sure we're still doing some unnecessary work... ###
    std::cout << "Index Count: " << sSolver.perfStatsIndexCount << std::endl;
    std::cout << "Square Count: " << sSolver.perfStatsSquareCount << std::endl;

    //display the solution in the table widget. with colors!
    for (uint16_t rowNo=0; rowNo<nRows; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<nCols; ++colNo)
        {
            QTableWidgetItem *pItem = this->item(rowNo,colNo);
            uint16_t boardVal = pItem->text().toInt();  //or use the QVariant data() thing properly?
            uint16_t solveVal = solutionBoard(rowNo,colNo);

            //correct value already entered. leave it
            if (boardVal == solveVal)
                continue;

            //square does not have correct value. Set to correct value
            pItem->setData(Qt::EditRole, QString::number(solutionBoard(rowNo,colNo)) );
            pItem->setText(pItem->data(Qt::EditRole).toString());
            QColor methodColor = ColorMaps::methodColors[ sSolver.Get_Solution_Method(rowNo,colNo) ];

            //user didn't enter anything. set text color based on how algorithm determined the number
            if (boardVal == 0)
            {
                pItem->setForeground( methodColor );
            }

            //user entered incorrect value. Set background to identify mistake
            else
            {
                pItem->setBackground( methodColor );
            }
        }
    }

    this->repaint();
}

//clear board, reset font, call when loading a new puzzle, erasing existing puzzle
void TableWidgetSudoku::Clear_Table(void)
{
    for (uint16_t rowNo=0; rowNo<nRows; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<nCols; ++colNo)
        {
            QTableWidgetItem *pItem = this->item(rowNo, colNo);
            pItem->setText("");
            pItem->setFont(guessFont);
            pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
            pItem->setForeground(Qt::black);
            pItem->setBackground(Qt::transparent);
            //pItem->setBackgroundColor(Qt::white);
        }
    }
    repaint();
}

//an animation of solving rather than instantaneous display. Work is done in Animate_Step (new thread)
void TableWidgetSudoku::Slow_Solve(void)
{
    Clear_Table();
    uint16_t v;
    pauseSolve = false;

    //step 1: fill in givens
    for (uint16_t rowNo=0; rowNo<nRows; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<nCols; ++colNo)
        {
            v = inputBoard(rowNo,colNo);
            if ( v != 0 )
            {
                QTableWidgetItem *pItem = this->item(rowNo, colNo);
                pItem->setText(QString::number(v));
                pItem->setFont(givenFont);
            }
        }
    }

    //step 2: solve the board (but don't update table widget)
    solutionBoard = sSolver.Solve_Board(inputBoard);

    //step 3: paint each solvd square in the order it was solved
    //Don't want to lock the ui while doing this so create time and send signal to paint next solution square every N msec
    solveStep = 0;
    solveTimer= new QTimer(this);
    connect(solveTimer, SIGNAL(timeout()), this, SLOT(Animate_Step() ) );
    solveTimer->setInterval(300);
    solveTimer->start();
}

void TableWidgetSudoku::Pause_Solve(void)
{
    pauseSolve = !pauseSolve;
}

void TableWidgetSudoku::Animate_Step(void)
{
    if ( pauseSolve )
        return;

    uint16_t row = sSolver.solveOrder[solveStep][0];
    uint16_t col = sSolver.solveOrder[solveStep][1];

    //debugging ###
    std::cout << "Step: " << solveStep << "   {" << row << "," << col << "}" << std::endl;

    QColor methodColor = ColorMaps::methodColors[ sSolver.Get_Solution_Method(row,col) ];
    QTableWidgetItem* pItem = this->item(row,col);
    pItem->setData(Qt::EditRole, QString::number(solutionBoard(row,col)) );
    pItem->setText(pItem->data(Qt::EditRole).toString());
    pItem->setForeground( methodColor );

    repaint();
    ++solveStep;

    //if finished
    if (solveStep == sSolver.Get_Solution_Step_Count() )
    {
       solveTimer->stop();
       emit( Solve_Done() );
       delete solveTimer;
    }
}
