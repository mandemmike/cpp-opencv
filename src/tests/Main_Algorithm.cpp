#include "Main_Algorithm.hpp"

// Include the single-file, header-only middleware libcluon to create
// high-performance microservices
#include "cluon-complete-v0.0.127.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define SIZE 11

double GenerateRandom(double min, double max)
{
    static bool first = true;
    if (first)
    {
        srand(time(NULL));
        first = false;
    }
    if (min > max)
    {
        std::swap(min, max);
    }
    return min + (double)rand() * (max - min) / (double)RAND_MAX;
}

bool runner(){

    bool test[SIZE];

    for(int i = 1; i <= 10; i++) {
        std::string image_path = cv::samples::findFile("frame" + i + ".png");
        cv::Mat img = imread(image_path, IMREAD_COLOR);

        bool test[i] = findConeCenter(img, GenerateRandom(0.99, -0.99));

    }

    for(int i = 1; i < SIZE; i++) {
        if(!test[i]){
            return false;
        }
    }

    



}



cv::Mat getRegionOfInterest(cv::Mat img)
{
  cv::Rect ROI = cv::Rect(0, 265, 600, 100);
  img = img(ROI);
  return img;
}
cv::Mat hsvConversion(cv::Mat img)
{
  cv::Mat hsvImage;
  // Convert image to hsv format
  cv::cvtColor(img, hsvImage, cv::COLOR_BGR2HSV);
  cv::Mat thresh;
  cv::inRange(hsvImage, cv::Scalar(92, 91, 25), cv::Scalar(155, 162, 255), thresh); // finding blue cones
  return thresh;
}
std::vector<std::vector<cv::Point>> detectBlueConeContours(cv::Mat img)
{
  cv::Mat hsvImage = hsvConversion(img);
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(hsvImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);
  cv::Mat image_copy = img.clone();
  cv::drawContours(image_copy, contours, -1, cv::Scalar(0, 255, 0), 2);
  return contours;
}
bool findConeCenter(cv::Mat img, double originalSteering)
{
  std::vector<std::vector<cv::Point>> contours = detectBlueConeContours(img);
  cv::Mat image_copy = img.clone();
  std::vector<cv::Moments> mu(contours.size());
  std::vector<cv::Point2f> mc(contours.size());
  double steeringAngle;
  std::string angleString;
  for (size_t i = 0; i < contours.size(); i++)
  {
      mu[i] = cv::moments(contours[i]);
  }
  for (size_t i = 0; i < contours.size(); i++)
  {
      // add 1e-5 to avoid division by zero
      mc[i] = cv::Point2f(static_cast<float>(mu[i].m10 / (mu[i].m00 + 1e-5)),
                          static_cast<float>(mu[i].m01 / (mu[i].m00 + 1e-5)));
  }
  // for (size_t i = 0; i < contours.size(); i++)
  //{
      cv::drawContours(image_copy, contours, -1, cv::Scalar(0, 255, 0), 2);
      // Draw a circle to show the center point
      cv::circle(image_copy, mc[0], 4, cv::Scalar(0, 0, 255), -1);
 
      double numerator = (mc[1].y*(325 - mc[0].x)) + (365 * (mc[0].x - mc[1].x)) + (mc[0].y * (mc[1].y - 325));
      double denominator = (mc[1].x - 325) * (325 - mc[0].x) + (mc[1].y - 365 - mc[0].x);
 
 
 
      //double numerator = (mc[1].y*(mc[0].x - 325)) + (mc[0].y * (325 - mc[1].x)) + (365 * (mc[1].y - mc[0].x));
      //double denominator = (mc[1].x - mc[0].x) * (mc[0].x - 325) + (mc[1].y - mc[0].y - 365);
      double ratio = numerator/denominator;
 
      double angleRad = static_cast<float>(atan(ratio));
     
     steeringAngle = static_cast<float>(atan2(mc[0].y - mc[1].y, mc[0].x - mc[1].x));
     if (steeringAngle > 1.1){
         steeringAngle = 0.0;
     } else {
         steeringAngle = angleRad / 12;
     }
       //steeringAngle = atan2(mc[0].y - mc[1].y, mc[0].x - mc[1].x);
       //steeringAngle /= 10;
    angleString = "Angle: " + std::to_string(steeringAngle);
    double deviation = abs(originalSteering / 2);
    double difference = abs(static_cast<double>(steeringAngle - originalSteering));
    std::cout << angleString << std::endl;
 
    if(abs(originalSteering) == 0.0){
        deviation = 0.05;
    }
 
    if (difference == 0.0) {
        cleared++;
        count++;
        std::cout << "CLEAR" << std::endl;
        
 
 
        cv::putText(image_copy,               // target image
            angleString,         // text
            cv::Point(70, 295), // top-left position
            cv::FONT_HERSHEY_DUPLEX,
            1.0,
            CV_RGB(255, 0, 0), // font color
            1);
 
           return true;
 
    }
 
    if (difference < deviation) {  
           cleared++;
           count++;
           std::cout << "CLEAR" << std::endl;
    } else {
           std::cout << "NOT CLEAR" << std::endl;
           count++;
    }
    procent = (cleared / count) * 100;
 
 
    
 
  return true;
}     