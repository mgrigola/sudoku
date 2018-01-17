#include "SudokuTable.h"

SudokuTable::SudokuTable(int nRows, int nCols, QWidget* _parent) :
    //QTableWidget(nRows,nCols,_parent),
    sudokuRows(nRows),
    sudokuCols(nCols)
{
    this->setParent(_parent);
    //QTableWidget::QTableWidget(nRows, nCols, _parent);
    this->setColumnCount(nCols);
    this->setRowCount(nRows);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    //this->setSelectionBehvior(QAbstractItemView::SingleSelection);



    //sTable = new QTableWidget(nRows , nCols, this);
    //setCentralWidget(sTable);


    this->horizontalHeader()->setDefaultSectionSize(60);
    this->verticalHeader()->setDefaultSectionSize(60);
    this->verticalScrollBar()->setEnabled(false);
    this->horizontalScrollBar()->setEnabled(false);
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);
    this->setShowGrid(false);
    //this->setStyleSheet("QTableWidget::item { padding: 20px }");

    guessFont = this->font();
    guessFont.setPointSize(28);
    givenFont = guessFont;
    givenFont.setBold(true);
    this->setFont(guessFont);

    //no idea what the deal with this is. pretty sure I didn't understand before either...
    borderDrawer = new DrawBorderDelegate(this);
    this->setItemDelegate( borderDrawer );

    for (int rowNo=0; rowNo<sudokuRows; ++rowNo)
    {
        for (int colNo=0; colNo<sudokuCols; ++colNo)
        {
            QTableWidgetItem* pItem =  new QTableWidgetItem("");
            pItem->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            this->setItem(rowNo, colNo, pItem);
        }
    }
}

void SudokuTable::Read_Board(const std::string& fileName, int puzzleNo)
{
    sBoard.Read_Board(fileName, puzzleNo);
    Clear_Board();
    unsigned short v;
    for (unsigned short rowNo=0; rowNo<sudokuRows; ++rowNo)
    {
        for (unsigned short colNo=0; colNo<sudokuCols; ++colNo)
        {
            v = sBoard.inputBoard[rowNo][colNo];
            if ( v != 0)
            {
                QTableWidgetItem* pItem = this->item(rowNo, colNo);
                pItem->setText(QString::number(v));      //set value to given number
                pItem->setFlags(pItem->flags() & ~Qt::ItemIsEditable);           //givens non-editable
                pItem->setFont(givenFont);
            }
        }
    }

    this->repaint();
}


void SudokuTable::Solve_Board()
{
    sBoard.Solve_Board();

    std::cout << "Index Count: " << sBoard.perfStatsIndexCount << std::endl;
    std::cout << "Square Count: " << sBoard.perfStatsSquareCount << std::endl;

    for (unsigned short rowNo=0; rowNo<sudokuRows; ++rowNo)
    {
        for (unsigned short colNo=0; colNo<sudokuCols; ++colNo)
        {
            QTableWidgetItem *pItem = this->item(rowNo,colNo);
            unsigned short boardVal = pItem->text().toInt();  //or use the QVariant data() thing properly?
            unsigned short solveVal = sBoard.solutionBoard[rowNo][colNo];

            //correct value already entered. leave it
            if (boardVal == solveVal)
                continue;

            //square does not have correct value. Set to correct value
            pItem->setData(Qt::EditRole, QString::number(sBoard.solutionBoard[rowNo][colNo]) );
            pItem->setText(pItem->data(Qt::EditRole).toString());

            //user didn't enter anything. set text color based on how algorithm determined the number
            if (boardVal == 0)
                pItem->setForeground( ColorMaps::methodColors[sBoard.methodBoard[rowNo][colNo]] );
                //pItem->setForeground( MethodColorMap(sBoard.methodBoard[rowNo][colNo]) );
                //pItem->setForeground(methodColors[sBoard.methodBoard[rowNo][colNo]]);

            //user entered incorrect value. Set background to identify mistake
            else
                pItem->setBackground( ColorMaps::methodColors[sBoard.methodBoard[rowNo][colNo]] );
                //pItem->setBackground( MethodColorMap(sBoard.methodBoard[rowNo][colNo]) );
                //pItem->setBackground(methodColors[sBoard.methodBoard[rowNo][colNo]]);
        }
    }

    this->repaint();
}


