cmake_minimum_required(VERSION 3.10)
project(motion_detection)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(/utf-8)


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

# ===================== 新增核心：自动拷贝dll（永久解决dll加载失败） =====================
# OpenCV bin目录，固定绝对路径，不受项目文件夹改名影响
set(OPENCV_BIN_ROOT "D:/opencv/build/x64/vc16/bin")

# Debug模式：复制所有带d后缀的Debug专用dll（包含parallel系列）
file(GLOB DEBUG_DLL_FILES "${OPENCV_BIN_ROOT}/*d.dll")
add_custom_command(TARGET motion_detection POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEBUG_DLL_FILES}
    $<TARGET_FILE_DIR:motion_detection>
)

# Release模式：复制Release无d后缀dll
file(GLOB RELEASE_DLL_FILES "${OPENCV_BIN_ROOT}/opencv_world*.dll")
add_custom_command(TARGET motion_detection POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RELEASE_DLL_FILES}
    $<TARGET_FILE_DIR:motion_detection>
)