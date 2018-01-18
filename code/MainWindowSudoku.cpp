#include "MainWindowSudoku.h"

MainWindowSudoku::MainWindowSudoku(QWidget* _parent)
{    
    this->setParent(_parent);
    this->setWindowTitle("Sudoku");
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); //this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMinimumSize(100, 100);

    labelFileLabel = new QLabel("File:");
    pushButtonLoad = new QPushButton("Load", this);

    comboBoxFileSelect = new QComboBox(this);
    comboBoxFileSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    comboBoxFileSelect->setEditable(false);

    sourceDir = QDir("C:/Users/m/Desktop/sudoku_test");
    QStringList puzzleFiles = sourceDir.entryList(QStringList("*.sudoku"));
    for (const QString& file : puzzleFiles)
        comboBoxFileSelect->addItem(file);

    labelPuzzleNo = new QLabel("Puzzle#:",this);
    labelPuzzleNo->setDisabled(true);
    labelPuzzleNo->show();

    spinBoxPuzzleNo = new QSpinBox(this);
    spinBoxPuzzleNo->setDisabled(true);
    spinBoxPuzzleNo->setMinimum(1);
    spinBoxPuzzleNo->setValue(1);
    spinBoxPuzzleNo->show();

    sudokuTable = new TableWidgetSudoku(9, 9, this);

    pushButtonSolve = new QPushButton("Solve", this);
    pushButtonSlowSolve = new QPushButton("Slow Solve", this);
    pushButtonPauseSolve = new QPushButton("pause", this);
    pushButtonPauseSolve->hide();
    checkboxShowLegend = new QCheckBox("Show Legend", this);
    //checkboxShowLegend->show();


    //still can't figure out how to make the window stick to the
    this->setMinimumWidth(550);
    this->setMinimumHeight(608);

    gLayoutMain = new QGridLayout();
    //gLayoutMain->setSizeConstraint(QLayout::SetMinimumSize);
    //gLayoutMain->setSizeConstraint(QLayout::SetNoConstraint);
    gLayoutMain->addWidget(labelFileLabel, 0, 0, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(comboBoxFileSelect, 0, 1, 1, 4, Qt::AlignLeft);
    gLayoutMain->addWidget(labelPuzzleNo, 0, 5, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(spinBoxPuzzleNo, 0, 6, 1, 1, Qt::AlignLeft);
    gLayoutMain->addWidget(pushButtonLoad, 0, 7, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(sudokuTable, 1, 0, 5, 8);
    gLayoutMain->addWidget(pushButtonSolve, 6, 3, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(pushButtonSlowSolve, 6, 4, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(pushButtonPauseSolve, 6, 5, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(checkboxShowLegend, 6, 6, 1, 1, Qt::AlignHCenter);

    gLayoutMain->setContentsMargins(4,4,4,4);

    setLayout(gLayoutMain);

    connect( pushButtonLoad, SIGNAL(clicked(bool)), this, SLOT(Read_Sudoku_Board()) );  //need to pass string but too dumb to do properly :(
    connect( pushButtonSolve, SIGNAL(clicked(bool)), sudokuTable, SLOT(Solve_Board()) );
    connect( comboBoxFileSelect, SIGNAL(activated(QString)), this, SLOT(Update_Display_Puzzle_Number(QString)));

    connect( pushButtonSlowSolve, SIGNAL(clicked(bool)), this, SLOT(Slow_Solve()) );
    connect( pushButtonPauseSolve, SIGNAL(clicked(bool)), this, SLOT(Pause_Solve()) );
    connect( this, SIGNAL(Send_Pause()), sudokuTable, SLOT(Pause_Solve()) );
    connect( this, SIGNAL(Send_Slow_Solve()), sudokuTable, SLOT(Slow_Solve()) );
    connect( sudokuTable, SIGNAL(Solve_Done()), this, SLOT(Solve_Done()) );

    comboBoxFileSelect->setCurrentIndex(1);
    emit( comboBoxFileSelect->activated( comboBoxFileSelect->currentText()) );

    Read_Sudoku_Board();
    //Create_Legend_Window();
}



void MainWindowSudoku::Read_Sudoku_Board()
{
    sudokuTable->Read_Board_File(sourceDir.path().toStdString()+"/"+comboBoxFileSelect->currentText().toStdString(), spinBoxPuzzleNo->value()-1);
}

//decide if the puzzle number spinbox should be grayed out (file has 1 puzzle)
//If multiple puzzles, set the range so user can't select puzzle number higher than number puzzles in file
void MainWindowSudoku::Update_Display_Puzzle_Number(QString fileName)
{
    QFile tempFile(QString(sourceDir.path()+"/"+fileName));
    tempFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream txtStream(&tempFile);
    int lineCount = 0;

    while( !txtStream.atEnd())
    {
        QString garbage = txtStream.readLine();
        ++lineCount;
    }
    tempFile.close();

    if (lineCount < 20)
    {
        spinBoxPuzzleNo->setEnabled(false);
        labelPuzzleNo->setEnabled(false);
        spinBoxPuzzleNo->setValue(1);
    }
    else
    {
        spinBoxPuzzleNo->setEnabled(true);
        labelPuzzleNo->setEnabled(true);
        spinBoxPuzzleNo->setMaximum(lineCount/10);
    }
}

//just a sample new window, playng with how to make Qt things
void MainWindowSudoku::Create_Legend_Window(void)
{
    windowLegend = new QWidget();

    QGridLayout* gLayoutLegend = new QGridLayout();
    windowLegend->setLayout(gLayoutLegend);
    windowLegend->setWindowTitle("Solution Legend");

    //QWidget* widgetLegendLines = new QWidget(windowLegend);
    WidgetLegendLines* widgetLegendLines = new WidgetLegendLines(this);

    QLabel* labelLegendLabel1 = new QLabel("Last Number Possible in Cell",windowLegend);
    QLabel* labelLegendLabel2 = new QLabel("Need Every Number in Row",windowLegend);
    QLabel* labelLegendLabel3 = new QLabel("Need Every Number in Column",windowLegend);
    QLabel* labelLegendLabel4 = new QLabel("Need Every Number in Block",windowLegend);

    gLayoutLegend->addWidget(widgetLegendLines,0,0,4,1);
    gLayoutLegend->addWidget(labelLegendLabel1,0,1,1,1);
    gLayoutLegend->addWidget(labelLegendLabel2,1,1,1,1);
    gLayoutLegend->addWidget(labelLegendLabel3,2,1,1,1);
    gLayoutLegend->addWidget(labelLegendLabel4,3,1,1,1);
    gLayoutLegend->setColumnMinimumWidth(0,30);
    //gLayoutLegend->setHorizontalSpacing(5);
    //gLayoutLegend->setRowStretch();

//    QPainter painterLegend(widgetLegendLines);

//    painterLegend.setPen(QPen(sudokuTable->methodColors[1], 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
////    QBrush brushLegend(sudokuTable->methodColors[1]);
////    painterLegend.setBrush(brushLegend);
//    painterLegend.drawLine(5,5,25,5);
//    painterLegend.drawLine(5,55,25,55);
//    painterLegend.drawLine(5,105,25,105);
//    painterLegend.drawLine(5,155,25,155);

    windowLegend->show();
}

//animation of solving in order SudokuBoard figured it out
void MainWindowSudoku::Pause_Solve(void)
{
    if (pushButtonPauseSolve->text() == "pause" )
        pushButtonPauseSolve->setText("unpause");
    else
        pushButtonPauseSolve->setText("pause");

    emit( Send_Pause() );
}

void MainWindowSudoku::Slow_Solve()
{
    pushButtonPauseSolve->show();
    pushButtonPauseSolve->setText("pause");
    emit( Send_Slow_Solve() );
}

void MainWindowSudoku::Solve_Done(void)
{
    pushButtonPauseSolve->hide();
}
