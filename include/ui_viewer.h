#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// UI 渲染器 (成员 B 负责)
class UIViewer {
public:
    // ==========================================================
    // 现有功能：屏幕信息绘制 (保留不动)
    // ==========================================================
    /**
     * @brief 绘制 OSD 信息面板 (On-Screen Display)
     * @param frame 要绘制的彩色原图
     * @param algoName 当前使用的算法名称 (如 "Hybrid Fusion")
     * @param fps 当前帧率
     * @param processTime 耗时 (ms)
     */
    static void drawOSD(cv::Mat& frame, const std::string& algoName, double fps, double processTime);

    // ==========================================================
    // 新增功能：OpenCV 原生交互控制面板
    // ==========================================================
    /**
     * @brief 初始化主控制面板，创建滑动条 (绑定至 Config 全局变量)
     */
    static void setupMainWindow();

    /**
     * @brief 统一渲染多路画面，保持主循环代码整洁
     * @param original 原图 (需提前画好运动框和 OSD)
     * @param fgMask 前景掩码图 (干净的黑白图)
     */
    static void render(const cv::Mat& original, const cv::Mat& fgMask);
};