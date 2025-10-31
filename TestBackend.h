#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include "IBackendInterface.h"
#include <QRandomGenerator>

/**
 * @brief 测试用后端实现
 * 
 * 这是一个简单的测试后端，生成随机移动的生物
 * 真实的后端实现应该包含生态模拟逻辑
 */
class TestBackend : public IBackendInterface
{
public:
    TestBackend();
    
    DataPacket getNextFrame() override;

private:
    // 用于演示的简单数据
    struct Animal {
        float x, y;
        float vx, vy;  // 速度
        SpeciesType type;
    };
    
    QList<Animal> m_animals;
    int m_frameCount;
    
    void initializeAnimals();
    void updateAnimals();
};

#endif // TESTBACKEND_H