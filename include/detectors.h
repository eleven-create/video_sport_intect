#ifndef DETECTORS_H
#define DETECTORS_H
#include <opencv2/opencv.hpp>

enum DetType {
    MOG2,
    KNN,
    THREE_FRAME_DIFF
};

class ForegroundDetector
{
public:
    ForegroundDetector(DetType type);
    // 输入预处理完的灰度图，输出运动掩码
    cv::Mat getMask(const cv::Mat& gray);

private:
    DetType m_type;
    // MOG2/KNN实例
    cv::Ptr<cv::BackgroundSubtractor> mog2;
    cv::Ptr<cv::BackgroundSubtractor> knn;
    // 三帧差分缓存前两帧
    cv::Mat prev1, prev2;
};

#endif