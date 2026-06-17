#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>

// 引入我们新设计的架构头文件
#include "algorithms/frame_diff.h"
#include "algorithms/bg_subtractor.h"
#include "postprocess.h"
#include "algorithms/optical_flow.h"

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
    unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("MOG2");
    //unique_ptr<BaseDetector> detector = make_unique<OpticalFlowTracker>();
    //unique_ptr<BaseDetector> detector = make_unique<BGSubtractor>("KNN");

    Mat frame, fgMask;
    vector<Rect> bboxes;

    cout << "开始检测，按 ESC 键退出..." << endl;

    while (true) {
        cap >> frame; 
        
        if (frame.empty()) {
            cout << "读取不到画面，视频播放结束！" << endl;
            break; 
        }

        // ==========================================================
        // 1. 算法层：提取前景掩码 (屏蔽了底层细节，统一调用接口)
        // ==========================================================
        detector->processFrame(frame, fgMask);

        // ==========================================================
        // 2. 后处理层：形态学去噪与轮廓提取 (成员A负责的模块)
        // ==========================================================
        // 传入掩码，使用 5x5 的核进行开闭运算去噪
        PostProcessor::applyMorphology(fgMask, 5); 
        
        // 提取最小外接矩形，过滤掉面积小于 500 像素的噪点
        PostProcessor::findBoundingBoxes(fgMask, bboxes, 500); 

        // ==========================================================
        // 3. 可视化绘制
        // ==========================================================
        Mat displayFrame = frame.clone();
        for (const auto& box : bboxes) {
            // 在原图上绘制绿色矩形框，线宽为2
            rectangle(displayFrame, box, Scalar(0, 255, 0), 2);
            // 可以在框的上方打上标签
            putText(displayFrame, "Moving Object", Point(box.x, box.y - 5), 
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }

        // ==========================================================
        // 4. 画面放映
        // ==========================================================
        imshow("Original + BBoxes", displayFrame);   //原图与目标追踪
        imshow("Foreground Mask", fgMask);   //干净的前景掩码

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
