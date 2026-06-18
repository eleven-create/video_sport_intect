#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class UIViewer {
public:
    static void drawOSD(cv::Mat& frame, const std::string& algoName, double fps, double processTime);
    static void setupMainWindow();
    static void render(const cv::Mat& original, const cv::Mat& fgMask); // 备用单一渲染
    
    // ⭐ 新增：四宫格对比渲染器
    static void renderComparisonGrid(const cv::Mat& img1, const cv::Mat& img2, 
                                     const cv::Mat& img3, const cv::Mat& img4);
};
