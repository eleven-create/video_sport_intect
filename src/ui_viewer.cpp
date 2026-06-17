#include "ui_viewer.h"
#include <iomanip>
#include <sstream>

void UIViewer::drawOSD(cv::Mat& frame, const std::string& algoName, double fps, double processTime) {
    // 准备文字信息
    std::ostringstream textStream;
    textStream << "Algorithm: " << algoName;
    std::string text1 = textStream.str();
    
    textStream.str(""); // 清空流
    textStream << std::fixed << std::setprecision(1); // 保留一位小数
    textStream << "FPS: " << fps << " | Time: " << processTime << " ms";
    std::string text2 = textStream.str();

    // 在左上角画一个半透明的黑色底板，让文字更清晰
    cv::Rect bgRect(10, 10, 320, 60);
    cv::Mat roi = frame(bgRect);
    cv::Mat color(roi.size(), CV_8UC3, cv::Scalar(0, 0, 0)); 
    // 0.4 是透明度
    cv::addWeighted(color, 0.4, roi, 0.6, 0.0, roi); 

    // 画文字 (绿色字体)
    cv::putText(frame, text1, cv::Point(20, 35), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, text2, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
}
