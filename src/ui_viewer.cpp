#include "ui_viewer.h"
#include <iomanip>
#include <sstream>

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
