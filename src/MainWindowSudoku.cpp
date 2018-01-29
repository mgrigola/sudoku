#include "MainWindowSudoku.h"

MainWindowSudoku::MainWindowSudoku(QWidget* _parent) :
    windowLegend(nullptr)
{    
    this->setParent(_parent);
    this->setWindowTitle("Sudoku");
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); //this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMinimumSize(550,638);  //still can't figure out how to make the window stick to the
    sourceDir = QDir("../data");

    labelImageSelect = new QLabel("Image:");

    comboBoxImageSelect = new QComboBox(this);
    comboBoxImageSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    comboBoxImageSelect->setEditable(false);
    QStringList puzzleImages = sourceDir.entryList(QStringList("*.jpg"));
    for (const QString& file : puzzleImages)
        comboBoxImageSelect->addItem(file);

    pushButtonLoadImage = new QPushButton("Load Image", this);


    labelFileSelect = new QLabel("File:");

    comboBoxFileSelect = new QComboBox(this);
    comboBoxFileSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    comboBoxFileSelect->setEditable(false);
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

    pushButtonLoadFile = new QPushButton("Load File", this);


    sudokuTable = new TableWidgetSudoku(9, 9, this);

    pushButtonSolve = new QPushButton("Solve", this);
    pushButtonSlowSolve = new QPushButton("Slow Solve", this);
    pushButtonPauseSolve = new QPushButton("pause", this);
    pushButtonPauseSolve->hide();
    checkboxShowLegend = new QCheckBox("Show Legend", this);
    //checkboxShowLegend->show();    

    gLayoutMain = new QGridLayout();
    //gLayoutMain->setSizeConstraint(QLayout::SetMinimumSize);
    //gLayoutMain->setSizeConstraint(QLayout::SetNoConstraint);
    gLayoutMain->addWidget(labelImageSelect, 0, 0, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(comboBoxImageSelect, 0, 1, 1, 4, Qt::AlignLeft);
    gLayoutMain->addWidget(pushButtonLoadImage, 0, 7, 1, 1, Qt::AlignRight);

    gLayoutMain->addWidget(labelFileSelect, 1, 0, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(comboBoxFileSelect, 1, 1, 1, 4, Qt::AlignLeft);
    gLayoutMain->addWidget(labelPuzzleNo, 1, 5, 1, 1, Qt::AlignRight);
    gLayoutMain->addWidget(spinBoxPuzzleNo, 1, 6, 1, 1, Qt::AlignLeft);
    gLayoutMain->addWidget(pushButtonLoadFile, 1, 7, 1, 1, Qt::AlignRight);

    gLayoutMain->addWidget(sudokuTable, 2, 0, 5, 8);

    gLayoutMain->addWidget(pushButtonSolve, 7, 3, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(pushButtonSlowSolve, 7, 4, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(pushButtonPauseSolve, 7, 5, 1, 1, Qt::AlignHCenter);
    gLayoutMain->addWidget(checkboxShowLegend, 7, 6, 1, 1, Qt::AlignHCenter);

    gLayoutMain->setContentsMargins(4,4,4,4);

    setLayout(gLayoutMain);

    connect( pushButtonLoadImage, SIGNAL(clicked(bool)), this, SLOT(Read_Sudoku_Board_Image()) );

    connect( pushButtonLoadFile, SIGNAL(clicked(bool)), this, SLOT(Read_Sudoku_Board_File()) );  //need to pass string but too dumb to do properly :(
    connect( pushButtonSolve, SIGNAL(clicked(bool)), sudokuTable, SLOT(Solve_Board()) );
    connect( comboBoxFileSelect, SIGNAL(activated(QString)), this, SLOT(Update_Display_Puzzle_Number(QString)));

    connect( pushButtonSlowSolve, SIGNAL(clicked(bool)), this, SLOT(Slow_Solve()) );
    connect( pushButtonPauseSolve, SIGNAL(clicked(bool)), this, SLOT(Pause_Solve()) );
    connect( this, SIGNAL(Send_Pause()), sudokuTable, SLOT(Pause_Solve()) );
    connect( this, SIGNAL(Send_Slow_Solve()), sudokuTable, SLOT(Slow_Solve()) );
    connect( sudokuTable, SIGNAL(Solve_Done()), this, SLOT(Solve_Done()) );
    connect( checkboxShowLegend, SIGNAL(toggled(bool)), this, SLOT(Toggle_Checkbox_Show_Legend(bool)) );

    comboBoxFileSelect->setCurrentIndex(1);
    emit( comboBoxFileSelect->activated( comboBoxFileSelect->currentText()) );

    Read_Sudoku_Board_Image();
}



void MainWindowSudoku::Read_Sudoku_Board_File(void)
{
    sudokuTable->Read_Board_File(sourceDir.path().toStdString()+"/"+comboBoxFileSelect->currentText().toStdString(), spinBoxPuzzleNo->value()-1);
}

void MainWindowSudoku::Read_Sudoku_Board_Image(void)
{
    bool showContours = true;
    sudokuTable->Read_Board_Image(sourceDir.path().toStdString()+"/"+comboBoxImageSelect->currentText().toStdString(), showContours);
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

void MainWindowSudoku::Toggle_Checkbox_Show_Legend(bool isChecked)
{
    if (windowLegend == nullptr)
        Create_Legend_Window();
    else if (isChecked)
        windowLegend->show();
    else
        windowLegend->hide();
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
