#include "ui_viewer.h"
#include <iomanip>
#include <sstream>
#include <iostream> // 新增：用于终端输出提示
#include "../include/core/config.h"
// // 在唯一的源文件中真正分配内存并赋初值
namespace Config {
    int morphKernelSize = 5;
    int minArea = 500;
    int binarizeThresh = 127;
}

// 下面继续写 UIViewer 的已有实现代码...
// void UIViewer::drawOSD(...) { ... }

// ==========================================================
// 1. 现有功能：屏幕信息绘制 (保留你们优秀的防崩溃设计)
// ==========================================================
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
        // ==========================================================
        // ⭐ 护盾二：放弃 ROI 合成，改用全图安全融合！
        // 这种写法保证了 overlay 和 frame 的尺寸、通道数 100% 绝对一致，绝不崩溃！
        // ==========================================================
        cv::Mat overlay = frame.clone();
        cv::rectangle(overlay, bgRect, cv::Scalar(0, 0, 0), cv::FILLED);
        cv::addWeighted(overlay, 0.4, frame, 0.6, 0.0, frame); 
    }

    cv::putText(frame, text1, cv::Point(20, 35), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, text2, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 2);
}

// ==========================================================
// 2. 新增功能：初始化控制面板与滑动条
// ==========================================================
void UIViewer::setupMainWindow() {
    // 创建主控制台窗口
    cv::namedWindow("Control Panel", cv::WINDOW_AUTOSIZE);

    // 绑定滑动条到 Config 命名空间下的全局变量
    // 参数说明："滑动条名", "依附的窗口名", 绑定的整型变量地址, 最大值
    cv::createTrackbar("Morph Kernel", "Control Panel", &Config::morphKernelSize, 21);
    cv::createTrackbar("Min Area", "Control Panel", &Config::minArea, 5000);
    
    std::cout << "UI 控制面板初始化完成！" << std::endl;
}

// ==========================================================
// 3. 新增功能：统一渲染画面
// ==========================================================
void UIViewer::render(const cv::Mat& original, const cv::Mat& fgMask) {
    // 显示处理后的主画面 (带画框和OSD)
    if (!original.empty()) {
        cv::imshow("Main System", original);
    }
    
    // 显示干净的二值化掩码图 (方便调试阈值和去噪效果)
    if (!fgMask.empty()) {
        cv::imshow("Foreground Mask", fgMask);
    }
}