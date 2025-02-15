/* INCLUDES FOR THIS PROJECT */
#include "dataStructures.h"
#include "matching2D.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <sstream>
#include <vector>

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[]) {
  // clang-format off
  /* INIT VARIABLES AND DATA STRUCTURES */

  // data location
  string dataPath        = "../";

  // camera
  string imgBasePath     = dataPath + "images/";
  string imgPrefix       =
      "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
  string imgFileType     = ".png";
  int imgStartIndex      = 0; // first file index to load (assumes Lidar and camera
                         // names have identical naming convention)
  int imgEndIndex        = 9;   // last file index to load
  int imgFillWidth       =
      4; // no. of digits which make up the file index (e.g. img-0001.png)

  // misc
  int dataBufferSize     = 2; // no. of images which are held in memory (ring
                          // buffer) at the same time
  FrameQueue *dataBuffer = new FrameQueue(dataBufferSize); // list of data frames which are held in
                                      // memory at the same time
  // vector<DataFrame> dataBuffer;
  bool bVis              = true; // visualize results
  // total run time taken
  double sumRunTime      = 0.0;
  // total number of keypoints detected
  double numKeypoints    = 0.0;
  // total number of keypoints matched
  double numMatches      = 0.0;
  string detectorType ; 
  string descriptorType ;
  // clang-format on MAIN LOOP OVER ALL IMAGES */
  /// enable string-based selection based on descriptorType / ->
  /// BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT

  vector<string> descriptorTypeToUse =
      vector<string>{// "BRISK", "BRIEF", "ORB", "FREAK",
                     "SIFT", "AKAZE"};

  /// detectorType /
  ///-> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

  vector<string> detectorTypeToUse =
      vector<string>{"HARRIS", "SHITOMASI", "BRISK", "FAST", "ORB", "AKAZE",

                     "SIFT"};
  // LOOP THROUGH ALL COMBINATIONS OF DESCRIPTOR TYPE AND DETECTOR TYPE
  // for (string descriptorTypeUsing : descriptorTypeToUse) {
  //   for (string detectorTypeUsing : detectorTypeToUse) {
  //     FrameQueue *dataBuffer = new FrameQueue(
  //         dataBufferSize); // list of data frames which are held in
  //
  for (int imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++) {
    /* LOAD IMAGE INTO BUFFER */

    // assemble filenames for current index
    ostringstream imgNumber;
    imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
    string imgFullFilename =
        imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

    // load image from file and convert to grayscale
    cv::Mat img, imgGray;
    img = cv::imread(imgFullFilename);
    cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

    //// STUDENT ASSIGNMENT
    //// TODO : TASK MP.1 -> replace the following code with ring buffer of
    /// size
    /// dataBufferSize

    // push image into data frame buffer
    DataFrame frame;
    frame.cameraImg = imgGray;
    dataBuffer->push(frame);

    //// EOF STUDENT ASSIGNMENT
    // cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

    /* DETECT IMAGE KEYPOINTS */

    // extract 2D keypoints from current image
    vector<cv::KeyPoint>
        keypoints; // create empty feature list for current image
    detectorType = "FAST";

    //// STUDENT ASSIGNMENT
    //// TODO :TASK MP.2 -> add the following keypoint detectors in file
    /// matching2D.cpp and enable string-based selection based on
    /// detectorType /
    ///-> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

    double t = detKeypointsModern(keypoints, imgGray, detectorType, bVis);
    // if (detectorType.compare("SHITOMASI") == 0) {
    //   detKeypointsShiTomasi(keypoints, imgGray, false);
    // } else if (detectorType.compare) {
    //   //...
    // }
    //// EOF STUDENT ASSIGNMENT

    //// STUDENT ASSIGNMENT
    //// TODO : TASK MP.3 -> only keep keypoints on the preceding vehicle

    // only keep keypoints on the preceding vehicle
    bool bFocusOnVehicle = true;
    cv::Rect vehicleRect(535, 180, 180, 150);
    // cout << "Filter Keypoints outside the target vehicle" << endl;
    if (bFocusOnVehicle) {
      // ...
      keypoints.erase(
          remove_if(keypoints.begin(), keypoints.end(),
                    [&](cv::KeyPoint kp) {
                      return !(vehicleRect.x < kp.pt.x &&
                               kp.pt.x < vehicleRect.x + vehicleRect.width) ||
                             !(vehicleRect.y < kp.pt.y &&
                               kp.pt.y < vehicleRect.y + vehicleRect.height);
                    }),
          keypoints.end());
    }
    // cout << "Number of keypoints tracking on the vehicle: " <<
    // keypoints.size()
    //      << endl;
    // Log keypoints for Task 7
    if (imgIndex != 0) {
      numKeypoints += keypoints.size();
      sumRunTime += t;
    }

    //// EOF STUDENT ASSIGNMENT

    // optional : limit number of keypoints (helpful for debugging and
    // learning) Debug only
    bool bLimitKpts = false;
    if (bLimitKpts) {
      int maxKeypoints = 50;

      if (detectorType.compare("SHITOMASI") ==
          0) { // there is no response info, so keep the first 50 as they
               // are sorted in descending quality order
        keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
      }
      cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
      cout << " NOTE: Keypoints have been limited!" << endl;
    }
    // push keypoints and descriptor for current frame to end of data buffer
    // (dataBuffer.end() - 1)->keypoints = keypoints;
    DataFrame *currentFrame = dataBuffer->getCurrentFrame();
    currentFrame->keypoints = keypoints;
    // cout << "#2 : DETECT KEYPOINTS done" << endl;

    /* EXTRACT KEYPOINT DESCRIPTORS */

    //// STUDENT ASSIGNMENT
    /// TODO : TASK MP.4 -> add the following descriptors in file
    /// matching2D.cpp and
    /// enable string-based selection based on descriptorType / -> BRIEF,
    /// ORB, FREAK, AKAZE, SIFT

    cv::Mat descriptors;
    descriptorType = "BRISK"; // BRIEF, ORB, FREAK, AKAZE, SIFT, BRISK
    descKeypoints(currentFrame->keypoints, currentFrame->cameraImg, descriptors,
                  descriptorType);
    //// EOF STUDENT ASSIGNMENT

    // push descriptors for current frame to end of data buffer
    currentFrame->descriptors = descriptors;

    // cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

    if (dataBuffer->size() >
        1) // wait until at least two images have been processed
    {

      /* MATCH KEYPOINT DESCRIPTORS */

      string matcherType = "MAT_BF";           // MAT_BF, MAT_FLANN
      string descriptorTypeReq = "DES_BINARY"; // DES_BINARY, DES_HOG
      string selectorType = "SEL_KNN";         // SEL_NN, SEL_KNN
      //// STUDENT ASSIGNMENT
      //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
      //// TODO : TASK MP.6 -> add KNN match selection and perform
      /// descriptor / distance / ratio filtering with t=0.8 in file
      /// matching2D.cpp
      DataFrame *lastFrame = dataBuffer->getLastFrame();
      vector<cv::DMatch> matches;
      matchDescriptors(currentFrame->keypoints, lastFrame->keypoints,
                       currentFrame->descriptors, lastFrame->descriptors,
                       matches, descriptorType, matcherType, selectorType);
      if (imgIndex != 0) {
        numMatches += matches.size();
      }
      //// EOF STUDENT ASSIGNMENT

      // store matches in current data frame
      currentFrame->kptMatches = matches;

      // cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

      // visualize matches between current and previous image
      if (bVis) {
        cv::Mat matchImg = (currentFrame->cameraImg).clone();
        cv::drawMatches(currentFrame->cameraImg, currentFrame->keypoints,
                        lastFrame->cameraImg, lastFrame->keypoints, matches,
                        matchImg, cv::Scalar::all(-1), cv::Scalar::all(-1),
                        vector<char>(),
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        string windowName = "Matching keypoints between two camera images";
        cv::namedWindow(windowName, 7);
        cv::imshow(windowName, matchImg);
        // cout << "Press key to continue to next image" << endl;
        cv::waitKey(0); // wait for key to be pressed
      }
    }
  } // eof loop over all images
  cout << "The Descriptors are :" << descriptorType << endl
       << "The Detector are : " << detectorType << endl;
  cout << "The Average time taken to process an image is : "
       << (sumRunTime / 9) * 1000 / 1.0 << "ms" << endl;
  cout << "The Average number of keypoints detected is : " << numKeypoints / 9
       << endl;

  cout << "The Average number of keypoints matched is : " << numMatches / 9
       << endl;
  // Reset Counters
  sumRunTime = 0;
  numKeypoints = 0;
  numMatches = 0;
// } // end of looping detector
// } // end of looping descriptor
return 0;
}
