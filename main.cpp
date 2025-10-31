#include "Widget.h"
#include "TestBackend.h"  // 测试后端
// #include "RealBackend.h"  // 后端开发者实现真实后端后，取消注释这行

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // ========== 后端开发者只需修改这里 ==========
    // 使用测试后端
    TestBackend backend;
    
    // 后端开发完成后，替换为真实后端:
    // RealBackend backend;
    // ============================================
    
    // 创建前端窗口并传入后端接口
    Widget w(&backend);
    w.resize(1000, 700);
    w.setWindowTitle("生态模拟系统");
    w.show();
    
    return a.exec();
}