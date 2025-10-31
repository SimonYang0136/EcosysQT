#include "TestBackend.h"
#include <QDebug>

TestBackend::TestBackend()
    : m_frameCount(0)
{
    initializeAnimals();
    qDebug() << "测试后端初始化完成";
}

void TestBackend::initializeAnimals()
{
    auto random = QRandomGenerator::global();
    
    // 生成食草动物
    for (int i = 0; i < 10; ++i) {
        Animal a;
        a.x = random->bounded(-80, 80);  // 使用整数
        a.y = random->bounded(-80, 80);
        a.vx = (random->bounded(-50, 50)) / 10.0f;  // 转换为 float
        a.vy = (random->bounded(-50, 50)) / 10.0f;
        a.type = SpeciesType::Herbivore;
        m_animals.append(a);
    }
    
    // 生成食肉动物
    for (int i = 0; i < 5; ++i) {
        Animal a;
        a.x = random->bounded(-80, 80);
        a.y = random->bounded(-80, 80);
        a.vx = (random->bounded(-30, 30)) / 10.0f;
        a.vy = (random->bounded(-30, 30)) / 10.0f;
        a.type = SpeciesType::Carnivore;
        m_animals.append(a);
    }
    
    // 生成杂食动物
    for (int i = 0; i < 7; ++i) {
        Animal a;
        a.x = random->bounded(-80, 80);
        a.y = random->bounded(-80, 80);
        a.vx = (random->bounded(-40, 40)) / 10.0f;
        a.vy = (random->bounded(-40, 40)) / 10.0f;
        a.type = SpeciesType::Omnivore;
        m_animals.append(a);
    }
}

void TestBackend::updateAnimals()
{
    // 简单的移动逻辑
    for (Animal &a : m_animals) {
        // 更新位置
        a.x += a.vx;
        a.y += a.vy;
        
        // 边界反弹
        if (a.x < -100 || a.x > 100) {
            a.vx = -a.vx;
            a.x = qBound(-100.0f, a.x, 100.0f);
        }
        if (a.y < -100 || a.y > 100) {
            a.vy = -a.vy;
            a.y = qBound(-100.0f, a.y, 100.0f);
        }
    }
}

DataPacket TestBackend::getNextFrame()
{
    m_frameCount++;
    updateAnimals();
    
    DataPacket packet;
    
    // 添加草 (随机生成，不移动)
    auto random = QRandomGenerator::global();
    int numGrass = random->bounded(30, 50);
    for (int i = 0; i < numGrass; ++i) {
        float x = random->bounded(-100, 100);  // 整数转 float
        float y = random->bounded(-100, 100);
        packet.append(DataItem(x, y, SpeciesType::Grass));
    }
    
    // 添加移动的动物
    for (const Animal &a : m_animals) {
        packet.append(DataItem(a.x, a.y, a.type));
    }
    
    qDebug() << "帧" << m_frameCount << ": 生成了" << packet.size() << "个数据项";
    
    return packet;
}