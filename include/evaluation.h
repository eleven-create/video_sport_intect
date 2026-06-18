#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>

// 性能与量化评估工具 (成员 B 负责)
class Evaluator {
private:
    // 时间评估指标
    int64 startTime;
    double fps;
    double processTimeMs;
    cv::TickMeter tm;

    // 量化评估指标 (CDnet 2014 标准)
    long long TP; // True Positive (检测对的像素)
    long long FP; // False Positive (误检的像素，比如噪点)
    long long FN; // False Negative (漏检的像素，比如小偷没被框出来)

public:
    Evaluator();
    
    // 计时功能
    void start();           
    void stop();            
    double getFPS() const;  
    double getTime() const; 

    // ⭐ 新增：量化统计功能
    void updateMetrics(const cv::Mat& predMask, const cv::Mat& gtMask);
    
    // 获取最终成绩
    double getPrecision() const; // 精确率 (查准率)
    double getRecall() const;    // 召回率 (查全率)
    double getF1() const;        // F1-Score (综合得分)
    
    // 打印最终跑分成绩单
    void printSummary() const;
};
