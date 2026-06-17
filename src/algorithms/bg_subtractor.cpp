#include "algorithms/bg_subtractor.h"
#include <iostream>

BGSubtractor::BGSubtractor(const std::string& type) : algorithmType(type) {
    if (type == "MOG2") {
        // history=500, varThreshold=16, detectShadows=true (自带阴影检测，极其关键！)
        pBackSub = cv::createBackgroundSubtractorMOG2(500, 16.0, true);
    } else if (type == "KNN") {
        // KNN的阴影检测效果通常比MOG2更好，且能更好地容忍树叶晃动
        pBackSub = cv::createBackgroundSubtractorKNN(500, 400.0, true);
    } else {
        std::cerr << "未知的算法类型，默认回退到 KNN" << std::endl;
        pBackSub = cv::createBackgroundSubtractorKNN();
    }
}

void BGSubtractor::processFrame(const cv::Mat& frame, cv::Mat& fgMask) {
    // apply 函数会自动更新背景模型，并输出掩码
    // 注意：如果 detectShadows=true，阴影区域在 fgMask 中的像素值会被标记为 127
    pBackSub->apply(frame, fgMask);

    // 剔除阴影（把值为 127 的灰色像素变成 0，只保留 255 的纯正前景）
    cv::threshold(fgMask, fgMask, 200, 255, cv::THRESH_BINARY);
}
