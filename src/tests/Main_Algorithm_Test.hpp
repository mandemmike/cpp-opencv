#ifndef Main_Algorithm_Test
#define Main_Algorithm_Test

// Include the single-file, header-only middleware libcluon to create
// high-performance microservices
#include "cluon-complete-v0.0.127.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


class AngleCalculator{

    public: 
        uint16_t Runner(uint16_t test);
        cv::Mat getRegionOfInterest(cv::Mat img);
        cv::Mat hsvConversion(cv::Mat img);
        std::vector<std::vector<cv::Point>> detectBlueConeContours(cv::Mat img);
        

};

#endif