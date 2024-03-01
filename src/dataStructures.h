#ifndef dataStructures_h
#define dataStructures_h

#include <opencv2/core.hpp>
#include <queue>
#include <vector>

struct DataFrame { // represents the available sensor information at the same
                   // time instance

  cv::Mat cameraImg; // camera image

  std::vector<cv::KeyPoint> keypoints; // 2D keypoints within camera image
  cv::Mat descriptors;                 // keypoint descriptors
  std::vector<cv::DMatch>
      kptMatches; // keypoint matches between previous and current frame
};

// data structure for queue the frame and allows to push the new frame to the
// back and pop the old frame from the front
struct FrameQueue {
  std::queue<DataFrame> dataBuffer;

  int BUFFER_SIZE;

  DataFrame lastFrame;
  DataFrame currentFrame;

public:
  // Constructor
  FrameQueue(int bufferSize) { this->BUFFER_SIZE = bufferSize; }
  // Destructor
  ~FrameQueue() {}

  // get the size of the queue
  int size() { return (int)this->dataBuffer.size(); }
  // obtain the next frame from the queue
  DataFrame getNextFrame();
  // obtain the last frame from the queue
  DataFrame getLastFrame();
  // obtain the second last frame from the queu
  DataFrame getSecondLastFrame();
  // Push the new frame to the back of the queue
  void push(DataFrame frame);
};
inline void FrameQueue::push(DataFrame frame) {
  if (this->dataBuffer.empty()) {
    this->dataBuffer.push(frame);
    this->lastFrame = frame;
    this->currentFrame = frame;

    return;
  }
  if ((int)this->dataBuffer.size() < BUFFER_SIZE - 1) {
    this->dataBuffer.push(frame);
    this->currentFrame = this->lastFrame;
    this->lastFrame = frame;

  } else {
    this->dataBuffer.pop();
    this->dataBuffer.push(frame);
    this->currentFrame = this->lastFrame;
    this->lastFrame = frame;
  }
}
inline DataFrame FrameQueue::getNextFrame() {
  DataFrame frame = this->dataBuffer.front();
  this->dataBuffer.pop();
  return frame;
};
inline DataFrame FrameQueue::getLastFrame() { return this->lastFrame; };
inline DataFrame FrameQueue::getSecondLastFrame() { return this->currentFrame; }

#endif /* dataStructures_h */
