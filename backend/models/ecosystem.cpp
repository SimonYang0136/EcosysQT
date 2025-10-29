#include "ecosystem.h"
#include <random>
#include <algorithm>
#include <iostream>

// --- SpeciesStatistics ---
SpeciesStatistics::SpeciesStatistics() {
    statistics[SpeciesType::GRASS] = 0;
    statistics[SpeciesType::COW] = 0;
    statistics[SpeciesType::TIGER] = 0;
}
void SpeciesStatistics::increment(SpeciesType type, int count) {
    statistics[type] += count;
}
void SpeciesStatistics::set_count(SpeciesType type, int count) {
    statistics[type] = count;
}
int SpeciesStatistics::get_count(SpeciesType type) const {
    auto it = statistics.find(type);
    return it != statistics.end() ? it->second : 0;
}
void SpeciesStatistics::reset() {
    for (auto& kv : statistics) kv.second = 0;
}
int SpeciesStatistics::grass() const { return get_count(SpeciesType::GRASS); }
void SpeciesStatistics::set_grass(int value) { set_count(SpeciesType::GRASS, value); }
int SpeciesStatistics::cow() const { return get_count(SpeciesType::COW); }
void SpeciesStatistics::set_cow(int value) { set_count(SpeciesType::COW, value); }
int SpeciesStatistics::tiger() const { return get_count(SpeciesType::TIGER); }
void SpeciesStatistics::set_tiger(int value) { set_count(SpeciesType::TIGER, value); }

// --- SpeciesRegistry ---
SpeciesRegistry::SpeciesRegistry(const EcosystemConfig& config) {
    register_species("grass", std::make_shared<Grass>(Position{0,0}), config.initial_grass);
    register_species("cow", std::make_shared<Cow>(Position{0,0}), config.initial_cows);
    register_species("tiger", std::make_shared<Tiger>(Position{0,0}), config.initial_tigers);
}
void SpeciesRegistry::register_species(const std::string& name, std::shared_ptr<Species> prototype, int initial_count) {
    registry[name] = SpeciesInfo{name, {}, initial_count};
}
std::vector<std::shared_ptr<Species>>& SpeciesRegistry::get_species_list(const std::string& name) {
    return registry[name].list;
}
int SpeciesRegistry::get_initial_count(const std::string& name) const {
    auto it = registry.find(name);
    return it != registry.end() ? it->second.initial_count : 0;
}
std::vector<std::string> SpeciesRegistry::get_all_species_names() const {
    std::vector<std::string> names;
    for (const auto& kv : registry) names.push_back(kv.first);
    return names;
}
void SpeciesRegistry::add_individual(const std::string& name, std::shared_ptr<Species> individual) {
    registry[name].list.push_back(individual);
}
void SpeciesRegistry::extend_individuals(const std::string& name, const std::vector<std::shared_ptr<Species>>& individuals) {
    auto& list = registry[name].list;
    list.insert(list.end(), individuals.begin(), individuals.end());
}
void SpeciesRegistry::clear_species(const std::string& name) {
    registry[name].list.clear();
}
void SpeciesRegistry::clear_all() {
    for (auto& kv : registry) kv.second.list.clear();
}
int SpeciesRegistry::get_species_count(const std::string& name) const {
    auto it = registry.find(name);
    return it != registry.end() ? it->second.list.size() : 0;
}
int SpeciesRegistry::get_total_count() const {
    int sum = 0;
    for (const auto& kv : registry) sum += kv.second.list.size();
    return sum;
}
void SpeciesRegistry::filter_alive(const std::string& name) {
    auto& list = registry[name].list;
    list.erase(std::remove_if(list.begin(), list.end(),
        [](const std::shared_ptr<Species>& s){ return !s->alive; }), list.end());
}
void SpeciesRegistry::filter_all_alive() {
    for (auto& kv : registry) filter_alive(kv.first);
}

// --- EcosystemState ---
EcosystemState::EcosystemState(const EcosystemConfig& config)
    : config(config), time_step(0), species_registry(config), births(), deaths(), population_history() {
    initialize_populations();
}

void EcosystemState::initialize_populations() {
    for (const auto& name : species_registry.get_all_species_names()) {
        int initial_count = species_registry.get_initial_count(name);
        for (int i = 0; i < initial_count; ++i) {
            int x = rand() % config.world_width;
            int y = rand() % config.world_height;
            std::shared_ptr<Species> individual;
            if (name == "grass") individual = std::make_shared<Grass>(Position{(double)x, (double)y});
            else if (name == "cow") individual = std::make_shared<Cow>(Position{(double)x, (double)y});
            else if (name == "tiger") individual = std::make_shared<Tiger>(Position{(double)x, (double)y});
            species_registry.add_individual(name, individual);
        }
    }
}

