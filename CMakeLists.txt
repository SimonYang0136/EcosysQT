# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(MyQtApp LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 核心 Qt 配置 ---
# 开启 Qt 的自动化工具，它会自动处理 MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找我们需要的 Qt 模块 (Widgets 是必须的)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# --- 添加源文件 ---
# 定义可执行文件及其源文件
add_executable(MyQtApp
  main.cpp
  mainwindow.cpp
  mainwindow.h
  mainwindow.ui
)

# --- 链接 Qt 库 ---
# 将你的可执行文件与 Qt Widgets 库链接起来
target_link_libraries(MyQtApp PRIVATE Qt5::Widgets)