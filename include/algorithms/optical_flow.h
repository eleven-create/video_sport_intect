#pragma once
#include "core/base_detector.h"
#include <vector>

// LK (Lucas-Kanade) 稀疏光流检测器
class OpticalFlowTracker : public BaseDetector {
private:
    cv::Mat prevGray;                  // 上一帧的灰度图
    std::vector<cv::Point2f> prevPts;  // 上一帧的特征点
    int maxCorners;                    // 允许提取的最大特征点数量
    int trackInterval;                 // 每隔多少帧重新提取一次特征点
    int frameCount;                    // 当前处理的帧数计数器

public:
    OpticalFlowTracker(int maxPoints = 100, int interval = 10);

    // 实现基类接口
    void processFrame(const cv::Mat& frame, cv::Mat& fgMask) override;
};
