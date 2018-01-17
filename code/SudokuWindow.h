#ifndef SUDOKUWINDOW_H
#define SUDOKUWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QString>
#include <QSpinBox>
#include <QTextStream>
#include <QIODevice>

#include "SudokuTable.h"
#include "WidgetLegendLines.h"
#include "DigitRecognizer.h"

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
//#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"

class SudokuWindow : public QWidget
{
    Q_OBJECT
private:
    QPushButton* pushButtonSolve, *pushButtonLoad, *pushButtonSlowSolve, *pushButtonPauseSolve;
    QComboBox* comboBoxFileSelect;
    QLabel* labelFileLabel, *labelPuzzleNo;
    QSpinBox* spinBoxPuzzleNo;
    QGridLayout* gLayoutMain;
    QHBoxLayout* hLayoutLoad;
    QVBoxLayout* vLayoutMain;
    QDir sourceDir;

    SudokuTable *sudokuTable;
    DigitRecognizer digReader;

    QWidget* windowLegend;

    void Create_Legend_Window();
    void MyTransform(std::vector<cv::Point2f>& inputPts, std::vector<cv::Point2f>& outputPts, cv::Mat& M);

public:
    SudokuWindow(QWidget* _parent = 0);

signals:
    void Send_Pause(void);
    void Send_Slow_Solve(void);

private slots:
    void Read_Sudoku_Board(void);
    void Update_Display_Puzzle_Number(QString fileName);
    void Pause_Solve(void);
    void Slow_Solve(void);
    void Solve_Done(void);
};

#endif
