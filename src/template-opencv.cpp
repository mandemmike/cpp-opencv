/*
* Copyright (C) 2020  Christian Berger
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications
#include "opendlv-standard-message-set.hpp"
 
// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;

float globalSteering;
float originalSteering;
 
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
 
cv::Mat findConeCenter(cv::Mat img)
{
   std::vector<std::vector<cv::Point>> contours = detectBlueConeContours(img);
   cv::Mat image_copy = img.clone();
   std::vector<cv::Moments> mu(contours.size());
   std::vector<cv::Point2f> mc(contours.size());
   float steeringAngle;
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
 
 
       cv::drawContours(image_copy, contours, -1, cv::Scalar(0, 255, 0), 2);
 
       // Draw a circle to show the center point
       cv::circle(image_copy, mc[0], 4, cv::Scalar(0, 0, 255), -1);
 
    steeringAngle = static_cast< float >(atan2(mc[0].y - mc[1].y, mc[0].x - mc[1].x));
       globalSteering = steeringAngle;
       angleString = "Angle: " + std::to_string(steeringAngle);

       cout << angleString << endl;
 
   return image_copy;
}      

 // Calculate the angle
   
       

/*
cv::putText(image_copy,               // target image
                   angleString,         // text
                   cv::Point(70, 295), // top-left position
                   cv::FONT_HERSHEY_DUPLEX,
                   1.0,
                   CV_RGB(255, 0, 0), // font color
                   1);
   //}*/
 
int32_t main(int32_t argc, char **argv)
{
   int32_t retCode{1};
   // Parse the command line parameters as we require the user to specify some mandatory information on startup.
   auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
   if ((0 == commandlineArguments.count("cid")) ||
       (0 == commandlineArguments.count("name")) ||
       (0 == commandlineArguments.count("width")) ||
       (0 == commandlineArguments.count("height")))
   {
       std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
       std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
       std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
       std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
       std::cerr << "         --width:  width of the frame" << std::endl;
       std::cerr << "         --height: height of the frame" << std::endl;
       std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
   }
   else
   {
       // Extract the values from the command line parameters
       const std::string NAME{commandlineArguments["name"]};
       const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
       const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
       const bool VERBOSE{commandlineArguments.count("verbose") != 0};
 
       // Attach to the shared memory.
       std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
       if (sharedMemory && sharedMemory->valid())
       {
           std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;
 
           // Interface to a running OpenDaVINCI session where network messages are exchanged.
           // The instance od4 allows you to send and receive messages.
           cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};
 
           opendlv::proxy::GroundSteeringRequest gsr;
           std::mutex gsrMutex;
           auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env)
           {
               // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
               // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
               std::lock_guard<std::mutex> lck(gsrMutex);
               gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
               originalSteering = gsr.groundSteering();
               std::cout << "lambda: groundSteering = " << originalSteering << std::endl;
           };
 
           od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);
 
           // Endless loop; end the program by pressing Ctrl-C.
           while (od4.isRunning())
           {
               // OpenCV data structure to hold an image.
               cv::Mat img;
 
               // Wait for a notification of a new frame.
               sharedMemory->wait();
 
               // Lock the shared memory.
               sharedMemory->lock();
               {
                   // Copy the pixels from the shared memory into our own data structure.
                   cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                   img = wrapped.clone();
               }
               // TODO: Here, you can add some code to check the sampleTimePoint when the current frame was captured.
               auto sampleTimeStampData = sharedMemory->getTimeStamp();
               sharedMemory->unlock();
 
               // TODO: Do something with the frame.
               // Example: Draw a red rectangle and display image.
               cv::rectangle(img, cv::Point(50, 50), cv::Point(100, 100), cv::Scalar(0, 0, 255));
               std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
               time_t convertedCurrentTime = std::chrono::system_clock::to_time_t(currentTime);
               char buf[sizeof "2011-10-08T07:07:09Z"];
               strftime(buf, sizeof buf, "%FT%TZ", gmtime(&convertedCurrentTime));
               std::string buffer = buf;
               std::string TS = std::to_string(cluon::time::toMicroseconds(sampleTimeStampData.second));
 
               std::string frameInfo = "Now: " + buffer + "; ts:" + TS + ";" + "Levkovets, Irina";
 
               // Display my name on the image
               cv::putText(img,               // target image
                           frameInfo,         // text
                           cv::Point(70, 70), // top-right position
                           cv::FONT_HERSHEY_DUPLEX,
                           1.0,
                           CV_RGB(255, 255, 255), // font color
                           1);
 
               // If you want to access the latest received ground steering, don't forget to lock the mutex:
               {
                   std::lock_guard<std::mutex> lck(gsrMutex);
                   std::cout << "main: groundSteering = " << gsr.groundSteering() << std::endl;
               }
 
               // Display image on your screen.
               if (VERBOSE)
               {
                   cv::Mat resultingImg = findConeCenter(img);
                   cv::imshow(sharedMemory->name().c_str(), getRegionOfInterest(resultingImg));
                   float deviation = static_cast<float>(globalSteering - originalSteering);
                   if(deviation > 0.05 || deviation < -0.05){
                       std::cout << "clear" << std::endl;
                   }else {
                       std::cout << "not clear" << std::endl;
                   }
                   cv::waitKey(1);
               }
           }
       }
       retCode = 0;
   }
   return retCode;
}


