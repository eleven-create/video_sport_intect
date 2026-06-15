cmake_minimum_required(VERSION 3.10)
project(motion_detection)

set(CMAKE_CXX_STANDARD 11)

# 【备用】如果CMake找不到OpenCV，就取消下面注释，改成你自己的OpenCV路径
set(OpenCV_DIR "D:/opencv/build")

# 强制指定架构和运行时，绕开自动检测的问题
set(OpenCV_ARCH x64)
set(OpenCV_RUNTIME vc16)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

# 所有源文件统一列在这里，新增文件时补充即可
add_executable(motion_detection
    main.cpp
    src/preprocess.cpp
    src/detectors.cpp
    src/postprocess.cpp
    src/evaluation.cpp
    src/utils.cpp
)

target_link_libraries(motion_detection ${OpenCV_LIBS})