#include "algorithms/frame_diff.h"

FrameDiff::FrameDiff(int thresh) : thresholdVal(thresh) {}

void FrameDiff::processFrame(const cv::Mat& frame, cv::Mat& fgMask) {
    cv::Mat grayFrame;
    // 强制转换为灰度图计算，降低算力消耗
    if (frame.channels() == 3) {
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
    } else {
        grayFrame = frame.clone();
    }

    // 如果历史帧为空（刚开始的前两帧），则初始化并返回全黑掩码
    if (prevFrame2.empty()) {
        if (prevFrame1.empty()) {
            prevFrame1 = grayFrame.clone();
        } else {
            prevFrame2 = prevFrame1.clone();
            prevFrame1 = grayFrame.clone();
        }
        fgMask = cv::Mat::zeros(grayFrame.size(), CV_8UC1);
        return;
    }

    cv::Mat diff1, diff2;
    // 步骤1：计算 D1 = |t - (t-1)|
    cv::absdiff(grayFrame, prevFrame1, diff1);
    // 步骤2：计算 D2 = |(t-1) - (t-2)|
    cv::absdiff(prevFrame1, prevFrame2, diff2);

    // 步骤3：二值化
    cv::threshold(diff1, diff1, thresholdVal, 255, cv::THRESH_BINARY);
    cv::threshold(diff2, diff2, thresholdVal, 255, cv::THRESH_BINARY);

    // 步骤4：逻辑与（AND）操作，保留共同的运动区域，消除重影
    cv::bitwise_and(diff1, diff2, fgMask);

    // 步骤5：更新历史帧
    prevFrame2 = prevFrame1.clone();
    prevFrame1 = grayFrame.clone();
}
