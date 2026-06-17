#include "utils.h"
#include <windows.h> // 必须呼叫 Windows 底层 API
#include <opencv2/opencv.hpp>
#include <string>
#include <algorithm>
#include <cstdio>

std::string zh(const std::string& utf8Str) {
    // 1. 先把 UTF-8 翻译成宽字符 (UTF-16)
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (len == 0) return utf8Str;
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], len);

    // 2. 再把宽字符翻译成 Windows 认识的 GBK (CP_ACP)
    len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) return utf8Str;
    std::string str(len, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], len,NULL,NULL);

    // 清理一下结尾的空白符
    if (!str.empty() && str.back() == '\0') str.pop_back();

    return str;
}
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

// 计算两个矩形框的IOU(IOU=交集面积/并集面积)
float calcIOU(cv::Rect a, cv::Rect b)
{
    // 步骤1：求相交区域左上角x、y（取两个框更大的x、更大的y）
    int inter_x1 = std::max(a.x, b.x);
    int inter_y1 = std::max(a.y, b.y);

    // 步骤2：求相交区域右下角x、y（取两个框更小的右下角坐标）
    int inter_x2 = std::min(a.x + a.width, b.x + b.width);
    int inter_y2 = std::min(a.y + a.height, b.y + b.height);

    // 步骤3：计算相交矩形宽、高；如果宽/高小于0，代表无交集
    int inter_w = inter_x2 - inter_x1;
    int inter_h = inter_y2 - inter_y1;
    if (inter_w <= 0 || inter_h <= 0)
    {
        // 无重叠，IOU直接为0
        return 0.0f;
    }

    // 步骤4：相交面积
    int inter_area = inter_w * inter_h;

    // 步骤5：两个框各自总面积
    int area_a = a.width * a.height;
    int area_b = b.width * b.height;

    // 步骤6：并集面积 = A面积 + B面积 - 相交面积
    int union_area = area_a + area_b - inter_area;

    // 步骤7：IOU = 相交面积 / 并集面积，转浮点返回
    float iou = static_cast<float>(inter_area) / union_area;
    return iou;
}

// 批量保存掩码图片到data目录
void saveMask(const cv::Mat& mask, const std::string& path)
{
    // 找到最后一个分隔符 / 或 \,获取目录路径，创建目录（如果不存在）
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        std::string dir = path.substr(0, pos);
        // Windows API创建单层文件夹
        CreateDirectoryA(dir.c_str(), NULL);
    }

    bool ret = cv::imwrite(path, mask);
    if (!ret)
    {
        printf("掩码保存失败，路径：%s\n", path.c_str());
    }
}

// 测试主函数
/*int main()
{
    // 1. 创建空白画布 高600，宽800，3通道灰色背景
    cv::Mat canvas(600, 800, CV_8UC3, cv::Scalar(50, 50, 50));

    // 2. 定义两个重叠测试框，用来测试IOU计算
    cv::Rect box1(100, 120, 220, 260);
    cv::Rect box2(180, 160, 200, 240);

    // 3. 调用drawBox绘制两个框，不同颜色+标签
    drawBox(canvas, box1, "obj1 score:0.92", cv::Scalar(0, 255, 0));  // 绿色
    drawBox(canvas, box2, "obj2 score:0.88", cv::Scalar(0, 0, 255));  // 红色

    // 4. 计算并打印IOU
    float iou_val = calcIOU(box1, box2);
    printf("IOU of box1 and box2 = %.4f\n", iou_val);

    // 5. 生成掩码图像（单通道二值掩码）
    cv::Mat mask = cv::Mat::zeros(canvas.rows, canvas.cols, CV_8UC1);
    // 在掩码上把box1区域涂成白色(255)，代表前景
    cv::rectangle(mask, box1, cv::Scalar(255), -1);
    // 保存掩码到 ./data/mask_test.png
    std::string mask_path = "../../data/mask_test.png";
    saveMask(mask, mask_path);
    printf("Mask image saved to: %s\n", mask_path.c_str());

    // 6. 窗口显示绘制好框的画布
    cv::namedWindow("drawBox IOU Test", cv::WINDOW_AUTOSIZE);
    cv::imshow("drawBox IOU Test", canvas);

    // 7. 等待按键，防止窗口闪退
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}*/
