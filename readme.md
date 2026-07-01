# 🏃‍♂️ 视频运动目标检测与评估系统 (Video Moving Object Detection)

本项目是一个基于 **C++11** 和 **OpenCV 4.x** 开发的面向对象视频运动目标检测系统。项目不仅实现了多种经典的计算机视觉基线算法，还创新性地提出了一种**时空增强融合算法（Hybrid Fusion）**，能够有效克服动态阴影、行人遮挡等复杂场景下的干扰，并在 CDnet 2014 标准数据集上取得了优异的量化评估成绩。

---

## 🌟 核心特性与亮点

1. **面向对象架构 (OOD)**：设计了统一的纯虚基类 `BaseDetector`，利用多态机制管理所有检测算法，实现了高内聚、低耦合的接口调用。
2. **多算法横向对比**：独立封装了三帧差分法、KNN 背景建模、MOG2 混合高斯模型以及 LK 稀疏光流法等基线算法。
3. **时空增强融合 (Hybrid Fusion)**：
   - **空间域**：利用 MOG2 提取细粒度的初始前景掩码。
   - **时序域**：利用 LK 稀疏光流追踪特征点，通过运动一致性校验剔除动态背景与阴影。
4. **鲁棒的后处理引擎**：集成了形态学去噪、连通域分析，以及**非极大值抑制 (NMS)** 去除冗余边界框。
5. **实时量化评估系统**：内置基于 CDnet 2014 标准的像素级评估器，可实时输出 TP、FP、FN 及 F1-Score。
6. **多窗口交互 UI**：支持运行时的参数热切换与多算法（MOG2 vs KNN vs Fusion）同屏可视化对比。

---

## 📂 工程目录结构

本项目采用了标准的 C++ 模块化目录结构：

```text
project/
├── CMakeLists.txt          # CMake 构建脚本 (已配置 DLL 自动拷贝机制)
├── main.cpp                # 程序入口点
├── include/                # 头文件目录
│   ├── core/               # 核心基类与配置 (BaseDetector, Config)
│   ├── algorithms/         # 具体检测算法头文件 (MOG2, LK, HybridFusion等)
│   ├── preprocess.h        # 数据预处理
│   ├── postprocess.h       # 后处理与 NMS
│   ├── evaluation.h        # 性能与量化评估
│   └── ui_viewer.h         # 多窗口交互与渲染
└── src/                    # 源文件目录
    ├── core/               # 核心接口实现
    ├── algorithms/         # 具体检测算法实现
    ├── preprocess.cpp
    ├── postprocess.cpp
    ├── evaluation.cpp
    └── ui_viewer.cpp
```

---

## 🛠️ 环境依赖

*   **操作系统**：Windows 10 / 11 (建议 x64 架构)
*   **编译器**：MSVC (Visual Studio 2019 / 2022) 或 MinGW
*   **构建工具**：CMake (VERSION $\ge$ 3.10)
*   **核心库**：**OpenCV 4.x** (建议 4.5.0 及以上版本)

---

## 🚀 编译与运行指南

本项目推荐使用 **CMake** 进行构建。

### 1. 配置 OpenCV 路径
在编译前，请确保您的电脑上已安装 OpenCV，并修改 `CMakeLists.txt` 中的 OpenCV 路径为您本机的实际路径：
```cmake
set(OpenCV_DIR "D:/opencv/build")
set(OPENCV_BIN_ROOT "D:/opencv/build/x64/vc16/bin")
```

### 2. CMake 构建步骤 (命令行方式)
在项目根目录（即 `CMakeLists.txt` 所在目录）下打开终端，依次执行：
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 3. 使用 IDE 运行 (如 VS Code 或 Visual Studio)
*   **VS Code**：安装 `CMake Tools` 插件后，点击底部状态栏的 `Build` 即可自动完成编译。
*   **Visual Studio**：直接通过 `文件 -> 打开 -> CMake` 选择项目根目录，VS 会自动解析并构建。

### 4. 运行说明
编译成功后，将在 `build` 或相应的输出目录生成 `motion_detection.exe`。
> **💡 注**：本项目的 `CMakeLists.txt` 中已内置了 **POST_BUILD 脚本**，在编译完成后会自动将 OpenCV 运行所需的 `.dll` 文件拷贝至可执行文件同级目录，彻底解决了运行时提示“找不到 opencv_world4xx.dll”的问题。

双击运行 `motion_detection.exe`，即可开启多窗口对比与实时评估之旅！