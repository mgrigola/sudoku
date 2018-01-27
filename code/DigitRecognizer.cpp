#include "DigitRecognizer.h"

DigitRecognizer::DigitRecognizer() :
    isInitialized(false)
{

}

DigitRecognizer::~DigitRecognizer()
{
    //delete pKNN;
}

uint32_t DigitRecognizer::Classify_Image(const cv::Mat& numImg)
{
    cv::Mat featureMat, outAry;
    Get_Image_Feature_Mat(numImg, featureMat);
    float val = pKNN->findNearest(featureMat, 1, outAry);  //wtf is this output array?
    //float val = pSVM->findNearest(numImg, 1, featureMat);  //wtf is this output array?
    return val;
}


//obviously i did not write this. I imagine this was written like before year 2000...
//the file is stored in little (or big? whatever is backwards) endian 4-byte unsigned integers. this flips it to correct direction
//read 4 byte
uint32_t DigitRecognizer::readFlippedInteger(FILE *fp)
{
    uint32_t ret = 0;  //this does assume 'int' is 4 bytes...
    uchar *temp = (uchar*)(&ret);  //uchar* temp = (uchar*)(&ret)
    std::fread(&temp[3], sizeof(uchar), 1, fp);
    std::fread(&temp[2], sizeof(uchar), 1, fp);
    std::fread(&temp[1], sizeof(uchar), 1, fp);
    std::fread(&temp[0], sizeof(uchar), 1, fp);

    return ret;
}


//okay!
//we need to assemble tr
bool DigitRecognizer::Train_Recognizer_Image_Grid(const std::string& trainPath, const std::string& labelsPath)
{
    std::string trPath = "C:/Users/m/Desktop/sudoku_test/digits.png";
    std::vector<size_t> testImgNos;

    cv::Mat digitsPng;          //input image/data/mat. one I'm using is a regular png picture. Often data sets use some custom binary format that would need reformatting
    cv::Mat trainSamples;       //the data to analyze (pictures of digits). Dims = [nSamples x pixelsInOneSample]. Each row is one digit image flattened into 1D row. in Ax=b, this is A.
    cv::Mat trainResponses;     //the known classification of each image (a digit 0-9). Dims = [nSamples x 1]
    cv::Mat oneDigitImg;        //intermediate step for clarity. this will hold image for one digit (temporarily, then converted and flattened)
    cv::Mat oneDigitImgFloat;   //intermediate step for clarity. same as oneDigitImgFloat but converted to float32, before flattening
    //cv::Mat oneDigitResponse = cv::Mat::zeros(cv::Size(1,1), CV_32SC1);  //the digit drawn in the picture, to be added to trainResponses

    digitsPng = cv::imread(trPath, CV_LOAD_IMAGE_GRAYSCALE);
    cv::Size eachDigitImageSize(20,20);
    cv::Size digitsPngTotalDims(2000,1000);
    size_t nDigitRows = digitsPngTotalDims.height / eachDigitImageSize.height;
    size_t nDigitCols = digitsPngTotalDims.width / eachDigitImageSize.width;

    //debugging ###
    cv::namedWindow("training images", cv::WINDOW_NORMAL);

    //in this case, digits.png is 2000x1000 pixels where each 20x20 block of pixels is a separate picture of a digit
    //dimensions are 100 images wide and 50 images deep. Each five rows are the same digit, starting with 0-4 are pictures of 0's. You can open digits.png and see it
    for (size_t digitRow=0; digitRow<nDigitRows; ++digitRow)
    {
        //oneDigitResponse.setTo(digitRow/5);
        for (size_t digitCol=0; digitCol<nDigitCols; ++digitCol)
        {
            cv::Point oneImageUpperLeft(digitCol*eachDigitImageSize.width, digitRow*eachDigitImageSize.height);
            cv::Rect oneImageBound(oneImageUpperLeft, eachDigitImageSize);
            oneDigitImg = digitsPng(oneImageBound);
            oneDigitImg.convertTo(oneDigitImgFloat, CV_32FC1);          //png is gonna be grayscale U8, fitting needs 32bit float
            trainSamples.push_back( oneDigitImgFloat.reshape(1,1) );    //flatten image: 2D to 1D, 20x20 to 400x1. reshape(#channels=1, #dims=1)
            //trainResponses.push_back( oneDigitResponse );
            trainResponses.push_back( int(digitRow/5) );

            //temp debugging ###
            if ((digitRow*20 + digitCol)%250 == 0)
            {
                cv::imwrite("dig_img"+std::to_string(digitRow/5)+"-"+std::to_string(20*digitRow+digitCol)+".png",oneDigitImg);
                testImgNos.push_back(digitRow*20 + digitCol);
                cv::imshow("training images", oneDigitImg);
                cv::waitKey(1000);
                std::cout << "testImgNo: " << digitRow*20 + digitCol << "    sampleNo: " <<  trainSamples.rows << std::endl;
            }
        }
    }

    //cv::Ptr<cv::ml::TrainData> tData = new cv::Ptr<cv::ml::TrainData()>;  //?
    //tData = cv::TrainData::create(trainSamples, cv::ml::SampleTypes::ROW_SAMPLE, );
    cv::Ptr<cv::ml::TrainData> tData = cv::ml::TrainData::create(trainSamples, cv::ml::SampleTypes::ROW_SAMPLE, trainResponses);

    //pKNN = cv::ml::KNearest::create();
    pKNN->setDefaultK(10);  //set number of top possiblities when finding nearest N? Or how many classes we guess there are? Or how many classifiers clusters we start with? not clear. (we'll actually only take 1)
    pKNN->setIsClassifier(true);
    pKNN->train(tData);

//    pSVM = cv::ml::SVM::create();
//    pSVM->

    //temp dugging ###
//    std::cout << "\n\nTesting!" << std::endl;
//    for (size_t imgNo=0; imgNo<testImgNos.size(); ++imgNo)
//    {
//        cv::Mat testSample = trainSamples.row(testImgNos[imgNo]);
//        std::cout << "known: " << imgNo/2 << "\t\tmeasured: " << pKNN->predict(testSample) << std::endl;
//        cv::Mat displaySample = testSample.reshape(1,20);
//        cv::imshow("training images", displaySample);
//        cv::waitKey(1000);
//    }
    return true;
}


