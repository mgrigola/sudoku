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
    bool isInitialized;
    cv::Ptr<cv::ml::KNearest> pKNN;
    cv::Ptr<cv::ml::SVM> pSVM;

    uint32_t readFlippedInteger(FILE *fp);
    void Get_Image_Feature_Mat(const cv::Mat& img, cv::Mat& featureMat);
    void Initialize_Classifier_KNN(cv::Ptr<cv::ml::TrainData> tData);

public:
    DigitRecognizer();
    ~DigitRecognizer();

    bool Train_Recognizer_Image_Grid(const std::string& trainPath, const std::string& labelsPath);
    bool Train_Recognizer_Image_Column(const std::string& trainPath, const std::string& responsePath, const cv::Size& imgSize = cv::Size(20,20));
    uint32_t Classify_Image(const cv::Mat& numImg);

    //bool train(char *trainPath, char *labelsPath);
    bool Is_Initialized(void);
};

#endif // DIGITRECOGNIZER_H
