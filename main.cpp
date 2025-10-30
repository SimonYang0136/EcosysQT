#include <QApplication>
#include "mainwindow.h"
#include "backend/include/species_factory.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 注册所有物种到工厂
    register_all_species();
    
    MainWindow w;
    w.show();
    return a.exec();
}