EcosystemStateData EcosystemState::get_ecosystem_state() const {
    EcosystemStateData state;
    state.world_width = config.world_width;
    state.world_height = config.world_height;
    state.grass_list = species_registry.registry.at("grass").list;
    state.cow_list = species_registry.registry.at("cow").list;
    state.tiger_list = species_registry.registry.at("tiger").list;
    state.time_step = time_step;

    // Precompute grass positions and alive objects
    state.grass_positions_array.clear();
    state.alive_grass_objects.clear();
    for (const auto& grass : state.grass_list) {
        if (grass->alive) {
            state.grass_positions_array.push_back(grass->position);
            state.alive_grass_objects.push_back(grass);
        }
    }
    return state;
}

void EcosystemState::update_species(const EcosystemStateData& state) {
    for (const auto& name : species_registry.get_all_species_names()) {
        auto& list = species_registry.get_species_list(name);
        for (auto& individual : list) {
            individual->update(state);
        }
    }
}

void EcosystemState::handle_reproduction() {
    EcosystemStateData state = get_ecosystem_state();
    for (const auto& name : species_registry.get_all_species_names()) {
        auto& list = species_registry.get_species_list(name);
        std::vector<std::shared_ptr<Species>> new_individuals;
        for (auto& individual : list) {
            if (individual->can_reproduce()) {
                auto offspring = individual->reproduce(state);
                if (offspring) new_individuals.push_back(std::move(offspring));
            }
        }
        species_registry.extend_individuals(name, new_individuals);
        SpeciesType type = name == "grass" ? SpeciesType::GRASS : (name == "cow" ? SpeciesType::COW : SpeciesType::TIGER);
        births.increment(type, new_individuals.size());
        if (!new_individuals.empty()) {
            std::cout << (name == "grass" ? "🌱" : name == "cow" ? "🐄" : "🐅")
                      << " " << new_individuals.size() << " new " << name << " individuals born\n";
        }
    }
}

void EcosystemState::update_statistics() {
    SpeciesStatistics stats = get_species_counts();
    std::map<SpeciesType, int> snapshot = stats.statistics;
    population_history.push_back(snapshot);
    if (population_history.size() > 100)
        population_history.erase(population_history.begin(), population_history.end() - 100);
}

void EcosystemState::cleanup_dead() {
    for (const auto& name : species_registry.get_all_species_names()) {
        auto& list = species_registry.get_species_list(name);
        int dead_count = std::count_if(list.begin(), list.end(),
            [](const std::shared_ptr<Species>& s){ return !s->alive; });
        SpeciesType type = name == "grass" ? SpeciesType::GRASS : (name == "cow" ? SpeciesType::COW : SpeciesType::TIGER);
        deaths.increment(type, dead_count);
        species_registry.filter_alive(name);
        if (dead_count > 0) {
            std::cout << "💀 " << dead_count << " " << name << " individuals died\n";
        }
    }
}

SpeciesStatistics EcosystemState::get_species_counts() const {
    SpeciesStatistics stats;
    for (const auto& name : species_registry.get_all_species_names()) {
        SpeciesType type = name == "grass" ? SpeciesType::GRASS : (name == "cow" ? SpeciesType::COW : SpeciesType::TIGER);
        int count = species_registry.get_species_count(name);
        stats.set_count(type, count);
    }
    return stats;
}

SpeciesPopulationData EcosystemState::get_species_data() const {
    SpeciesPopulationData data;
    for (const auto& name : species_registry.get_all_species_names()) {
        auto& list = species_registry.get_species_list(name);
        std::vector<BaseIndividualData> individuals;
        for (const auto& individual : list) {
            if (individual->alive) {
                BaseIndividualData ind;
                ind.id = reinterpret_cast<std::uintptr_t>(individual.get());
                ind.position = PositionData{individual->position.x, individual->position.y};
                ind.energy = individual->energy;
                ind.age = individual->age;
                ind.alive = individual->alive;
                ind.max_energy = individual->max_energy;
                individuals.push_back(ind);
            }
        }
        data.species_data[name] = individuals;
    }
    return data;
}

void EcosystemState::reset(const EcosystemConfig& new_config) {
    config = new_config;
    time_step = 0;
    species_registry.clear_all();
    births.reset();
    deaths.reset();
    population_history.clear();
    initialize_populations();
}

std::vector<std::string> EcosystemState::check_extinction() const {
    std::vector<std::string> extinct;
    for (const auto& name : species_registry.get_all_species_names()) {
        if (species_registry.get_species_count(name) == 0)
            extinct.push_back(name);
    }
    return extinct;
}