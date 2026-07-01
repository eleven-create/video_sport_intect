#include "algorithms/hybrid_fusion.h"

HybridFusion::HybridFusion() : frameCount(0) {
    // 底层引擎 MOG2
    pBackSub = cv::createBackgroundSubtractorMOG2(500, 16.0, true);
}

void HybridFusion::processFrame(const cv::Mat& frame, cv::Mat& fgMask) {
    if (frame.empty()) return;

    cv::Mat gray, rawMask;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // MOG2 提取空间背景 (使用自动学习率，让它快速捕捉目标)
    pBackSub->apply(frame, rawMask, -1); 
    
    if (rawMask.empty() || rawMask.size() != frame.size()) {
        rawMask = cv::Mat::zeros(frame.size(), CV_8UC1);
    } else {
        cv::threshold(rawMask, rawMask, 200, 255, cv::THRESH_BINARY);
    }
    
    cv::Mat flowMask = cv::Mat::zeros(frame.size(), CV_8UC1);

    // LK 光流提取
    if (prevGray.empty() || frameCount % 5 == 0 || prevPts.empty()) {
        // 在 MOG2 抠出的人影里找特征点
        cv::goodFeaturesToTrack(gray, prevPts, 300, 0.04, 10, rawMask);
    }

    if (!prevPts.empty() && !prevGray.empty()) {
        std::vector<cv::Point2f> nextPts;
        std::vector<uchar> status; 
        std::vector<float> err;    
        
        // 将光流搜索窗口放大到 cv::Size(31, 31)，金字塔层数增至 3，极大提升抗快速运动能力！
        cv::calcOpticalFlowPyrLK(prevGray, gray, prevPts, nextPts, status, err, cv::Size(31, 31), 3);

        std::vector<cv::Point2f> goodNewPts;
        for (size_t i = 0; i < prevPts.size(); i++) {
            if (status[i]) {
                double distance = cv::norm(nextPts[i] - prevPts[i]);
                //只要光流认为动了（>1.0），就不再设置上限，充分信任光流
                if (distance > 1.0) {
                    goodNewPts.push_back(nextPts[i]);
                    // 画一个半径为 20 的粗壮白色圆斑
                    cv::circle(flowMask, nextPts[i], 20, cv::Scalar(255), -1);
                }
            }
        }
        prevPts = goodNewPts; 
    }
    prevGray = gray.clone();
    frameCount++;

    // 时空融合
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(41, 41));
    cv::dilate(flowMask, flowMask, kernel);

    if (rawMask.type() != flowMask.type()) {
        flowMask.convertTo(flowMask, rawMask.type());
    }
    
    // 逻辑与融合：剔除 MOG2 产生的虚假反光和噪点
    cv::bitwise_and(rawMask, flowMask, fgMask);
}
