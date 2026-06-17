#include "algorithms/hybrid_fusion.h"

HybridFusion::HybridFusion() : frameCount(0) {
    // 采用 KNN 作为底层空间提取器，自带阴影剔除 (对应图中方案的“背景掩码”)
    pBackSub = cv::createBackgroundSubtractorKNN(500, 400.0, true);
}

void HybridFusion::processFrame(const cv::Mat& frame, cv::Mat& fgMask) {
    cv::Mat gray, rawMask;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // ==========================================================
    // 步骤 1：空间级 - 背景掩码提取 (粗提取)
    // ==========================================================
    // 第三个参数是 learningRate (0 到 1)。
    // 默认是 -1 (自动)。设为 0.005 意味着背景更新得很慢，小偷停下几秒钟依然会被框住！
    pBackSub->apply(frame, fgMask, 0.005); 
    // 二值化，将阴影(127)彻底变为0，只保留纯正前景(255)
    cv::threshold(rawMask, rawMask, 200, 255, cv::THRESH_BINARY);

    // ==========================================================
    // 步骤 2：时间级 - LK 光流运动矢量提取 (消除光照误检)
    // ==========================================================
    // 创建一个纯黑的掩码，用于存放真实的“运动轨迹”
    cv::Mat flowMask = cv::Mat::zeros(frame.size(), CV_8UC1);

    // 每隔 5 帧，或者特征点丢失时，重新在【前景区域(rawMask)】内寻找特征点！
    // 这是一个极大的优化：不在全图找点，只在疑似目标上找点。
    if (prevGray.empty() || frameCount % 5 == 0 || prevPts.empty()) {
        // 利用 rawMask 作为掩码提取角点
        cv::goodFeaturesToTrack(gray, prevPts, 100, 0.01, 10, rawMask);
    }

    if (!prevPts.empty() && !prevGray.empty()) {
        std::vector<cv::Point2f> nextPts;
        std::vector<uchar> status;
        std::vector<float> err;

        // 计算光流
        cv::calcOpticalFlowPyrLK(prevGray, gray, prevPts, nextPts, status, err);

        std::vector<cv::Point2f> goodNewPts;
        for (size_t i = 0; i < prevPts.size(); i++) {
            if (status[i]) {
                double distance = cv::norm(nextPts[i] - prevPts[i]);
                // 【核心逻辑】：只有真正发生了移动的特征点，才被认为是真实目标
                // 距离过小(可能是光照闪烁/摄像机微震)或过大(跟踪错误)都剔除
                if (distance > 1.0 && distance < 50.0) {
                    goodNewPts.push_back(nextPts[i]);
                    // 在光流掩码上画出粗壮的线或圆，代表“这里确实有东西在动”
                    cv::circle(flowMask, nextPts[i], 25, cv::Scalar(255), -1);
                }
            }
        }
        prevPts = goodNewPts; // 更新有效特征点
    }

    // 更新状态
    prevGray = gray.clone();
    frameCount++;

    // ==========================================================
    // 步骤 3：时空融合 (完美对应图中第3条方案)
    // ==========================================================
    // 膨胀光流掩码，让散落的圆点连成一片，覆盖整个目标
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(21, 21));
    cv::dilate(flowMask, flowMask, kernel);

    // 【融合大招】：逻辑与 (AND)
    // 只有在 rawMask(背景建模抠出来的人影) 且 flowMask(光流证明确实在动) 都为白色的地方，才保留！
    // 这样，闪烁的车灯、突然亮起的路灯（有轮廓但没运动矢量）瞬间被全部抹除！
    cv::bitwise_and(rawMask, flowMask, fgMask);
}
