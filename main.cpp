#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <windows.h> 

#include "algorithms/frame_diff.h"
#include "algorithms/bg_subtractor.h"
#include "algorithms/optical_flow.h"
#include "algorithms/hybrid_fusion.h"
#include "postprocess.h"
#include "evaluation.h"
#include "ui_viewer.h"
#include "core/config.h"

using namespace cv;
using namespace std;

// ==========================================================
// ⭐ 乱码克星：将代码的 UTF-8 转换为 Windows 弹窗认识的 GBK
// ==========================================================
string utf8_to_gbk(const string& str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    delete[] szGBK;
    delete[] wszGBK;
    return strTemp;
}

// ==========================================================
// 模式 1：交互式四宫格实时对比演示
// ==========================================================
void runLiveDemo() {
    UIViewer::setupMainWindow();
    VideoCapture cap(0); 
    if (!cap.isOpened()) {
        MessageBoxA(NULL, utf8_to_gbk("报告：摄像头或视频源打不开！请检查设备连接。").c_str(), utf8_to_gbk("错误").c_str(), MB_OK | MB_ICONERROR);
        return;
    }

    unique_ptr<BaseDetector> detMOG2 = make_unique<BGSubtractor>("MOG2");
    unique_ptr<BaseDetector> detKNN = make_unique<BGSubtractor>("KNN");
    unique_ptr<BaseDetector> detFusion = make_unique<HybridFusion>();

    Evaluator evalMOG2, evalKNN, evalFusion;          
    BoxSmoother smoothMOG2, smoothKNN, smoothFusion;    
    Mat frame;

    const float scale = 0.5f;

    while (true) {
        cap >> frame; 
        if (frame.empty()) {
            int waitKey_end = waitKey(0) & 0xFF; 
            if (waitKey_end == 27) break;
            else if (waitKey_end == 'v' || waitKey_end == 'V') { cap.open("data/videos/test.mp4"); continue; }
            else if (waitKey_end == 'c' || waitKey_end == 'C') { cap.open(0); continue; }
        }

        if (frame.channels() == 4) cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
        if (frame.channels() == 1) cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);

        Mat smallFrame;
        resize(frame, smallFrame, Size(), scale, scale);

        Mat maskMOG2 = Mat::zeros(smallFrame.size(), CV_8UC1);
        Mat maskKNN = Mat::zeros(smallFrame.size(), CV_8UC1);
        Mat maskFusion = Mat::zeros(smallFrame.size(), CV_8UC1);

        Mat viewMOG2 = smallFrame.clone();
        Mat viewKNN = smallFrame.clone();
        Mat viewFusion = smallFrame.clone();
        Mat viewOriginal = smallFrame.clone(); 

        vector<Rect> boxMOG2, boxKNN, boxFusion;

        int safeKernel = Config::morphKernelSize;
        if (safeKernel < 1) safeKernel = 1;
        if (safeKernel % 2 == 0) safeKernel += 1;

        evalMOG2.start(); detMOG2->processFrame(smallFrame, maskMOG2); evalMOG2.stop();
        PostProcessor::applyMorphology(maskMOG2, safeKernel); 
        PostProcessor::findBoundingBoxes(maskMOG2, boxMOG2, Config::minArea); 
        PostProcessor::mergeBoundingBoxes(boxMOG2, Config::mergeMargin);
        smoothMOG2.smoothBBoxes(boxMOG2);

        evalKNN.start(); detKNN->processFrame(smallFrame, maskKNN); evalKNN.stop();
        PostProcessor::applyMorphology(maskKNN, safeKernel); 
        PostProcessor::findBoundingBoxes(maskKNN, boxKNN, Config::minArea); 
        PostProcessor::mergeBoundingBoxes(boxKNN, Config::mergeMargin);
        smoothKNN.smoothBBoxes(boxKNN);

        evalFusion.start(); detFusion->processFrame(smallFrame, maskFusion); evalFusion.stop();
        PostProcessor::applyMorphology(maskFusion, safeKernel); 
        PostProcessor::findBoundingBoxes(maskFusion, boxFusion, Config::minArea); 
        PostProcessor::mergeBoundingBoxes(boxFusion, Config::mergeMargin);
        smoothFusion.smoothBBoxes(boxFusion);

        auto drawMyBoxes = [](Mat& img, const vector<Rect>& bboxes) {
            for (const auto& box : bboxes) rectangle(img, box, Scalar(0, 255, 0), 2);
        };
        drawMyBoxes(viewMOG2, boxMOG2);
        drawMyBoxes(viewKNN, boxKNN);
        drawMyBoxes(viewFusion, boxFusion);

        UIViewer::drawOSD(viewMOG2, "MOG2", evalMOG2.getFPS(), evalMOG2.getTime());
        UIViewer::drawOSD(viewKNN, "KNN", evalKNN.getFPS(), evalKNN.getTime());
        UIViewer::drawOSD(viewFusion, "Hybrid Fusion", evalFusion.getFPS(), evalFusion.getTime());
        UIViewer::drawOSD(viewOriginal, "Original View", evalFusion.getFPS(), 0.0);

        UIViewer::renderComparisonGrid(viewMOG2, viewKNN, viewFusion, viewOriginal);

        int key = waitKey(30) & 0xFF;
        if (key == 27) break;
        else if (key == 'v' || key == 'V') cap.open("data/videos/test.mp4"); 
        else if (key == 'c' || key == 'C') cap.open(0);
    }
}

