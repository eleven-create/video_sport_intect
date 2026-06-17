#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>
#include "utils.h"

// 声明一个编码转换器：专门把 UTF-8 翻译成 Windows 认识的 GBK
std::string zh(const std::string& utf8Str);

// 绘制带标签的目标框
void drawBox(cv::Mat& img, cv::Rect rect, const std::string& label, cv::Scalar color);

// 计算矩形IOU（给NMS用）
float calcIOU(cv::Rect a, cv::Rect b);

// 批量保存掩码图片到data目录
void saveMask(const cv::Mat& mask, const std::string& path);


