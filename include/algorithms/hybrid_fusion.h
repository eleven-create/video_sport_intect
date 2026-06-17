#pragma once
#include "core/base_detector.h"
#include <vector>

// 终极大招：背景建模(KNN)与LK光流的时空融合检测器
class HybridFusion : public BaseDetector {
private:
    cv::Ptr<cv::BackgroundSubtractor> pBackSub; // 背景建模器 (提供空间掩码)
    cv::Mat prevGray;                           // 上一帧灰度图 (用于光流)
    std::vector<cv::Point2f> prevPts;           // 跟踪的特征点
    int frameCount;                             // 帧计数器

public:
    HybridFusion();

    // 实现统一接口
    void processFrame(const cv::Mat& frame, cv::Mat& fgMask) override;
};
