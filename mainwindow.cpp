#include "mainwindow.h"
#include "ui_mainwindow.h" // 这个文件由 CMAKE_AUTOUIC 自动生成

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}