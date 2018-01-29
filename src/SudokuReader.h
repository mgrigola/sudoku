#ifndef SUDOKUREADER_H
#define SUDOKUREADER_H

#include "SudokuBoard.h"
#include "DigitRecognizer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/calib3d.hpp"

class SudokuReader
{
private:
    cv::Mat sclImg, bgImg, fgImg, warpImg;
    DigitRecognizer digitReader;
    uint16_t Read_File_Character(char cVal);
    std::vector<cv::Point2f> fourCorners;

    bool Find_Sudoku_Board_Boundary(std::vector<cv::Point2f>& boundPts);
    bool Fix_Sudoku_Corner_Order(std::vector<cv::Point2f>& unorderedPts, std::vector<cv::Point2f>& orderedPts);
    bool Unwarp_And_Normalize_Board(int cellDim, int cellPadding, std::vector<cv::Point2f>& fourCornersInRaw);
    bool Get_Sudoku_Cell_Digit(cv::Mat& cellMat, cv::Mat& digitImg);

public:
    SudokuReader();
    bool Read_Board_File(SudokuBoard& inBoard, const std::string& fileName, uint16_t puzzleNo=0);
    bool Read_Board_Image(SudokuBoard& inBoard, const std::string& fileName);
    void Draw_Contour_Image(cv::Mat& contourImg);
};

#endif
