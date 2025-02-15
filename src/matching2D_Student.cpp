#include "matching2D.hpp"
#include <cstdio>
#include <iostream>
#include <numeric>
#include <opencv2/features2d.hpp>

using namespace std;

// Find best matches for keypoints in two camera images based on several
// matching methods
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource,
                      std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource,
                      cv::Mat &descRef, std::vector<cv::DMatch> &matches,
                      std::string descriptorType, std::string matcherType,
                      std::string selectorType) {
  // TODO : configure matcher
  bool crossCheck = false;
  double ratio_thresh = 0.8;
  cv::Ptr<cv::DescriptorMatcher> matcher;

  if (matcherType.compare("MAT_BF") == 0) {
    int normType = cv::NORM_HAMMING;
    matcher = cv::BFMatcher::create(normType, crossCheck);
  } else if (matcherType.compare("MAT_FLANN") ==
             0) { // TODO : Use FLANN Matcher
    if (descSource.type() != CV_32F ||
        descRef.type() != CV_32F) { // OpenCV bug workaround : convert binary
                                    // descriptors to floating point due to a
                                    // bug in current OpenCV implementation
      descSource.convertTo(descSource, CV_32F);
      descRef.convertTo(descRef, CV_32F);
    }

    matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
  }
  // descriptors to floating point due to a
  // bug in current OpenCV implementation
  descSource.convertTo(descSource, CV_8U);
  descRef.convertTo(descRef, CV_8U);

  // TODO : perform matching task
  if (selectorType.compare("SEL_NN") == 0) { // nearest neighbor (best match)

    matcher->match(
        descSource, descRef,
        matches); // Finds the best match for each descriptor in desc1
  } else if (selectorType.compare("SEL_KNN") ==
             0) { // k nearest neighbors (k=2)

    // Flatten KNN Match point into Matches
    std::vector<std::vector<cv::DMatch>> knnMatches;
    matcher->knnMatch(descSource, descRef, knnMatches, 2);

    for (auto kp : knnMatches) {
      if (kp[0].distance < ratio_thresh * kp[1].distance)
        matches.push_back(kp[0]);
    }
  }
}
void blurImage(cv::Mat &img, cv::Mat &imgBlur, int radius) {
  cv::GaussianBlur(img, imgBlur, cv::Size(radius, radius), 0);
}
// Use one of several types of state-of-art descriptors to uniquely identify
// keypoints
double descKeypoints(vector<cv::KeyPoint> &keypoints, cv::Mat &img,
                     cv::Mat &descriptors, string descriptorType) {
  // TODO : select appropriate descriptor
  cv::Ptr<cv::DescriptorExtractor> extractor;
  int threshold = 30;        // FAST/AGAST detection threshold score.
  int octaves = 3;           // detection octaves (use 0 to do single scale)
  float patternScale = 1.0f; // apply this scale to the pattern used for
                             // sampling the neighbourhood of a keypoint.

  switch (DescriptorTypeDict.at(descriptorType)) {
  case DescriptorType::BRISK: {
    extractor = cv::BRISK::create(threshold, octaves, patternScale);
  } break;
  case DescriptorType::ORB: {
    // TODO :  Use ORB detectors
    extractor = cv::ORB::create();

  } break;
  case DescriptorType::SIFT: {
    extractor = cv::SIFT::create();
  } break;

  case DescriptorType::FREAK: {

    extractor = cv::xfeatures2d::FREAK::create();
  } break;
  case DescriptorType::AKAZE: {

    extractor = cv::AKAZE::create();
  } break;
  case DescriptorType::BRIEF: {

    extractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
  } break;
  }
  // perform feature description
  double t = (double)cv::getTickCount();

  extractor->compute(img, keypoints, descriptors);
  t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
  // cout << descriptorType << " descriptor extraction in " << 1000 * t / 1.0
  //      << " ms" << endl;
  return t;
}

