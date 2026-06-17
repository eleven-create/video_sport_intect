#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// UI 渲染器 (成员 B 负责)
class UIViewer {
public:
    /**
     * @brief 绘制 OSD 信息面板 (On-Screen Display)
     * @param frame 要绘制的彩色原图
     * @param algoName 当前使用的算法名称 (如 "Hybrid Fusion")
     * @param fps 当前帧率
     * @param processTime 耗时 (ms)
     */
    static void drawOSD(cv::Mat& frame, const std::string& algoName, double fps, double processTime);
};
