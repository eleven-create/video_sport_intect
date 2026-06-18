#pragma once
#include "core/base_detector.h"
#include <string>

// 基于 OpenCV BackgroundSubtractor 的封装类
class BGSubtractor : public BaseDetector {
private:
    cv::Ptr<cv::BackgroundSubtractor> pBackSub; // OpenCV 背景减除器智能指针
    std::string algorithmType;

public:
    // 默认使用 KNN，因为你们调研发现它在复杂背景下表现极好
    BGSubtractor(const std::string& type = "KNN");

    void processFrame(const cv::Mat& frame, cv::Mat& fgMask) override;
};