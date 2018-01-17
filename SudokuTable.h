#ifndef SUDOKUTABLE_H
#define SUDOKUTABLE_H


#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QThread>
#include <QTimer>

#include <thread>
#include <chrono>

#include <string>
#include <iostream>
#include <fstream>
#include "SudokuBoard.h"
#include "DrawBorderDelegate.h"
#include "ColorMaps.h"

//what in the fuck is this...?
//testing some syntax - public struct to create a lookup table outside SudokuTable class
//
//QColor MethodColorMap(int index)
//{
//    static const struct MyColorMap
//    {
//        QColor methodColors[8];
//        MyColorMap()
//        {

//        }
//    } converter;

//    return converter.methodColors[index];
//}

class SudokuTable : public QTableWidget
{
    Q_OBJECT
private:
    unsigned short sudokuRows, sudokuCols;   //really both 9, but first step towards generalizing probem is parameterize #rows and stuff
//    std::string openFile;
//    std::ifstream openStream;
    SudokuBoard sBoard;
    DrawBorderDelegate* borderDrawer;
    QFont givenFont, guessFont;
    bool pauseSolve;

    QTimer* solveTimer;
    unsigned short solveStep; //, solveRow, solveCol;

public:
    SudokuTable(int nRows = 9, int nCols = 9, QWidget* _parent = 0);
    //QTableWidget *sTable;
    //unsigned short inputBoard[9][9];   //the board as defined in puzzle (without anything calculated, only givens)

signals:
    void Solve_Done(void);

public slots:
    void Read_Board(const std::string& fileName, int puzzleNo=0);
    void Solve_Board(void);
    void Clear_Board(void);
    void Slow_Solve(void);
    void Pause_Solve(void);
    void Animate_Step(void);
};

#endif // SUDOKUTABLE_H
