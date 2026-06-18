#include "algorithms/optical_flow.h"

OpticalFlowTracker::OpticalFlowTracker(int maxPoints, int interval) 
    : maxCorners(maxPoints), trackInterval(interval), frameCount(0) {}

void OpticalFlowTracker::processFrame(const cv::Mat& frame, cv::Mat& fgMask) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // 初始化掩码为全黑
    fgMask = cv::Mat::zeros(gray.size(), CV_8UC1);

    // 如果是第一帧，或者达到了设定的间隔周期，或者特征点全丢了，就需要重新找特征点
    if (prevGray.empty() || frameCount % trackInterval == 0 || prevPts.empty()) {
        // 使用 Shi-Tomasi 算法寻找角点作为跟踪特征点
        cv::goodFeaturesToTrack(gray, prevPts, maxCorners, 0.01, 10);
        gray.copyTo(prevGray);
        frameCount++;
        return; 
    }

    std::vector<cv::Point2f> nextPts; // 当前帧的特征点
    std::vector<uchar> status;        // 跟踪状态 (1表示跟踪成功，0表示丢失)
    std::vector<float> err;           // 误差

    // 核心函数：计算金字塔 LK 光流
    cv::calcOpticalFlowPyrLK(prevGray, gray, prevPts, nextPts, status, err);

    std::vector<cv::Point2f> goodNewPts;

    // 遍历所有跟踪到的点
    for (size_t i = 0; i < prevPts.size(); i++) {
        // 如果点成功被跟踪到
        if (status[i]) {
            goodNewPts.push_back(nextPts[i]);

            // 计算运动向量 (当前点坐标 - 上一帧点坐标)
            cv::Point2f pt1 = prevPts[i];
            cv::Point2f pt2 = nextPts[i];
            
            // 计算移动的欧氏距离
            double distance = cv::norm(pt2 - pt1);
            
            // 过滤掉极其微小的移动 (可能是摄像机抖动造成的噪声)
            if (distance > 2.0 && distance < 50.0) { 
                // 将发生明显移动的特征点位置，在 Mask 上画个白色的圆，作为“前景”
                cv::circle(fgMask, pt2, 10, cv::Scalar(255), -1);
            }
        }
    }

    // 更新状态，为下一帧做准备
    prevGray = gray.clone();
    prevPts = goodNewPts;
    frameCount++;
}