cmake_minimum_required(VERSION 3.10)
project(motion_detection)
add_compile_options(/utf-8)
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
# 1. 核心基类与配置 (如果你们采用了 Header-only，这里可以留空；若写了cpp则取消注释)
set(CORE_SRC
    src/core/config.cpp
    src/core/base_detector.cpp
)

# 2. 核心算法层 (原 detectors.cpp 拆分后的产物)
set(ALGORITHM_SRC
    src/algorithms/frame_diff.cpp        # 成员A: 帧差法
    src/algorithms/bg_subtractor.cpp     # 成员A: MOG2 / KNN
    src/algorithms/optical_flow.cpp      # 成员A: LK 光流法
    src/algorithms/hybrid_fusion.cpp     # A/B协作: 复合检测融合层
)

# 3. 外围处理与工程展现层
set(MODULE_SRC
    src/preprocess.cpp                   # 预处理
    src/postprocess.cpp                  # 后处理 (形态学, NMS等)
    src/evaluation.cpp                   # 量化评估 (PR曲线, IoU计算)
    src/ui_viewer.cpp                    # UI面板与多窗口渲染
)

# ------------------------------------------------------------------
# [生成可执行文件]
# ------------------------------------------------------------------

# 将所有分组的源文件和 main.cpp 一起编译
add_executable(motion_detection
    main.cpp
    ${CORE_SRC}
    ${ALGORITHM_SRC}
    ${MODULE_SRC}
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