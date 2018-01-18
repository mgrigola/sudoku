#ifndef SUDOKUREADER_H
#define SUDOKUREADER_H

#include "SudokuBoard.h"
#include "DigitRecognizer.h"
#include <string>
#include <fstream>
#include <iostream>

#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
//#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"

class SudokuReader
{
private:
    DigitRecognizer digitReader;
    uint16_t Read_File_Character(char cVal);
public:
    SudokuReader();
    bool Read_Board_File(SudokuBoard& inBoard, const std::string& fileName, uint16_t puzzleNo=0);
    bool Read_Board_Image(SudokuBoard& inBoard, cv::Mat rawBoardImg);
};

#endif
