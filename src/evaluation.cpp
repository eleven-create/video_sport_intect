#include "evaluation.h"
#include <iomanip>

Evaluator::Evaluator() : fps(0.0), processTimeMs(0.0), TP(0), FP(0), FN(0) {}

void Evaluator::start() { tm.reset(); tm.start(); }

void Evaluator::stop() {
    tm.stop();
    processTimeMs = tm.getTimeMilli();
    if (processTimeMs > 0) fps = 1000.0 / processTimeMs;
}

double Evaluator::getFPS() const { return fps; }
double Evaluator::getTime() const { return processTimeMs; }

// ⭐ 核心逻辑：基于 CDnet 标准的像素级比对
void Evaluator::updateMetrics(const cv::Mat& predMask, const cv::Mat& gtMask) {
    if (predMask.empty() || gtMask.empty() || predMask.size() != gtMask.size()) return;

    // 确保预测掩码是纯粹的 0 和 255
    cv::Mat pred_binary;
    cv::threshold(predMask, pred_binary, 127, 255, cv::THRESH_BINARY);

    // 根据 CDnet 2014 规则：
    // GT 中 255 为真实运动目标，0 为静态背景。85 和 170 为阴影和边缘，不参与评分。
    cv::Mat gt_motion = (gtMask == 255);
    cv::Mat gt_bg = (gtMask == 0);

    // 1. 计算 TP (预测是 255，且 GT 也是 255)
    cv::Mat tp_mat;
    cv::bitwise_and(pred_binary, gt_motion, tp_mat);
    TP += cv::countNonZero(tp_mat);

    // 2. 计算 FP (预测是 255，但 GT 其实是 0) -> 误报
    cv::Mat fp_mat;
    cv::bitwise_and(pred_binary, gt_bg, fp_mat);
    FP += cv::countNonZero(fp_mat);

    // 3. 计算 FN (预测是 0，但 GT 其实是 255) -> 漏报
    cv::Mat pred_inv;
    cv::bitwise_not(pred_binary, pred_inv);
    cv::Mat fn_mat;
    cv::bitwise_and(pred_inv, gt_motion, fn_mat);
    FN += cv::countNonZero(fn_mat);
}

double Evaluator::getPrecision() const {
    if (TP + FP == 0) return 0.0;
    return static_cast<double>(TP) / (TP + FP);
}

double Evaluator::getRecall() const {
    if (TP + FN == 0) return 0.0;
    return static_cast<double>(TP) / (TP + FN);
}

double Evaluator::getF1() const {
    double P = getPrecision();
    double R = getRecall();
    if (P + R == 0.0) return 0.0;
    return 2.0 * (P * R) / (P + R);
}

void Evaluator::printSummary() const {
    std::cout << "\n============================================\n";
    std::cout << "        CDnet 2014 量化评估成绩单\n";
    std::cout << "============================================\n";
    std::cout << "Precision (精确率) : " << std::fixed << std::setprecision(4) << getPrecision() * 100 << " %\n";
    std::cout << "Recall    (召回率) : " << std::fixed << std::setprecision(4) << getRecall() * 100 << " %\n";
    std::cout << "F1-Score  (综合得分) : " << std::fixed << std::setprecision(4) << getF1() * 100 << " %\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "TP 像素点总数: " << TP << "\n";
    std::cout << "FP 像素点总数: " << FP << " (误检)\n";
    std::cout << "FN 像素点总数: " << FN << " (漏检)\n";
    std::cout << "============================================\n\n";
}
