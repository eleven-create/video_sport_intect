#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;
int main()
{
    system("chcp 65001"); 

    VideoCapture cap(0); 
    
    if (!cap.isOpened()) {
        cout << "报告：摄像头打不开！请检查设备连接。" << endl;
        return -1;
    }

    Mat frame;      
    Mat grayFrame;  

    while (true) {
        cap >> frame; 
        
        if (frame.empty()) {
            cout << "读取不到画面，视频播放结束！" << endl;
            break; 
        }

        // 核心动作：转换为黑白图
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // 放映室
        imshow("彩色原图", frame);
        imshow("黑白图", grayFrame);

        if (waitKey(30) == 27) {
            cout << "接收到 ESC 指令，程序退出。" << endl;
            break;
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}