
#pragma once
#include <opencv2/opencv.hpp>

// 性能评估工具 (成员 B 负责)
class Evaluator {
private:
    int64 startTime;
    double fps;
    double processTimeMs;
    cv::TickMeter tm; // OpenCV 官方计时器

public:
    Evaluator();
    void start();           // 在算法 processFrame 之前调用
    void stop();            // 在算法完成后调用
    double getFPS() const;  // 获取当前 FPS
    double getTime() const; // 获取耗时 (毫秒)
};