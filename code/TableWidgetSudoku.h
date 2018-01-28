#ifndef TABLEWIDGETSUDOKU_H
#define TABLEWIDGETSUDOKU_H


#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QThread>      //### no longer needed?
#include <QTimer>

#include <thread>       //### no longer needed?
#include <chrono>       //### no longer needed?

#include <string>
#include <iostream>     //### no longer needed?
#include <fstream>      //### no longer needed?
#include "SudokuSolver.h"
#include "SudokuBoard.h"
#include "SudokuReader.h"
#include "DrawBorderDelegate.h"
#include "ColorMaps.h"

//testing some syntax - public struct to create a lookup table outside TableWidgetSudoku class
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

class TableWidgetSudoku : public QTableWidget
{
    Q_OBJECT
private:
    uint16_t nRows, nCols;   //really both 9, but first step towards generalizing probem is parameterize #rows and stuff
    SudokuSolver sSolver;
    SudokuReader sReader;
    SudokuBoard inputBoard, solutionBoard;
    DrawBorderDelegate* borderDrawer;
    QFont givenFont, guessFont;
    bool pauseSolve;

    QTimer* solveTimer;
    uint16_t solveStep;

    void Display_Input_Board(void);

public:
   TableWidgetSudoku(uint16_t _nRows = 9, uint16_t _nCols = 9, QWidget* _parent = 0);

signals:
    void Solve_Done(void);

public slots:
    void Read_Board_File(const std::string& fileName, uint16_t puzzleNo=0);
    void Read_Board_Image(const std::string& fileName);
    void Solve_Board(void);
    void Clear_Table(void);
    void Slow_Solve(void);
    void Pause_Solve(void);
    void Animate_Step(void);
};

#endif
