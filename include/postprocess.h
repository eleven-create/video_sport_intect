#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

// 后处理工具类（静态方法，方便全局调用）
class PostProcessor {
public:
    /**
     * @brief 形态学操作去噪与连通
     * @param mask 输入/输出的二值掩码
     * @param kernelSize 形态学核大小 (默认5)
     */
    static void applyMorphology(cv::Mat& mask, int kernelSize = 5);

    /**
     * @brief 提取连通域并生成最小外接矩形 (Bounding Boxes)
     * @param mask 干净的二值掩码
     * @param bboxes 输出的矩形框列表
     * @param minArea 面积过滤阈值（滤除过小噪点）
     */
    static void findBoundingBoxes(const cv::Mat& mask, std::vector<cv::Rect>& bboxes, int minArea = 500);

    /**
     * @brief 框级融合算法：合并重叠或距离相近的矩形框
     * @param bboxes 输入/输出的矩形框列表
     * @param margin 允许的最大间距(像素)，小于这个间距的框会被吸合在一起 (默认20)
     */
    static void mergeBoundingBoxes(std::vector<cv::Rect>& bboxes, int margin = 20);
};
