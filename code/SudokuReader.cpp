#include "SudokuReader.h"

SudokuReader::SudokuReader()
{
}

//read board from text file (simple, directly grab each square)
bool SudokuReader::Read_Board_File(SudokuBoard& inBoard, const std::string& fileName, uint16_t puzzleNo)
{
    char lnTxt[50];  //don't think I actually use this... at least should read the full line then iterate over characters there instead of reading file chars 1 by 1?
    std::string lnStr;
    std::ifstream ifStream(fileName.c_str());
    if (!ifStream.is_open())     //whoops!
        return false;

    for (uint16_t pNo=0; pNo<puzzleNo; ++pNo)
    {
        std::getline(ifStream,lnStr,'\n');  //title
        for (uint16_t rowNo=0; rowNo<9; ++rowNo)  std::getline(ifStream,lnStr,'\n');  //puzzle rows
    }

    //this is some really old school method of reading a text file. Not very good practice
    ifStream.getline(lnTxt, 10);                                  //I believe this throws out the header/title row
    for (uint16_t rowNo=0; rowNo<9; ++rowNo)
    {
        for (uint16_t colNo=0; colNo<9; ++colNo)
            inBoard(rowNo,colNo) = Read_File_Character(ifStream.get());  //convert char to number. Store file data in input board  ### this setting format okay?

        ifStream.get();                                                       //throws away newline char...
    }
    return true;
}

uint16_t SudokuReader::Read_File_Character(char cVal)
{
    return cVal-48;  //convert from char to int (ascii stuff)
}




//read board from image file (cv::Mat). OCR and processing stuff. Requires a training set to teach the digitRecognizer
bool SudokuReader::Read_Board_Image(SudokuBoard& inBoard, const std::string& fileName)
{
    cv::Mat rawBoardImg = cv::imread(fileName, CV_LOAD_IMAGE_GRAYSCALE);
    if (rawBoardImg.channels() != 1)
        cv::cvtColor(rawBoardImg, rawBoardImg, cv::ColorConversionCodes::COLOR_BGR2GRAY);

    //resize input image into constant size. Mostly to make blur sizes consistent
    //this could be removed and blurring scaled... it's pretty slow to blur(80,80), and 4800x3600 px image. Faster to resize to 1200x900 + blur 20
    //cv::Size rawSize = cv::Size(rawMat.cols, rawMat.rows);
    int dim = std::sqrt(rawBoardImg.cols*rawBoardImg.cols + rawBoardImg.rows*rawBoardImg.rows);
    double scl = 1500.0/dim;  //want around x pixels for board diagonal? like 1200x900 ish
    cv::resize(rawBoardImg, sclImg, cv::Size(), scl, scl );

    //figure out where the sudoku board is in sclImg and how it's oriented/skewed.
    std::vector<cv::Point2f> fourCorners;
    if (!Find_Sudoku_Board_Boundary(fourCorners))
    {
        std::cout << "Failure - could not find 4 corners of sudoku board :(" << std::endl;
        return false;
    }

    //unskew board, crop it, and resize it so it's easy to loop over the cells. Input: fgImg. Output:warpImg
    int cellDim = 20;
    int cellPadding = 2;
    int cellTotSize = cellDim+2*cellPadding;
    if (!Unwarp_And_Normalize_Board(cellDim, cellPadding, fourCorners))
    {
        std::cout << "Failure - couldn't unwarp the sudoku board  :(" << std::endl;
        return false;
    }

    //assumes most of the trainingwork is done. Read nicely formatted training+response input files
    digitReader.Train_Recognizer_Image_Column("../data/train.png", "../data/response.png");

    //now extract digits from undistorted image of sudoku board and use the recognizer to interpret the numbers
    for (size_t row=0; row<9; ++row)
    {
        for (size_t col=0; col<9; ++col)
        {
            cv::Rect cellRect(cellTotSize*col + cellPadding + 1, cellTotSize*row + cellPadding + 1, cellDim, cellDim);  //### if change cellDim change this +offset!

            //seems to be a bug in findContours for opencv 3.1. can't find contours for mat defined as an ROI of another map. So we need to make a hard copy
            cv::Mat cellMat, digitMat;
            warpImg(cellRect).copyTo(cellMat);

            if (!Get_Sudoku_Cell_Digit(cellMat, digitMat))
            {
                inBoard(row,col) = 0;
                continue;
            }
            //cv::imshow("test detect image", cellMat);
            //cv::imshow("combo", digitMat);

            //cv::imwrite("dig_img "+std::to_string(row)+"-"+std::to_string(col)+".png", digitMat);
//            char c = cv::waitKey(0);

//            if (c == 27)  //escape == 27
//                return;

//            if (c == 32)  //space == 32
//                continue;

//            uchar trueVal = c-48;  //assuming c is "0"-"9"

//            trainMat.push_back(digitMat);
//            responseMat.push_back(trueVal);
            inBoard(row,col) = digitReader.Classify_Image(digitMat);
        }
    }

    return true;
}

