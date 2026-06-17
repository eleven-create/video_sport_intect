#include "postprocess.h"

void PostProcessor::applyMorphology(cv::Mat& mask, int kernelSize) {
    if (mask.empty()) return;

    // 创建矩形核
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));

    // 先开运算（Open）：先腐蚀后膨胀，消除散落的小白点噪声
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    
    // 后闭运算（Close）：先膨胀后腐蚀，填补前景目标内部的“空洞”
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
}

void PostProcessor::findBoundingBoxes(const cv::Mat& mask, std::vector<cv::Rect>& bboxes, int minArea) {
    bboxes.clear();
    if (mask.empty()) return;

    // 查找外部轮廓（RETR_EXTERNAL：只提取最外层轮廓，忽略目标内部的孔洞轮廓）
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 遍历轮廓，滤除面积过小的，并生成外接矩形
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area >= minArea) {
            cv::Rect boundingBox = cv::boundingRect(contour);
            bboxes.push_back(boundingBox);
        }
    }
}
