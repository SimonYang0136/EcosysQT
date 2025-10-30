/*
物种工厂类实现
实现物种创建的工厂模式，支持动态注册和创建物种实例
*/

#include "species_factory.h"
#include <stdexcept>

// 全局工厂实例定义
SpeciesFactory g_species_factory;

// 注册物种创建函数
void SpeciesFactory::register_species(const std::string& name, Creator creator_func) {
    creators[name] = creator_func;
}

// 根据名称创建物种实例
std::unique_ptr<Species> SpeciesFactory::create(const std::string& name, Position pos) {
    auto it = creators.find(name);
    if (it == creators.end()) {
        throw std::invalid_argument("Unknown species name: " + name);
    }
    return it->second(pos);
}

// 获取所有已注册物种的名称
std::vector<std::string> SpeciesFactory::get_all_species_names() const {
    std::vector<std::string> names;
    for (const auto& pair : creators) {
        names.push_back(pair.first);
    }
    return names;
}

// 检查物种是否已注册
bool SpeciesFactory::is_registered(const std::string& name) const {
    return creators.find(name) != creators.end();
}

// 清除所有注册的物种
void SpeciesFactory::clear() {
    creators.clear();
}

// 注册所有物种的函数实现
void register_all_species() {
    // 注册草
    g_species_factory.register_species("grass", [](Position pos) {
        return std::make_unique<Grass>(pos);
    });
    
    // 注册牛
    g_species_factory.register_species("cow", [](Position pos) {
        return std::make_unique<Cow>(pos);
    });
    
    // 注册老虎
    g_species_factory.register_species("tiger", [](Position pos) {
        return std::make_unique<Tiger>(pos);
    });
}