//re-maps a skewed sudoku board to be a square aligned with x&y axes. Output is warpImg (member var)
//cellDim - size of image for each cell of the sudoku board, to feed to DigitRecognizer for identifying digits. Assumed square today. Used to determine # pixels in warpImg
//cellPadding - padding around each side of the cellDim x cellDim square used for framing the digit. The padding is thrown out (assuming some white space and bleed from grid lines.
//fourCornersInRaw - pixels locations of 4 corners of sudoku board in fgImg. Order (in cv::Mat frmt): upLeft, upRight, downRight, downLeft (0,0);(w,0);(w,h);(0,h)
bool SudokuReader::Unwarp_And_Normalize_Board(int cellDim, int cellPadding, std::vector<cv::Point2f>& fourCornersInRaw)
{
    int cellTotSize = cellDim+2*cellPadding;
    int transformSize = 9*(cellTotSize);  //252=28, 216=24, 576=64

    //note the assumed order here. fourCornersInRaw needs same order or board will be rotated or flipped, etc. Find_Sudoku_Board_Boundary/Fix_Sudoku_Corner_Order outputs same order
    std::vector<cv::Point2f> mapPts = {cv::Point2f(0,0), cv::Point2f(transformSize,0), cv::Point2f(transformSize,transformSize), cv::Point2f(0,transformSize)};
    cv::Mat H = cv::findHomography(fourCornersInRaw, mapPts);
    cv::warpPerspective(fgImg, warpImg, H, cv::Size(transformSize,transformSize) ); // warpInput.size() );  //warpInput
    return true;
}

