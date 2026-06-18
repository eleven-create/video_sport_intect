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
void PostProcessor::mergeBoundingBoxes(std::vector<cv::Rect>& bboxes, int margin) {
    if (bboxes.empty()) return;

    bool merged = true;
    // 不断循环合并，直到没有任何框可以再合并为止
    while (merged) {
        merged = false;
        for (size_t i = 0; i < bboxes.size(); ++i) {
            for (size_t j = i + 1; j < bboxes.size(); ++j) {
                // 将框 A 向外膨胀一定的 margin 像素
                cv::Rect rectA = bboxes[i];
                cv::Rect inflatedA(
                    rectA.x - margin, 
                    rectA.y - margin, 
                    rectA.width + 2 * margin, 
                    rectA.height + 2 * margin
                );

                // 检查膨胀后的 A 是否与 B 相交 (即 A 和 B 的距离是否小于 margin)
                cv::Rect intersection = inflatedA & bboxes[j];
                
                if (intersection.area() > 0) {
                    // 如果相交/相近，则将这两个框合并为一个包含它们的大框 (求并集 |)
                    bboxes[i] = bboxes[i] | bboxes[j];
                    // 删除已经被合并的框 B
                    bboxes.erase(bboxes.begin() + j);
                    merged = true; // 标记发生了一次合并，需要重新检查
                    break;         // 跳出内层循环，重新从头开始检查合并后的框
                }
            }
            if (merged) break; // 跳出外层循环，重新开始
        }
    }
}
void BoxSmoother::smoothBBoxes(std::vector<cv::Rect>& currentBBoxes) {
    if (prevBBoxes.empty()) {
        prevBBoxes = currentBBoxes;
        return;
    }

    std::vector<cv::Rect> smoothedBoxes;
    for (auto& currBox : currentBBoxes) {
        bool matched = false;
        for (auto& prevBox : prevBBoxes) {
            // 计算两个框中心点的距离
            cv::Point currCenter(currBox.x + currBox.width/2, currBox.y + currBox.height/2);
            cv::Point prevCenter(prevBox.x + prevBox.width/2, prevBox.y + prevBox.height/2);
            double dist = cv::norm(currCenter - prevCenter);

            // 如果距离小于 50 像素，认为是同一个物体
            if (dist < 50.0) {
                // EMA 平滑公式 (0.6 的历史权重，0.4 的当前权重，让框的移动变得极其粘滞平滑)
                cv::Rect sBox;
                sBox.x = prevBox.x * 0.6 + currBox.x * 0.4;
                sBox.y = prevBox.y * 0.6 + currBox.y * 0.4;
                sBox.width = prevBox.width * 0.6 + currBox.width * 0.4;
                sBox.height = prevBox.height * 0.6 + currBox.height * 0.4;
                
                smoothedBoxes.push_back(sBox);
                matched = true;
                break;
            }
        }
        // 如果是新出现的物体，直接加入
        if (!matched) smoothedBoxes.push_back(currBox);
    }
    prevBBoxes = smoothedBoxes; // 更新记忆
    currentBBoxes = smoothedBoxes; // 输出平滑后的框
}