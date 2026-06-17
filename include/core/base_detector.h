#pragma once
#include <opencv2/opencv.hpp>

// 算法检测器的纯虚基类（接口类）
class BaseDetector {
public:
    virtual ~BaseDetector() = default;

    /**
     * @brief 统一处理单帧图像的接口
     * @param frame 输入的当前帧 (通常是 BGR 彩色图)
     * @param fgMask 输出的前景掩码 (二值图：255为运动前景，0为背景)
     */
    virtual void processFrame(const cv::Mat& frame, cv::Mat& fgMask) = 0;
};