void SudokuTable::Clear_Board(void)
{
//    for (unsigned short rowNo=0; rowNo<sudokuRows; ++rowNo)
//        for (unsigned short colNo=0; colNo<sudokuCols; ++colNo)
//        {
//            QTableWidgetItem *pItem = this->item(rowNo, colNo);
//            pItem->setBackgroundColor(Qt::white);
//        }

    repaint();

    for (unsigned short rowNo=0; rowNo<sudokuRows; ++rowNo)
    {
        for (unsigned short colNo=0; colNo<sudokuCols; ++colNo)
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

//this apparently refers to slow as in like a movie of solving instead of instantaneous. Not computationally slow (it's same algorithm).

void SudokuTable::Slow_Solve(void)
{
    Clear_Board();
    unsigned short v;
    pauseSolve = false;

    //step 1: solve the board (but don't update table widget). Also fill in givens?
    for (unsigned short rowNo=0; rowNo<sudokuRows; ++rowNo)
    {
        for (unsigned short colNo=0; colNo<sudokuCols; ++colNo)
        {
            v = sBoard.inputBoard[rowNo][colNo];
            if ( v != 0 )
            {
                QTableWidgetItem *pItem = this->item(rowNo, colNo);
                pItem->setText(QString::number(v));
                pItem->setFont(givenFont);
            }
        }
    }

    sBoard.Solve_Board();

    solveStep = 0;

    solveTimer= new QTimer(this);
    connect(solveTimer, SIGNAL(timeout()), this, SLOT(Animate_Step() ) );
    solveTimer->setInterval(300);
    solveTimer->start();

//    //step 3: fill in table widget with colord numbers
//    unsigned short stepCount = sBoard.Get_Solution_Step_Count();
//    for (unsigned short step=0; step<stepCount; ++step)
//    {
//        if (pauseSolve)
//            QThread::msleep(200);

//        unsigned short row = sBoard.solveOrder[step][0];
//        unsigned short col = sBoard.solveOrder[step][1];

//        std::cout << "Step: " << step << "   {" << row << "," << col << "}" << std::endl;

//        QTableWidgetItem* pItem = this->item(row,col);
//        pItem->setData(Qt::EditRole, QString::number(sBoard.solutionBoard[row][col]));
//        pItem->setText(pItem->data(Qt::EditRole).toString());
//        pItem->setForeground( ColorMaps::methodColors[sBoard.methodBoard[row][col]] );
//        //pItem->setForeground( MethodColorMap(sBoard.methodBoard[row][col]) );
//        //pItem->setForeground(methodColors[sBoard.methodBoard[row][col]]);
//        //QThread::msleep(200);
//        std::this_thread::sleep_for(std::chrono::milliseconds(200));
//        repaint();
//        //update();
//    }
}

void SudokuTable::Pause_Solve(void)
{
    pauseSolve = !pauseSolve;
}


void SudokuTable::Animate_Step(void)
{
    if ( pauseSolve )
        return;

    unsigned short row = sBoard.solveOrder[solveStep][0];
    unsigned short col = sBoard.solveOrder[solveStep][1];
    std::cout << "Step: " << solveStep << "   {" << row << "," << col << "}" << std::endl;

    QTableWidgetItem* pItem = this->item(row,col);
    pItem->setData(Qt::EditRole, QString::number(sBoard.solutionBoard[row][col]));
    pItem->setText(pItem->data(Qt::EditRole).toString());
    pItem->setForeground( ColorMaps::methodColors[sBoard.methodBoard[row][col]] );
    //pItem->setForeground( MethodColorMap(sBoard.methodBoard[row][col]) );
    //pItem->setForeground(methodColors[sBoard.methodBoard[row][col]]);

    repaint();
    ++solveStep;

    if (solveStep == sBoard.Get_Solution_Step_Count() )
    {
       solveTimer->stop();
       emit( Solve_Done() );
       delete solveTimer;
    }
}