// Detect keypoints in image using the traditional Shi-Thomasi detector
double detKeypointsShiTomasi(vector<cv::KeyPoint> &keypoints, cv::Mat &img,
                             bool bVis) {
  // compute detector parameters based on image size
  int blockSize = 4; //  size of an average block for computing a derivative
                     //  covariation matrix over each pixel neighborhood
  double maxOverlap = 0.0; // max. permissible overlap between two features in %
  double minDistance = (1.0 - maxOverlap) * blockSize;
  int maxCorners =
      img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

  double qualityLevel = 0.01; // minimal accepted quality of image corners
  double k = 0.04;

  // Apply corner detection
  double t = (double)cv::getTickCount();
  vector<cv::Point2f> corners;
  cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance,
                          cv::Mat(), blockSize, false, k);

  // TODO :add corners to result vector
  for (auto it = corners.begin(); it != corners.end(); ++it) {

    cv::KeyPoint newKeyPoint;
    newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
    newKeyPoint.size = blockSize;
    keypoints.push_back(newKeyPoint);
  }
  t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
  // cout << "Shi-Tomasi detection with n=" << keypoints.size() << " keypoints
  // in "
  //      << 1000 * t / 1.0 << " ms" << endl;

  // visualize results
  if (bVis) {
    cv::Mat visImage = (*&img).clone();
    cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    string windowName = "Shi-Tomasi Corner Detector Results";
    cv::namedWindow(windowName, 6);
    imshow(windowName, visImage);
    cv::waitKey(0);
  }
  return t;
}
double detKeypointsHarris(vector<cv::KeyPoint> &keypoints, cv::Mat &img,
                          bool bVis) {
  // compute detector parameters
  int blockSize = 3;
  int apertureSize = 3;
  double k = 0.05;
  int thresh = 100;

  double t = (double)cv::getTickCount();
  cv::Mat dst = cv::Mat::zeros(img.size(), CV_32FC1);
  cornerHarris(img, dst, blockSize, apertureSize, k);
  cv::Mat dst_norm, dst_norm_scaled;
  normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
  convertScaleAbs(dst_norm, dst_norm_scaled);
  double maxOverlap = 0.0; // max. permissible overlap between two features in
                           // %, used during non-maxima suppression
  for (size_t j = 0; j < dst_norm.rows; j++) {
    for (size_t i = 0; i < dst_norm.cols; i++) {
      int response = (int)dst_norm.at<float>(j, i);
      if (response > thresh) { // only store points above a threshold

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f(i, j);
        newKeyPoint.size = 2 * apertureSize;
        newKeyPoint.response = response;
        keypoints.push_back(newKeyPoint);
        // perform non-maximum suppression (NMS) in local neighbourhood around
        // new key point
        // bool bOverlap = false;
        // for (auto it = keypoints.begin(); it != keypoints.end(); ++it) {
        //   double kptOverlap = cv::KeyPoint::overlap(newKeyPoint, *it);
        //   if (kptOverlap > maxOverlap) {
        //     bOverlap = true;
        //     if (newKeyPoint.response >
        //         (*it).response) { // if overlap is >t AND response is higher
        //                           // for new kpt
        //       *it = newKeyPoint;  // replace old key point with new one
        //       break;              // quit loop over keypoints
        //     }
        //   }
        // }
        // if (!bOverlap) { // only add new key point if no overlap has been
        //                  // found in previous NMS
        //   keypoints.push_back(
        //       newKeyPoint); // store new keypoint in dynamic list
        // }
      }
    } // eof loop over cols
  }   // eof loop over rows
  t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
  // cout << "Shi-Tomasi detection with n=" << keypoints.size() << " keypoints
  // in "
  //      << 1000 * t / 1.0 << " ms" << endl;

  if (bVis) {
    cv::Mat visImage = (*&img).clone();
    cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1),
                      cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    string windowName = "Harris Corner Detector Results";
    cv::namedWindow(windowName, 6);
    imshow(windowName, visImage);
    cv::waitKey(0);
  }
  return t;
}
double detKeypointsModern(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img,
                          std::string detectorType, bool bVis) {

  cv::Ptr<cv::FeatureDetector> detector;

  bool useModern = true; //
  double t = 0.0;
  // using matching detector
  switch (DetectorTypeDict.at(detectorType)) {
  case DetectorType::SHITOMASI: {
    useModern = false;
    t = detKeypointsShiTomasi(keypoints, img, bVis);
  } break;
  case DetectorType::FAST: {
    detector = cv::FastFeatureDetector::create();
  } break;
  case DetectorType::HARRIS: {
    useModern = false;
    t = detKeypointsHarris(keypoints, img, bVis);
  } break;
  case DetectorType::BRISK: {
    detector = cv::BRISK::create();
  } break;
  case DetectorType::BRIEF: {
    cout << "BRIEF is not a detector, use it in descriptor extraction instead"
         << endl;
    return -1;
  } break;
  case DetectorType::ORB: {
    detector = cv::ORB::create();
  } break;
  case DetectorType::AKAZE: {
    detector = cv::AKAZE::create();
  } break;
  case DetectorType::SIFT: {
    detector = cv::SIFT::create();
  } break;
  }
  // perform feature description
  if (useModern) {
    t = (double)cv::getTickCount();

    img.convertTo(img, CV_8U);
    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    // cout << detectorType << " descriptor extraction in " << 1000 * t / 1.0
    //      << " ms" << endl;
    // visualize results
    if (bVis) {
      cv::Mat visImage = (*&img).clone();
      cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1),
                        cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
      string windowName = detectorType + "Detector Results";
      // char *windowName;
      // windowName =
      //     (char *)std::malloc(sizeof(char) * (strlen(windowName) + 30));
      // sprintf(windowName, "%s Corner Detector Results",
      // detectorType.c_str());
      cv::namedWindow(windowName, 6);
      imshow(windowName, visImage);
      cv::waitKey(0);
    }
  }
  return t;
}
