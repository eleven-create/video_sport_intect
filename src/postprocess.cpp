#include "postprocess.h"
#include <iostream>
#include "utils.h"
void testHSVShadow(cv::Mat &frame) {
    cv::Mat hsvFrame,mask;
    cv::cvtColor(frame,hsvFrame,cv::COLOR_BGR2HSV);//将色彩原图转化为hsv格式
    cv::Scalar lowBound(0,0,0);
    cv::Scalar upperBound(180,255,80);//设定色彩的hsv上下限
    cv::inRange(hsvFrame,lowBound,upperBound,mask);
    cv::imshow(zh("HSV处理结果"),hsvFrame);
    cv::imshow(zh("抠图结果（白色为符合条件的）"),mask);
}
