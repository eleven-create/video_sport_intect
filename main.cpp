#include <opencv2/opencv.hpp>
#include <iostream>
#include "postprocess.h"
#include "utils.h"
#include <windows.h>
using namespace cv;
using namespace std;

int main()
{
    system("chcp 65001"); 
    //打开摄像头
    VideoCapture cap(0); 
    
    if (!cap.isOpened()) {
        cout << zh("报告：摄像头打不开！请检查设备连接。") << endl;
        return -1;
    }

    Mat frame;      
    Mat grayFrame;  

    while (true) {
        cap >> frame; 
        
        if (frame.empty()) {
            cout << zh("读取不到画面，视频播放结束！") << endl;
            break; 
        }

        // 转换为黑白图
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // 进行放映
        imshow(zh("彩色原图"), frame);
        imshow(zh("黑白图"), grayFrame);
        //放映HSV图片
        testHSVShadow(frame);
        if (waitKey(30) == 27) {
            cout <<zh( "接收到 ESC 指令，程序退出。") << endl;
            break;
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}