bool SudokuReader::Find_Sudoku_Board_Boundary(std::vector<cv::Point2f>& boundPts)
{
    double blurSigma = sqrt(sclImg.cols*sclImg.cols + sclImg.rows*sclImg.rows) / 50;

    //correct for shading by bluring original image and subtract.
    cv::GaussianBlur(sclImg, bgImg, cv::Size(), blurSigma, blurSigma); //cv::GaussianBlur(rawMat,bgImg,cv::Size(),55,55);

    //bgImg we're trying to detect dark lines/numbers in original image, so blur-original is >0 where original is dark
    fgImg = bgImg-sclImg;

    cv::threshold(fgImg,fgImg,1,255, CV_8U);    //dark lines in original are 255 in fgImg, other is 0

    //identify the sudoku board via connected component analysis (finding the contours and looking at their props)
    //could also do like morphological close before findContours to try and fill gaps in border. Hasn;t been necessary yet but potential mode of failure
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(fgImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    //look for our sudoku. It should be roughly a square (possibly rotated and slightly warped). should also be fairly large, hopefull one of larger rects
    double bestFitVal = 0;
    size_t bestContNo = 0;
    cv::RotatedRect bestRotRect;
    for (size_t contNo=0; contNo<contours.size(); ++contNo)
    {
        cv::Moments moms = cv::moments(contours[contNo]);
        if (moms.m00 < 500)       //board needs to be pretty big to read the numbers. weed out obvious non-boards
            continue;

        //ideally rotRect.area ~= m00 -> contour is a rectangle-ish
        cv::RotatedRect rotRect = cv::minAreaRect(contours[contNo]);
        cv::Size2f rectSize = rotRect.size;
        double rArea        = rectSize.area();                                      //area of minimum bounding rect
        double areaDiff     = 1 - (std::abs(moms.m00 - rArea) / moms.m00 );         //difference in area between min rect and actual area (normalized by true area)
        double aspectDiff   = 1 - (std::abs(rectSize.width - rectSize.height) / (rectSize.width/2 + rectSize.height/2) );  //difference between length/width (normalized by average of length and width)
        double fitVal       = std::sqrt(rArea) * areaDiff * areaDiff * aspectDiff;  //some value I think is reasonable to determine the fitness of a contour as a sudoku box. certainly not very academic, I know

        if (fitVal > bestFitVal)
        {
            bestContNo = contNo;
            bestFitVal = fitVal;
            bestRotRect = rotRect;
        }
    }

    //approximate full contour as a polygon. hopefully this will be a quadrilateral...
    //if not we should iterate with different epsilon until boundCnt==4, but not implementing yet (this works for all my test images)
    double epsilon = std::sqrt(bestRotRect.size.area())/5;  // sqrt(area)/5 ~=1/20 perimeter? am i doing this right? On test image there's quite a large range that gives the expected 4pts, like 1/2th perim to 1/200th perim
    std::vector<cv::Point2f> tempBoundPts;
    cv::approxPolyDP(contours[bestContNo], tempBoundPts, epsilon, true);
    size_t boundCnt = tempBoundPts.size();

    //check that boundPts is correctly oriented - we assume upper left bound point is upper left sudoku pt (note assumption), and require points be listed in counterclockwise order
    Fix_Sudoku_Corner_Order(tempBoundPts, boundPts);

    return (boundCnt==4);
}



//we want ordered pts to always follow same direction
//looking at cv::Mat with 0,0 in upper left, the orientation is clockwise
//looking at like cartesian coords with 0,0 in lower left, the orientation is counterclockwise
//this is not at all general or elegant :(
bool SudokuReader::Fix_Sudoku_Corner_Order(std::vector<cv::Point2f>& unorderedPts, std::vector<cv::Point2f>& orderedPts)
{
    orderedPts.clear();
    orderedPts.reserve(4);

    //first point is upper left. actually it's kind of arbitrary assuming board is roughly facing natural direction in raw image
    cv::Point2f* upLeft;
    float minDistSqr = 999999.f;
    for (cv::Point2f& pt : unorderedPts)
    {
        float distSqr = pt.x*pt.x + pt.y*pt.y;
        if (distSqr < minDistSqr)
        {
            minDistSqr = distSqr;
            upLeft = &pt;
        }
    }

    orderedPts.push_back(*upLeft);

    //arbitrary point-1 (0,99999), point0 is upLeft, point1 is pt. Find pt with max angle
    double maxAng = 0;
    cv::Point2f* upRight;
    //### a needs to be flipped in direction: both vectors should originate at the last known point
    //cv::Vec2f a = cv::Vec2f( upLeft->x - 0,upLeft->y - 99999);  // -1->0 = [0]-[-1]
    cv::Vec2f a = cv::Vec2f( 0 - upLeft->x, 99999 - upLeft->y);  // 0->-1 = [-1]-[0]
    cv::Vec2f b;
    for (cv::Point2f& pt : unorderedPts)
    {
        if (&pt == upLeft)
            continue;

        b = cv::Vec2f(pt.x-upLeft->x, pt.y-upLeft->y); //0->1 = [1]-[0]
        double dot = a.dot(b);  //a.b?
        double mag = sqrt(a.dot(a)*b.dot(b));
        double ang = std::acos(dot/mag);
        if (ang>maxAng)
        {
            maxAng = ang;
            upRight = &pt;
        }
    }

    orderedPts.push_back(*upRight);

    maxAng = 0;
    cv::Point2f* downRight;
    a = cv::Vec2f( upLeft->x - upRight->x, upLeft->y - upRight->y);  // 1->0 = [0]-[1]
    for (cv::Point2f& pt : unorderedPts)
    {
        if (&pt == upLeft || &pt == upRight)
            continue;

        b = cv::Vec2f(pt.x-upLeft->x, pt.y-upLeft->y); //2->3
        double dot = a.dot(b);  //a.b?
        double mag = sqrt(a.dot(a)*b.dot(b)); // ||a||*||b||
        double ang = std::acos(dot/mag);  // arccos(a.b / ||a||*||b||) = theta?
        if (ang>maxAng)
        {
            maxAng = ang;
            downRight = &pt;
        }
    }

    orderedPts.push_back(*downRight);
    cv::Point2f* downLeft;

    for (cv::Point2f& pt : unorderedPts)
    {
        if (&pt == upLeft || &pt == upRight || &pt == downRight)
            continue;

        downLeft = &pt;
    }

    orderedPts.push_back(*downLeft);
    return true;
}


//starts with a rough image of a sudoku cell and cleans it up, removing superfluous contours and centering the digit
bool SudokuReader::Get_Sudoku_Cell_Digit(cv::Mat& cellMat, cv::Mat& digitImg)
{
    if (cv::mean(cellMat)[0] < 24)  //binarized to 0/255, so this says > 1/16 pixels are foreground/number
        return false;

    int cellW = cellMat.cols;
    int cellH = cellMat.rows;
    //this bit not needed if using fgImg as base for earp (fgImg is threshoded so 0 or 255, bgImg, is not normlized and would need this bit);
    //double minVal,maxVal,sclToMax;
    //cv::minMaxIdx(testDetectMat, &minVal, &maxVal, nullptr, nullptr);
    //sclToMax = 255/maxVal;
    //testDetectMat *= sclToMax;

    double maxContArea=0;
    int maxCont = -1;
    std::vector<std::vector<cv::Point>> cellContours;
    cv::findContours(cellMat, cellContours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE); //actually don't need hierarchy
//    std::vector<cv::Vec4i> hierarchy;
//    cv::findContours(cellMat, cellContours, hierarchy, cv::RetrievalModes::RETR_LIST, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE);

    for (size_t contNo=0; contNo<cellContours.size(); ++contNo)
    {
        cv::Moments cellMoms = cv::moments(cellContours[contNo]);    //use these as fit params/vars?
        if (cellMoms.m00 > (3*cellW*cellH/4))  //contour is pixels around edge and thus greater than or equal to size of entire mat...
            continue;

        if (cellMoms.m00 > maxContArea)
        {
            maxContArea = cellMoms.m00;
            maxCont = contNo;
            //cv::Point centPt(cellMoms.m01/cellMoms.m00, cellMoms.m10/cellMoms.m00);
        }
    }

    if (maxContArea < cellW*cellH/16)
        return false;

    cv::Rect bRect = cv::boundingRect(cellContours[maxCont]);
//    std::cout << bRect << std::endl;

//    size_t heirNo = maxCont;
    cv::Mat filtContMat = cv::Mat::zeros(cellW, cellH, CV_8UC1);
    cv::drawContours(filtContMat,cellContours,maxCont,cv::Scalar(255,255,255),cv::FILLED,cv::LINE_8);
//    cv::drawContours(redrawContMat,cellContours,maxCont,cv::Scalar(255,255,255),cv::FILLED,cv::LINE_8,hierarchy);

    //remove extraneous contours not part of digit (like grid lines bleeding into box around edges)
    cv::bitwise_and(cellMat, filtContMat, filtContMat);

    //digit generally isn't centered in cellMat, but now we have the contour we can shift it so bounding box around digit is centered
    digitImg = cv::Mat::zeros(cellW, cellH, CV_8UC1);
    cv::Rect centeredRect((cellW-bRect.width)/2, (cellH-bRect.height)/2, bRect.width, bRect.height);
    filtContMat(bRect).copyTo( digitImg(centeredRect) );

    //cv::Rect centeredRect((cellDim-bRect.width+1)/2, (cellDim-bRect.height+1)/2, bRect.width, bRect.height);
    //centeredMat(centeredRect) = filtContMat(bRect);
    return true;
}