// ==========================================================
// 模式 2：CDnet 2014 数据集严谨跑分评估 (支持算法热切换)
// ==========================================================
void runDatasetEvaluation(int algoChoice) {
    string datasetPath = "D:/code/VisualStudio/C++/video/project/data/CDnet2014/wetSnow/";
    int startFrame = 1, endFrame = 999999;
    ifstream roiFile(datasetPath + "temporalROI.txt");
    if (roiFile.is_open()) {
        roiFile >> startFrame >> endFrame;
    } else {
        MessageBoxA(NULL, utf8_to_gbk("报错：找不到 temporalROI.txt！\n请检查数据集路径是否正确。").c_str(), utf8_to_gbk("错误").c_str(), MB_OK | MB_ICONERROR);
        return;
    }

    unique_ptr<BaseDetector> detector;
    string currentAlgoName;
    if (algoChoice == IDYES) {
        detector = make_unique<HybridFusion>();
        currentAlgoName = "Hybrid Fusion (MOG2+LK)";
    } else if (algoChoice == IDNO) {
        detector = make_unique<BGSubtractor>("MOG2");
        currentAlgoName = "MOG2 Baseline";
    } else if (algoChoice == IDCANCEL) {
        detector = make_unique<BGSubtractor>("KNN");
        currentAlgoName = "KNN Baseline";
    }

    Evaluator eval;          
    BoxSmoother smoother;    
    Mat frame, fgMask;
    vector<Rect> bboxes;
    int frameIdx = 1; 

    while (true) {
        string imgPath = format("%sinput/in%06d.jpg", datasetPath.c_str(), frameIdx);
        frame = imread(imgPath);
        if (frame.empty()) break; 

        eval.start();
        detector->processFrame(frame, fgMask);
        eval.stop();

        if (frameIdx >= startFrame && frameIdx <= endFrame) {
            string gtPath = format("%sgroundtruth/gt%06d.png", datasetPath.c_str(), frameIdx);
            Mat gtMask = imread(gtPath, IMREAD_GRAYSCALE);
            eval.updateMetrics(fgMask, gtMask);
        }

        PostProcessor::applyMorphology(fgMask, 5); 
        PostProcessor::findBoundingBoxes(fgMask, bboxes, 100); 
        PostProcessor::mergeBoundingBoxes(bboxes, 10);
        smoother.smoothBBoxes(bboxes);

        Mat displayFrame = frame.clone();
        for (const auto& box : bboxes) rectangle(displayFrame, box, Scalar(0, 255, 0), 2);

        UIViewer::drawOSD(displayFrame, currentAlgoName, eval.getFPS(), eval.getTime());
        putText(displayFrame, format("Frame: %d / %d", frameIdx, endFrame), Point(20, 85), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 165, 255), 2);

        imshow("Dataset Testing - Original", displayFrame);
        if (waitKey(1) == 27) {
            MessageBoxA(NULL, utf8_to_gbk("跑分已手动中断，将输出当前进度下的成绩单。").c_str(), utf8_to_gbk("提示").c_str(), MB_OK | MB_ICONINFORMATION);
            break; 
        }
        frameIdx++;
    }
    
    eval.printSummary();
    string resultMsg = "测试算法: " + currentAlgoName + "\n\n" +
                       "Precision (精确率): " + to_string(eval.getPrecision() * 100) + "%\n" +
                       "Recall (召回率): " + to_string(eval.getRecall() * 100) + "%\n" +
                       "F1-Score (综合得分): " + to_string(eval.getF1() * 100) + "%\n\n" +
                       "详情请查看控制台输出！";
    MessageBoxA(NULL, utf8_to_gbk(resultMsg).c_str(), utf8_to_gbk("跑分完成").c_str(), MB_OK | MB_ICONINFORMATION);
}

// ==========================================================
// 主函数：通过 Windows 弹窗进行引导
// ==========================================================
int main() {
    system("chcp 65001"); 

    int modeChoice = MessageBoxA(NULL, 
        utf8_to_gbk(
        "欢迎使用 运动目标智能检测与评估系统！\n\n"
        "点击 [ 是 ]：进入交互式四宫格实时演示 (可滑动调参)\n"
        "点击 [ 否 ]：进入 CDnet2014 数据集量化评估\n"
        "点击 [取消]：退出程序").c_str(), 
        utf8_to_gbk("系统模式选择").c_str(), 
        MB_YESNOCANCEL | MB_ICONQUESTION);
    
    if (modeChoice == IDYES) {
        runLiveDemo();
    } 
    else if (modeChoice == IDNO) {
        int algoChoice = MessageBoxA(NULL, 
            utf8_to_gbk(
            "请选择要进行跑分评估的算法：\n\n"
            "点击 [ 是 ]：测试 Hybrid Fusion (复合创新算法)\n"
            "点击 [ 否 ]：测试 MOG2 (传统基线算法)\n"
            "点击 [取消]：测试 KNN (传统基线算法)").c_str(), 
            utf8_to_gbk("评估算法选择").c_str(), 
            MB_YESNOCANCEL | MB_ICONQUESTION);
            
        runDatasetEvaluation(algoChoice);
    } 
    else {
        return 0; 
    }
    
    destroyAllWindows();
    return 0;
}
