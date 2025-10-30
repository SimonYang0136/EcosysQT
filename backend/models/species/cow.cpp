/*
物种数据模型 - Cow 类实现
定义生态系统中的牛类，实现初级消费者逻辑
*/

#include "species.h"
#include "ecosystem.h"
#include <random>
#include <algorithm>
#include <cmath>

// --- Cow ---
// 牛类 - 初级消费者
Cow::Cow(Position pos)
    : Animal(pos,                 // 初始位置
             400,                 // 初始能量
             4000,                // 最大能量
             400,                 // 繁殖能量阈值
             3.0,                 // 移动速度
             2,                   // 繁殖冷却时间
             5.0,                 // 繁殖能量消耗
             1.0,                 // 基础能量消耗
             800.0,               // 寿命上限
             {"grass"},           // 可食用物种列表
             0),                  // 初始年龄
      eating_range(5.0)           // 吃草范围（米）
{}

void Cow::update(const EcosystemState& ecosystem_state) {
    // 更新牛的状态
    Animal::update(ecosystem_state);
    if (!alive) return;
    intelligent_move(ecosystem_state);
    energy -= energy_consumption;
    
    // 使用通用查询接口寻找附近的草
    auto grass_in_range = ecosystem_state.get_species_in_range("grass", position, eating_range);
    
    // 吃草
    for (const auto& grass : grass_in_range) {
        if (grass->alive) {
            energy = std::min(max_energy, energy + grass->energy);
            grass->die_from_predation("Cow");
            break;
        }
    }
    
    if (energy <= 0) die_from_starvation();
}

void Cow::_eat_grass(const std::vector<Grass*>& grass_list) {
    // 吃草
    for (auto* grass : grass_list) {
        if (grass->alive && position.distance_to(grass->position) <= eating_range) {
            energy = std::min(max_energy, energy + grass->energy);
            grass->die_from_predation("Cow");
            break;
        }
    }
}

bool Cow::can_reproduce() const {
    // Check if can reproduce
    return Animal::can_reproduce() && age > 20;
}

std::unique_ptr<Species> Cow::reproduce(const EcosystemState& ecosystem_state) {
    // 繁殖以创建新牛
    Animal::reproduce(ecosystem_state);
    if (!can_reproduce()) return nullptr;
    energy -= reproduction_energy_cost;
    reproduction_cooldown = 200;
    const auto& ecosystem_data = ecosystem_state.get_ecosystem_state();
    int world_width = ecosystem_data.world_width;
    int world_height = ecosystem_data.world_height;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist_x(-10, 10);
    std::uniform_real_distribution<> dist_y(-10, 10);

    double new_x = std::max(0.0, std::min((double)world_width, position.x + dist_x(gen)));
    double new_y = std::max(0.0, std::min((double)world_height, position.y + dist_y(gen)));
    Position new_position{new_x, new_y};
    return std::make_unique<Cow>(new_position);
}