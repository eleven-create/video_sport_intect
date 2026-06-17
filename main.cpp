#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>

// 引入我们新设计的架构头文件
#include "algorithms/frame_diff.h"
#include "algorithms/bg_subtractor.h"
#include "algorithms/optical_flow.h"
#include "algorithms/hybrid_fusion.h"
#include "postprocess.h"
#include "evaluation.h"
#include "ui_viewer.h"

using namespace cv;
using namespace std;

int main() {
    // 解决 Windows 控制台中文乱码
    system("chcp 65001"); 
    
    // 打开摄像头 (也可以换成测试视频路径，如 "data/videos/test.mp4")
    VideoCapture cap(0); 
    
    if (!cap.isOpened()) {
        cerr << "报告：摄像头或视频源打不开！请检查设备连接。" << endl;
        return -1;
    }

    // ==========================================================
    // ⭐ 【核心亮点】：利用 C++ 多态动态切换对比算法
    // ==========================================================
    // 作为 Tier 1 的基线对比测试，你可以在这里自由取消注释来切换算法:
    // 
    //unique_ptr<BaseDetector> detector = make_unique<FrameDiff>(25);
    // string currentAlgoName = "Frame Difference";

    //unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("MOG2");
    // string currentAlgoName = "MOG2 Background Subtractor";

    //unique_ptr<BaseDetector> detector = make_unique<OpticalFlowTracker>();
    // string currentAlgoName = "LK Optical Flow";

    //unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("KNN");
    // string currentAlgoName = "KNN Background Subtractor";
    
    unique_ptr<BaseDetector> detector = make_unique<HybridFusion>();
    string currentAlgoName = "Hybrid Fusion (KNN + LK)";

    // ⭐ 2. 实例化外围工具
    // ==========================================================
    Evaluator eval;          // 性能评估器 (计算 FPS 和 耗时)
    BoxSmoother smoother;    // 秘籍三：EMA 框级平滑器

    Mat frame, fgMask;
    vector<Rect> bboxes;

    // 秘籍一：降采样缩放比例 (0.5 表示长宽都缩小为一半)
    const float scale = 0.5f; 

    cout << "系统初始化完毕，开始检测... (按 ESC 键退出)" << endl;

    while (true) {
        cap >> frame; 
        
        if (frame.empty()) {
            cout << "读取不到画面，视频播放结束！" << endl;
            break; 
        }

         // ⭐ 秘籍一：降维打击 (只在缩小的画面上跑沉重的算法)
        // ==========================================================
        Mat smallFrame;
        resize(frame, smallFrame, Size(), scale, scale);

        // --- 开始性能计时 ---
        eval.start();

        // ==========================================================
        // 1. 算法层：提取前景掩码 (屏蔽了底层细节，统一调用接口)
        // ==========================================================
        detector->processFrame(smallFrame, fgMask);

        // --- 停止性能计时 ---
        eval.stop();

        // ==========================================================
        // 2. 后处理层：形态学去噪与轮廓提取 (成员A负责的模块)
        // ==========================================================
        // 传入掩码，使用 5x5 的核进行开闭运算去噪
        PostProcessor::applyMorphology(fgMask, 5); 
        
        // 提取最小外接矩形，过滤掉面积小于 100 像素的噪点
        PostProcessor::findBoundingBoxes(fgMask, bboxes, 100); 
        // 2. ⭐ 调用框级融合算法，把碎框吸合（参数 20 表示相距 20 像素以内的框都会被合并）
        PostProcessor::mergeBoundingBoxes(bboxes, 10);

        // ⭐ 秘籍三：EMA 时序平滑滤波 (给框涂上润滑油，消除闪烁)
        // ==========================================================
        smoother.smoothBBoxes(bboxes);

        // ==========================================================
        // 3. 可视化绘制
        // ==========================================================
        Mat displayFrame = frame.clone();

        for (const auto& box : bboxes) {
            // 将小图里的框坐标，按比例放大回原图的真实坐标
            Rect realBox(box.x / scale, box.y / scale, box.width / scale, box.height / scale);
            
            // 绘制绿色矩形框
            rectangle(displayFrame, realBox, Scalar(0, 255, 0), 2);
            // 绘制标签
            putText(displayFrame, "Moving Target", Point(realBox.x, realBox.y - 8), 
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        }

        // ⭐ OSD 引擎渲染 (在左上角打上算法名称、FPS和耗时)
        UIViewer::drawOSD(displayFrame, currentAlgoName, eval.getFPS(), eval.getTime());

        // 放映画面 (使用全英文标题防止 Windows 乱码)
        imshow("Motion Detection System - Original", displayFrame);
        
        // 为了方便观察，把小尺寸的 Mask 放大回原图尺寸显示
        Mat displayMask;
        resize(fgMask, displayMask, frame.size());
        imshow("Motion Detection System - Mask", displayMask);

        // 接收到 ESC 指令 (ASCII码为27) 退出
        if (waitKey(30) == 27) {
            cout << "接收到 ESC 指令，程序退出。" << endl;
            break;
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}
