#include "ui_viewer.h"
#include "core/config.h" // 确保路径正确
#include <iomanip>
#include <sstream>
#include <iostream>

namespace Config {
    int morphKernelSize = 5;
    int minArea = 100;
    int mergeMargin = 10;
}

void UIViewer::drawOSD(cv::Mat& frame, const std::string& algoName, double fps, double processTime) {
    std::ostringstream textStream;
    textStream << "Algorithm: " << algoName;
    std::string text1 = textStream.str();
    
    textStream.str("");
    textStream << std::fixed << std::setprecision(1);
    textStream << "FPS: " << fps << " | Time: " << processTime << " ms";
    std::string text2 = textStream.str();

    cv::Rect bgRect(10, 10, 320, 60);
    bgRect &= cv::Rect(0, 0, frame.cols, frame.rows); 
    
    if (bgRect.width > 0 && bgRect.height > 0) {
        cv::Mat overlay = frame.clone();
        cv::rectangle(overlay, bgRect, cv::Scalar(0, 0, 0), cv::FILLED);
        cv::addWeighted(overlay, 0.4, frame, 0.6, 0.0, frame); 
    }

    cv::putText(frame, text1, cv::Point(20, 35), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, text2, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2);
}

void UIViewer::setupMainWindow() {
    cv::namedWindow("Control Panel", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Morph Kernel", "Control Panel", &Config::morphKernelSize, 21);
    cv::createTrackbar("Min Area", "Control Panel", &Config::minArea, 5000);
    cv::createTrackbar("Merge Margin", "Control Panel", &Config::mergeMargin, 100);
    
    // 提前建好四宫格窗口，允许自由拉伸
    cv::namedWindow("Multi-Algorithm Comparison", cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);
    std::cout << "UI 控制面板初始化完成！" << std::endl;
}

void UIViewer::render(const cv::Mat& original, const cv::Mat& fgMask) {
    if (!original.empty()) cv::imshow("Main System", original);
    if (!fgMask.empty()) cv::imshow("Foreground Mask", fgMask);
}

// ⭐ 四宫格拼图渲染逻辑
void UIViewer::renderComparisonGrid(const cv::Mat& img1, const cv::Mat& img2, 
                                    const cv::Mat& img3, const cv::Mat& img4) {
    if (img1.empty() || img2.empty() || img3.empty() || img4.empty()) return;

    int w = img1.cols;
    int h = img1.rows;

    cv::Mat canvas(h * 2, w * 2, CV_8UC3, cv::Scalar(0, 0, 0));

    auto pasteToCanvas = [&](const cv::Mat& src, int x, int y) {
        cv::Mat temp;
        if (src.channels() == 1) cv::cvtColor(src, temp, cv::COLOR_GRAY2BGR);
        else temp = src.clone();
        temp.copyTo(canvas(cv::Rect(x, y, w, h)));
    };

    pasteToCanvas(img1, 0, 0);       // 左上
    pasteToCanvas(img2, w, 0);       // 右上
    pasteToCanvas(img3, 0, h);       // 左下
    pasteToCanvas(img4, w, h);       // 右下

    cv::line(canvas, cv::Point(w, 0), cv::Point(w, h * 2), cv::Scalar(255, 255, 255), 2);
    cv::line(canvas, cv::Point(0, h), cv::Point(w * 2, h), cv::Scalar(255, 255, 255), 2);

    cv::imshow("Multi-Algorithm Comparison", canvas);
}
