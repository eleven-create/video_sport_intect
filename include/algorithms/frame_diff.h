#pragma once
#include "core/base_detector.h"

// 三帧差分检测器
class FrameDiff : public BaseDetector {
private:
    cv::Mat prevFrame1; // 上一帧 t-1
    cv::Mat prevFrame2; // 上上帧 t-2
    int thresholdVal;   // 差分二值化阈值

public:
    // 构造函数，可传入阈值（默认25）
    FrameDiff(int thresh = 25);

    // 实现基类接口
    void processFrame(const cv::Mat& frame, cv::Mat& fgMask) override;
};
