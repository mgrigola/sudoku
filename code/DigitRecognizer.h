#ifndef DIGITRECOGNIZER_H
#define DIGITRECOGNIZER_H

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/ml.hpp"

#include <string>
#include <fstream>
#include <iostream>

class DigitRecognizer
{
private:
    //cv::ml::KNearest* pKNN;
    cv::Ptr<cv::ml::KNearest> pKNN;

    uint32_t readFlippedInteger(FILE *fp);


public:
    DigitRecognizer();
    ~DigitRecognizer();

    bool Train_Recognizer(const std::string& trainPath, const std::string& labelsPath);
    uint32_t Classify_Image(const cv::Mat& numImg);

    bool train(char *trainPath, char *labelsPath);
};

#endif // DIGITRECOGNIZER_H
