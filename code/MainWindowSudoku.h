#ifndef SUDOKUWINDOW_H
#define SUDOKUWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QString>
#include <QSpinBox>
#include <QTextStream>
#include <QIODevice>

#include "TableWidgetSudoku.h"
#include "WidgetLegendLines.h"
//#include "SudokuReader.h"

class MainWindowSudoku : public QWidget
{
    Q_OBJECT
private:
    QPushButton* pushButtonSolve, *pushButtonLoadFile, *pushButtonLoadImage, *pushButtonSlowSolve, *pushButtonPauseSolve;
    QCheckBox* checkboxShowLegend;
    QComboBox* comboBoxFileSelect, *comboBoxImageSelect;
    QLabel* labelFileSelect, *labelImageSelect, *labelPuzzleNo;
    QSpinBox* spinBoxPuzzleNo;
    QGridLayout* gLayoutMain;
    //QHBoxLayout* hLayoutLoad;
    //QVBoxLayout* vLayoutMain;
    QDir sourceDir;

    TableWidgetSudoku *sudokuTable;
    //SudokuImageReader sReader;

    QWidget* windowLegend;

    void Create_Legend_Window();

public:
    MainWindowSudoku(QWidget* _parent = 0);

signals:
    void Send_Pause(void);
    void Send_Slow_Solve(void);

private slots:
    void Read_Sudoku_Board_File(void);
    void Read_Sudoku_Board_Image(void);
    void Update_Display_Puzzle_Number(QString fileName);
    void Pause_Solve(void);
    void Slow_Solve(void);
    void Solve_Done(void);
};

#endif