bool DigitRecognizer::Train_Recognizer_Image_Column(const std::string& trainPath, const std::string& responsePath, const cv::Size& imgSize)
{
    //std::string trPath = "C:/Users/m/Desktop/sudoku_test/digits.png";
    //std::vector<size_t> testImgNos;

    cv::Mat trainPng;           //input image/data/mat. one I'm using is a regular png picture. Often data sets use some custom binary format that would need reformatting
    cv::Mat trainSamples;       //the data to analyze (pictures of digits). Dims = [nSamples x pixelsInOneSample]. Each row is one digit image flattened into 1D row. in Ax=b, this is A.
    cv::Mat responsePng;        //input known digits drawn in trainPng. Indices to match trainPng indices.
    cv::Mat responseSamples;    //the known classification of each image (a digit 0-9). Dims = [nSamples x 1]

//    cv::Mat oneDigitImg;        //intermediate step for clarity. this will hold image for one digit (temporarily, then converted and flattened)
    cv::Mat oneDigitFeatures;   //intermediate step for clarity. same as oneDigitImgFloat but converted to float32, before flattening

    trainPng = cv::imread(trainPath, CV_LOAD_IMAGE_GRAYSCALE);
    responsePng = cv::imread(responsePath, CV_LOAD_IMAGE_GRAYSCALE);

    cv::Size totalTrainSize(trainPng.cols,trainPng.rows);           //size of train img
    size_t nDigitRows = totalTrainSize.height / imgSize.height; //number of samples in train img (assuiming 1 x nSamples shape)
    size_t nDigitCols = totalTrainSize.width  / imgSize.width;  //should be 1. training set is 1 digit image wide.

    //in this case, digits.png is 2000x1000 pixels where each 20x20 block of pixels is a separate picture of a digit
    //dimensions are 100 images wide and 50 images deep. Each five rows are the same digit, starting with 0-4 are pictures of 0's. You can open digits.png and see it
    for (size_t digitRow=0; digitRow<nDigitRows; ++digitRow)
    {
        //oneDigitResponse.setTo(digitRow/5);
        for (size_t digitCol=0; digitCol<nDigitCols; ++digitCol)
        {
            cv::Point oneImageUpperLeft(digitCol*imgSize.width, digitRow*imgSize.height);
            cv::Rect oneImageBound(oneImageUpperLeft, imgSize);
            Get_Image_Feature_Mat(trainPng(oneImageBound), oneDigitFeatures);

            trainSamples.push_back( oneDigitFeatures );
            responseSamples.push_back( float(responsePng.at<uchar>(digitRow,digitCol)) );  //each entry in responseSamples is a single uchar value (0-9)
        }
    }

    std::cout << "CV_32F: " << CV_32F << std::endl;
    std::cout << "train type: " << trainSamples.type() << std::endl;
    std::cout << "response type: " << responseSamples.type() << std::endl;
    std::cout << "train cols: " << trainSamples.cols << std::endl;
    std::cout << "response cols: " << responseSamples.cols << std::endl;
    std::cout << "train rows: " << trainSamples.rows << std::endl;
    std::cout << "response rows: " << responseSamples.rows << std::endl;



    cv::Ptr<cv::ml::TrainData> tData = cv::ml::TrainData::create(trainSamples, cv::ml::SampleTypes::ROW_SAMPLE, responseSamples);

    //attempt 1: always use simple knn. Other models may work better. Also using just each pixel as the features is very suboptimal - can try stuff like contour moments
    Initialize_Classifier_KNN(tData);
    return true;
}


void DigitRecognizer::Get_Image_Feature_Mat(const cv::Mat& img, cv::Mat& featureMat)
{
    img.convertTo(featureMat, CV_32FC1);          //png is gonna be grayscale U8, fitting needs 32bit float (i think?)
    featureMat = featureMat.reshape(1,1);    //flatten image: 2D to 1D, 20x20 to 400x1. reshape(#channels=1, #dims=1)  //###does reshape alter or only return? i.e. do i need the=?
}


void DigitRecognizer::Initialize_Classifier_KNN(cv::Ptr<cv::ml::TrainData> tData)
{
    pKNN = cv::ml::KNearest::create();
    pKNN->setDefaultK(10);  //?set number of top possiblities when finding nearest N? Or how many classes we guess there are? Or how many classifiers clusters we start with? not clear. (we'll actually only take 1)
    pKNN->setIsClassifier(true);
    pKNN->train(tData);
    isInitialized = true;
}

bool DigitRecognizer::Is_Initialized(void)
{
    return isInitialized;
}
