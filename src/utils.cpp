#include <opencv2/opencv.hpp>
#include <string>

// 绘制目标框
void drawBox(cv::Mat& img, cv::Rect rect, const std::string& label, cv::Scalar color)
{
    // 1. 绘制目标矩形框
    int boxThickness = 2;                            // 线宽2
    cv::rectangle(img, rect, color, boxThickness);   // 绘制矩形框

    // 文字配置
    double fontScale = 0.5;    // 字体缩放比例
    int fontThickness = 1;     // 字体线宽
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;   // 字体类型

    // 2. 获取文字宽高、基线，根据文字大小计算标签背景框位置和尺寸
    int baseLine = 0;  // 基线：文字底部到基线的距离
    cv::Size textSize = cv::getTextSize(label, fontFace, fontScale, fontThickness, &baseLine);  //计算一段文字渲染出来有多宽、多高

    // 标签背景框坐标：放在矩形框左上角上方
    cv::Rect textBgRect(
        rect.x,
        rect.y - textSize.height - baseLine - 3,
        textSize.width + 6,
        textSize.height + baseLine + 3
    );

    // 边界保护：如果标签超出图片上边缘，把标签放到框下方
    if (textBgRect.y < 0)
    {
        textBgRect.y = rect.y + rect.height;
    }

    // 3. 填充标签背景色块
    //cv::rectangle(img, textBgRect, color, -1);

    // 4. 绘制白色文字，提升对比度
    cv::Point textPos(textBgRect.x + 3, textBgRect.y + textSize.height);
    cv::putText(img, label, textPos, fontFace, fontScale, cv::Scalar(255, 255, 255), fontThickness);
    //label:文字，textPos:文字位置，fontFace:字体类型，fontScale:字体缩放比例，cv::Scalar(255, 255, 255):字体、字号、白色，fontThickness:字体线宽
}

// 测试主函数
/*int main()
{
    // 创建测试图
    cv::Mat img(600, 800, CV_8UC3, cv::Scalar(40, 40, 40));
    
    // 目标框坐标
    cv::Rect targetRect(120, 150, 280, 320);
    std::string tag = "person 0.95";
    cv::Scalar boxColor(0, 255, 0); // 0蓝，255绿，0红->绿色框

    // 调用绘制函数
    drawBox(img, targetRect, tag, boxColor);

    cv::imshow("drawBox Demo", img);
    cv::waitKey(0);  //0代表无限等待
    cv::destroyAllWindows();
    return 0;
}*/