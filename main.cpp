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
#include "core/config.h" // ⭐ 新增：引入全局参数，连接滑动条与算法

using namespace cv;
using namespace std;

int main() {
    // 解决 Windows 控制台中文乱码
    system("chcp 65001"); 
    
    // ⭐ 1. 初始化控制面板 (成员B的 UI 模块)
    // ==========================================================
    UIViewer::setupMainWindow();

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
    string currentAlgoName = "Hybrid Fusion (MOG2 + LK)";

    // ⭐ 2. 实例化外围工具
    // ==========================================================
    Evaluator eval;          // 性能评估器 (计算 FPS 和 耗时)
    BoxSmoother smoother;    // 秘籍三：EMA 框级平滑器

    Mat frame, fgMask;
    vector<Rect> bboxes;

    cout << "系统初始化完毕，开始检测... " << endl;
    cout << "========================================" << endl;
    cout << "操作指南: \n [V] 播放测试视频 \n [C] 切换摄像头 \n [ESC] 退出" << endl;
    cout << "========================================" << endl;

    // ⭐ 开启降维加速！画面缩小一半，FPS 翻 4 倍！
    const float scale = 0.5f;

    while (true) {
        cap >> frame; 
        
        if (frame.empty()) {
            cout << "读取不到画面，视频播放结束！(可按 V 或 C 切换数据源)" << endl;
            // ⭐ 遇到视频播完不要直接 break，利用 waitKey 阻塞等待用户热切视频源
            int waitKey_end = waitKey(0) & 0xFF; 
            if (waitKey_end == 27) break;
            else if (waitKey_end == 'v' || waitKey_end == 'V') {
                cap.open("data/videos/test.mp4");
                continue;
            }
            else if (waitKey_end == 'c' || waitKey_end == 'C') {
                cap.open(0);
                continue;
            }
        }

        // ⭐ 护盾一：强制统一画面格式！
        // 无论你的摄像头吐出来的是 4 通道还是单通道，强行剥离成标准的 3 通道 BGR
        // ==========================================================
        if (frame.channels() == 4) cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
        if (frame.channels() == 1) cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);

        // ==========================================================
        // 安全缩小画面！
        // ==========================================================
        Mat smallFrame;
        resize(frame, smallFrame, Size(), scale, scale);
        
        // 提前在小图尺寸上准备一张黑图，百分百防止 0x0 崩溃
        fgMask = Mat::zeros(smallFrame.size(), CV_8UC1);

        // --- 开始性能计时 ---
        eval.start();

        // ==========================================================
        // 1. 算法层：提取前景掩码 (屏蔽了底层细节，统一调用接口)
        // ==========================================================
        // 直接将原图 frame 传入，不再进行任何缩放！
        detector->processFrame(smallFrame, fgMask);

        // --- 停止性能计时 ---
        eval.stop();

        // ==========================================================
        // 2. 后处理层：形态学去噪与轮廓提取 
        // ==========================================================
        // ⭐ 修改点：使用 Config::morphKernelSize 替代硬编码的 5，实现滑动条联动
        PostProcessor::applyMorphology(fgMask, Config::morphKernelSize); 
        
        // ⭐ 修改点：使用 Config::minArea 替代硬编码的 50，实现滑动条联动
        PostProcessor::findBoundingBoxes(fgMask, bboxes, Config::minArea); 
        
        // 调用框级融合算法，把碎框吸合（参数 10 表示相距 10 像素以内的框都会被合并）
        PostProcessor::mergeBoundingBoxes(bboxes, 10);

        // ⭐ 秘籍三：EMA 时序平滑滤波 (给框涂上润滑油，消除闪烁)
        // ==========================================================
        smoother.smoothBBoxes(bboxes);

        // ==========================================================
        // 3. 可视化绘制
        // ==========================================================
        Mat displayFrame = smallFrame.clone();

        for (const auto& box : bboxes) {
            // 不再需要恶心的 scale 换算，直接使用原始 box 坐标进行绘制！
            rectangle(displayFrame, box, Scalar(0, 255, 0), 2);
            putText(displayFrame, "Moving Target", Point(box.x, box.y - 8), 
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
        }

        // ⭐ OSD 引擎渲染 (在左上角打上算法名称、FPS和耗时)
        UIViewer::drawOSD(displayFrame, currentAlgoName, eval.getFPS(), eval.getTime());

        // ⭐ 4. 统一渲染放映画面 (调用 UIViewer 的 render 接口替代原生的 imshow)
        // ==========================================================
        UIViewer::render(displayFrame, fgMask);

        // ⭐ 5. 交互按键热切换监听
        // ==========================================================
        int key = waitKey(30) & 0xFF;
        
        if (key == 27) { // 接收到 ESC 指令退出
            cout << "接收到 ESC 指令，程序退出。" << endl;
            break;
        } 
        else if (key == 'v' || key == 'V') { // 热切换到视频
            cout << "切换到：本地测试视频" << endl;
            cap.release();
            cap.open("data/videos/test.mp4"); // 请确保测试视频在这个路径
        } 
        else if (key == 'c' || key == 'C') { // 热切换到摄像头
            cout << "切换到：实时摄像头" << endl;
            cap.release();
            cap.open(0);
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}