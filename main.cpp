#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream> // ⭐ 用于读取 TXT 文件

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
    
    /*// 打开摄像头 (也可以换成测试视频路径，如 "data/videos/test.mp4")
    VideoCapture cap(0); 
    
    if (!cap.isOpened()) {
        cerr << "报告：摄像头或视频源打不开！请检查设备连接。" << endl;
        return -1;
    }*/
    //数据集路径配置 (请确认路径与你的文件夹名称完全一致！)
    string datasetPath = "D:/code/VisualStudio/C++/video/project/data/CDnet2014/pedestrians/";
    
    // 1. 读取评估范围 (temporalROI.txt)
    int startFrame = 1, endFrame = 999999;
    ifstream roiFile(datasetPath + "temporalROI.txt");
    if (roiFile.is_open()) {
        roiFile >> startFrame >> endFrame;
        cout << "数据集加载成功！有效评估范围: 第 " << startFrame << " 帧 到 第 " << endFrame << " 帧" << endl;
    } else {
        cerr << "报错：找不到 temporalROI.txt！请检查数据集路径是否正确：" << datasetPath << endl;
        return -1;
    }

    // ⭐ 【核心亮点】：利用 C++ 多态动态切换对比算法
    // 作为 Tier 1 的基线对比测试，你可以在这里自由取消注释来切换算法:
    // 
    //unique_ptr<BaseDetector> detector = make_unique<FrameDiff>(25);
    // string currentAlgoName = "Frame Difference";

    //unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("MOG2");
    //string currentAlgoName = "MOG2 Background Subtractor";

    //unique_ptr<BaseDetector> detector = make_unique<OpticalFlowTracker>();
    // string currentAlgoName = "LK Optical Flow";

    //unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("KNN");
    // string currentAlgoName = "KNN Background Subtractor";
    
    unique_ptr<BaseDetector> detector = make_unique<HybridFusion>();
    string currentAlgoName = "Hybrid Fusion (MOG2 + LK)";

    // ⭐ 2. 实例化外围工具
    Evaluator eval;          // 性能评估器 (计算 FPS 和 耗时)
    BoxSmoother smoother;    // 秘籍三：EMA 框级平滑器

    Mat frame, fgMask;
    vector<Rect> bboxes;

    int frameIdx = 1; // 帧数计数器
    cout << "开始高速跑分测试... (按 ESC 终止)" << endl;

    while (true) {
        // 自动拼装图片路径并读取 (in000001.jpg, in000002.jpg...)
        string imgPath = format("%sinput/in%06d.jpg", datasetPath.c_str(), frameIdx);
        frame = imread(imgPath);
        
        if (frame.empty()) {
            cout << "\n数据集读取完毕！" << endl;
            break; 
        }

    /*cout << "系统初始化完毕，开始检测... (按 ESC 键退出)" << endl;
    // ⭐ 开启降维加速！画面缩小一半，FPS 翻 4 倍！
    const float scale = 0.5f;
    while (true) {
        cap >> frame;  
        if (frame.empty()) {
            cout << "读取不到画面，视频播放结束！" << endl;
            break; 
        }
        // ⭐ 护盾一：强制统一画面格式！
        // 无论你的摄像头吐出来的是 4 通道还是单通道，强行剥离成标准的 3 通道 BGR
        if (frame.channels() == 4) cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
        if (frame.channels() == 1) cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
        // 安全缩小画面！
        Mat smallFrame;
        resize(frame, smallFrame, Size(), scale, scale);
        // 提前在小图尺寸上准备一张黑图，百分百防止 0x0 崩溃
        fgMask = Mat::zeros(smallFrame.size(), CV_8UC1);*/
        
        
        // --- 开始性能计时 ---
        eval.start();
        detector->processFrame(frame, fgMask);
        // --- 停止性能计时 ---
        eval.stop();

        // 量化评估 (仅在 temporalROI 规定的有效帧范围内进行)
        if (frameIdx >= startFrame && frameIdx <= endFrame) {
            // 读取对应的黑白真值图 (gt000001.png...)
            string gtPath = format("%sgroundtruth/gt%06d.png", datasetPath.c_str(), frameIdx);
            Mat gtMask = imread(gtPath, IMREAD_GRAYSCALE);
            
            // 将算法算出来的 fgMask 和真值 gtMask 交给评估器去算分！
            eval.updateMetrics(fgMask, gtMask);
        }

        // 2. 后处理层：形态学去噪与轮廓提取
        // 传入掩码，使用 5x5 的核进行开闭运算去噪
        PostProcessor::applyMorphology(fgMask, 5); 
        // 提取最小外接矩形，过滤掉面积小于 100 像素的噪点
        PostProcessor::findBoundingBoxes(fgMask, bboxes, 50); 
        // 调用框级融合算法，把碎框吸合（参数 10 表示相距 10 像素以内的框都会被合并）
        PostProcessor::mergeBoundingBoxes(bboxes, 10);

        // 秘籍三：EMA 时序平滑滤波 (给框涂上润滑油，消除闪烁)
        smoother.smoothBBoxes(bboxes);

        // 3. 可视化绘制
        Mat displayFrame = frame.clone();

        for (const auto& box : bboxes) {
            rectangle(displayFrame, box, Scalar(0, 255, 0), 2);
        }

        // OSD 显示当前进度
        UIViewer::drawOSD(displayFrame, currentAlgoName, eval.getFPS(), eval.getTime());
        putText(displayFrame, format("Frame: %d / %d", frameIdx, endFrame), 
                Point(20, 85), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 165, 255), 2);

        imshow("Dataset Testing - Original", displayFrame);

        // 安全校验：只有当 fgMask 不为空时才显示，防止第一帧空图崩溃
        if (!fgMask.empty()) imshow("Dataset Testing - Mask", fgMask);

        // 接收到 ESC 指令 (ASCII码为27) 退出
        // 如果想让它跑得飞快，把 waitKey(30) 改成 waitKey(1)
        if (waitKey(1) == 27) break; 
        
        frameIdx++;
    }
    
    destroyAllWindows();

    // 跑分结束，打印最终成绩单
    eval.printSummary();

    return 0;
}
