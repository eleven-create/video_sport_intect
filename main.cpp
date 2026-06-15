#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::Mat img = cv::Mat::zeros(500, 800, CV_8UC3);
    cv::circle(img, cv::Point(400, 250), 120, cv::Scalar(0, 255, 0), 3);

    cv::imshow("OpenCV Test", img);
    std::cout << "OpenCV 版本: " << CV_VERSION << std::endl;

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}