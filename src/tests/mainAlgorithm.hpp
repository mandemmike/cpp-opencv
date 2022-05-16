#ifndef MAINALGORITHM
#define MAINALGORITHM

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


class mainAlgorithm {

    public: 
        cv::Mat getRegionOfInterest(cv::Mat img);
        cv::Mat hsvConversion(cv::Mat img);
        std::vector<std::vector<cv::Point>> detectBlueConeContours(cv::Mat img);
        std::int16_t findConeCenter(cv::Mat img, double originalSteering);



};

#endif