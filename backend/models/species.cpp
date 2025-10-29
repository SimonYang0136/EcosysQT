#include "species.h"
#include <random>
#include <algorithm>
#include <cmath>

// --- Species ---
Species::Species(Position pos, double energy, int max_age, double reproduction_energy_cost)
    : position(pos), energy(reproduction_energy_cost), max_energy(energy * 4), age(0), max_age(max_age),
      alive(true), reproduction_cooldown(0), death_reason(""), species_name("Species"),
      reproduction_energy_cost(reproduction_energy_cost) {}

void Species::update(const EcosystemStateData& ecosystem_state) {
    if (!alive) return;
    if (reproduction_cooldown > 0) reproduction_cooldown -= 1;
}

bool Species::can_reproduce() const {
    return alive && energy >= reproduction_energy_cost * 2 && reproduction_cooldown <= 0;
}

std::unique_ptr<Species> Species::reproduce(const EcosystemStateData& ecosystem_state) {
    return nullptr;
}

void Species::move_randomly(int world_width, int world_height, double speed) {
    if (!alive) return;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> angle_dist(0, 2 * M_PI);
    double angle = angle_dist(gen);
    double dx = std::cos(angle) * speed;
    double dy = std::sin(angle) * speed;
    position.x = std::max(0.0, std::min((double)world_width, position.x + dx));
    position.y = std::max(0.0, std::min((double)world_height, position.y + dy));
}

void Species::age_one_step() {
    age += 1;
    if (age >= max_age) die_from_old_age();
}

void Species::die(const std::string& reason) {
    if (alive) {
        alive = false;
        death_reason = reason;
    }
}

void Species::die_from_old_age() { die("Old age"); }
void Species::die_from_starvation() { die("Starvation"); }
void Species::die_from_predation(const std::string& predator_name) { die("Predation by " + predator_name); }

// --- Animal ---
Animal::Animal(Position pos, double energy, int max_age, double reproduction_energy_cost,
               double movement_speed, int energy_consumption, double hunting_range,
               double hunting_success_rate, double detection_range,
               std::vector<std::string> food_types, int hunting_cooldown_duration)
    : Species(pos, energy, max_age, reproduction_energy_cost),
      movement_speed(movement_speed), energy_consumption(energy_consumption),
      hunting_range(hunting_range), hunting_success_rate(hunting_success_rate),
      detection_range(detection_range), food_types(food_types),
      hunting_cooldown(0), hunting_cooldown_duration(hunting_cooldown_duration) {}

std::optional<Position> Animal::find_nearest_food(const EcosystemStateData& ecosystem_state) {
    std::optional<Position> nearest_food;
    double min_distance = std::numeric_limits<double>::max();

    for (const auto& food_type : food_types) {
        const std::vector<std::shared_ptr<Species>>* food_list = nullptr;
        if (food_type == "grass") food_list = &ecosystem_state.grass_list;
        else if (food_type == "cow") food_list = &ecosystem_state.cow_list;
        else if (food_type == "tiger") food_list = &ecosystem_state.tiger_list;
        else continue;

        for (const auto& food : *food_list) {
            if (food->alive) {
                double distance = position.distance_to(food->position);
                if (distance <= detection_range && distance < min_distance) {
                    min_distance = distance;
                    nearest_food = food->position;
                }
            }
        }
    }
    return nearest_food;
}

void Animal::move_towards_target(const Position& target_position, int world_width, int world_height) {
    if (!alive) return;
    double dx = target_position.x - position.x;
    double dy = target_position.y - position.y;
    double distance = std::sqrt(dx * dx + dy * dy);

    if (distance > 0) {
        dx = (dx / distance) * movement_speed;
        dy = (dy / distance) * movement_speed;
        position.x = std::max(0.0, std::min((double)world_width, position.x + dx));
        position.y = std::max(0.0, std::min((double)world_height, position.y + dy));
    }
}

void Animal::intelligent_move(const EcosystemStateData& ecosystem_state) {
    if (!alive) return;
    if (hunting_cooldown > 0) {
        hunting_cooldown -= 1;
        return;
    }
    int world_width = ecosystem_state.world_width;
    int world_height = ecosystem_state.world_height;
    auto nearest_food = find_nearest_food(ecosystem_state);
    if (nearest_food.has_value()) {
        move_towards_target(nearest_food.value(), world_width, world_height);
    } else {
        move_randomly(world_width, world_height, movement_speed);
    }
}

void Animal::start_hunting_cooldown() {
    hunting_cooldown = hunting_cooldown_duration;
}

// --- Grass ---
Grass::Grass(Position pos)
    : Species(pos, 40, 2000, 40),
      base_growth_rate(0.9), reproduction_chance(0.4),
      competition_radius(30.0), max_competition_effect(0.9) {}

double Grass::calculate_nearby_grass_density_optimized(const EcosystemStateData& ecosystem_state) {
    // 这里只实现简单逻辑，实际可用 Eigen 或其他库优化
    const auto& positions = ecosystem_state.grass_positions_array;
    const auto& alive_grass_objects = ecosystem_state.alive_grass_objects;
    if (positions.empty()) return 0.0;

    // 找到自身索引
    int self_index = -1;
    for (size_t i = 0; i < alive_grass_objects.size(); ++i) {
        if (alive_grass_objects[i].get() == this) {
            self_index = static_cast<int>(i);
            break;
        }
    }
    // 排除自身
    std::vector<Position> filtered_positions;
    for (size_t i = 0; i < positions.size(); ++i) {
        if ((int)i != self_index) filtered_positions.push_back(positions[i]);
    }
    if (filtered_positions.empty()) return 0.0;

    // 计算距离
    int nearby_grass_count = 0;
    for (const auto& pos : filtered_positions) {
        double dist = position.distance_to(pos);
        if (dist <= competition_radius) nearby_grass_count++;
    }
    double max_possible_grass = M_PI * (competition_radius * competition_radius) / 400.0;
    double density = std::min(1.0, nearby_grass_count / max_possible_grass);
    return density;
}

double Grass::calculate_nearby_grass_density(const EcosystemStateData& ecosystem_state) {
    // 优先用优化版
    if (!ecosystem_state.grass_positions_array.empty())
        return calculate_nearby_grass_density_optimized(ecosystem_state);

    // Fallback
    const auto& grass_list = ecosystem_state.grass_list;
    if (grass_list.empty()) return 0.0;

    std::vector<Position> alive_grass_positions;
    for (const auto& grass : grass_list) {
        if (grass.get() != this && grass->alive)
            alive_grass_positions.push_back(grass->position);
    }
    if (alive_grass_positions.empty()) return 0.0;

    int nearby_grass_count = 0;
    for (const auto& pos : alive_grass_positions) {
        double dist = position.distance_to(pos);
        if (dist <= competition_radius) nearby_grass_count++;
    }
    double max_possible_grass = M_PI * (competition_radius * competition_radius) / 100.0;
    double density = std::min(1.0, nearby_grass_count / max_possible_grass);
    return density;
}

double Grass::get_competition_adjusted_growth_rate(const EcosystemStateData& ecosystem_state) {
    double density = calculate_nearby_grass_density(ecosystem_state);
    double competition_factor = 1.0 - (std::pow(density, 0.3) * max_competition_effect);
    if (density == 0) competition_factor = 2.0;
    double adjusted_growth_rate = base_growth_rate * competition_factor;
    double min_growth_rate = base_growth_rate * 0.01;
    return std::max(min_growth_rate, adjusted_growth_rate);
}

void Grass::update(const EcosystemStateData& ecosystem_state) {
    Species::update(ecosystem_state);
    if (!alive) return;
    double adjusted_growth_rate = get_competition_adjusted_growth_rate(ecosystem_state);
    energy = std::min(max_energy, energy + adjusted_growth_rate);
    age_one_step();
    if (age >= max_age) die();
}

bool Grass::can_reproduce() const {
    return Species::can_reproduce() && (static_cast<double>(rand()) / RAND_MAX < reproduction_chance);
}

std::unique_ptr<Species> Grass::reproduce(const EcosystemStateData& ecosystem_state) {
    Species::reproduce(ecosystem_state);
    if (!can_reproduce()) return nullptr;
    int world_width = ecosystem_state.world_width;
    int world_height = ecosystem_state.world_height;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist_x(-200, 200);
    std::uniform_real_distribution<> dist_y(-200, 200);

    double new_x = std::max(0.0, std::min((double)world_width, position.x + dist_x(gen)));
    double new_y = std::max(0.0, std::min((double)world_height, position.y + dist_y(gen)));
    if (new_x <= 0 || new_x >= world_width || new_y <= 0 || new_y >= world_height) return nullptr;

    energy -= reproduction_energy_cost;
    reproduction_cooldown = 10;
    Position new_position{new_x, new_y};
    return std::make_unique<Grass>(new_position);
}

// --- Cow ---
Cow::Cow(Position pos)
    : Animal(pos, 400, 4000, 400, 3.0, 2, 5.0, 1.0, 800.0, {"grass"}, 0),
      eating_range(5.0) {}

void Cow::update(const EcosystemStateData& ecosystem_state) {
    Animal::update(ecosystem_state);
    if (!alive) return;
    intelligent_move(ecosystem_state);
    energy -= energy_consumption;
    // Eat grass
    std::vector<Grass*> grass_ptrs;
    for (const auto& grass : ecosystem_state.grass_list) {
        Grass* g = dynamic_cast<Grass*>(grass.get());
        if (g) grass_ptrs.push_back(g);
    }
    _eat_grass(grass_ptrs);
    age_one_step();
    if (energy <= 0) die_from_starvation();
}

void Cow::_eat_grass(const std::vector<Grass*>& grass_list) {
    for (auto* grass : grass_list) {
        if (grass->alive && position.distance_to(grass->position) <= eating_range) {
            energy = std::min(max_energy, energy + grass->energy);
            grass->die_from_predation("Cow");
            break;
        }
    }
}

bool Cow::can_reproduce() const {
    return Animal::can_reproduce() && age > 20;
}

std::unique_ptr<Species> Cow::reproduce(const EcosystemStateData& ecosystem_state) {
    Animal::reproduce(ecosystem_state);
    if (!can_reproduce()) return nullptr;
    energy -= reproduction_energy_cost;
    reproduction_cooldown = 200;
    int world_width = ecosystem_state.world_width;
    int world_height = ecosystem_state.world_height;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist_x(-10, 10);
    std::uniform_real_distribution<> dist_y(-10, 10);

    double new_x = std::max(0.0, std::min((double)world_width, position.x + dist_x(gen)));
    double new_y = std::max(0.0, std::min((double)world_height, position.y + dist_y(gen)));
    Position new_position{new_x, new_y};
    return std::make_unique<Cow>(new_position);
}

// --- Tiger ---
Tiger::Tiger(Position pos)
    : Animal(pos, 4000, 8000, 4000, 4.0, 20, 6.0, 0.2, 1000.0, {"cow"}, 4) {}

void Tiger::update(const EcosystemStateData& ecosystem_state) {
    Animal::update(ecosystem_state);
    if (energy <= reproduction_energy_cost / 3) {
        hunting_success_rate = 0.2 + 0.6 * (1.0 - age / (double)max_age);
    } else {
        hunting_success_rate = 0.2;
    }
    if (!alive) return;
    intelligent_move(ecosystem_state);
    energy -= energy_consumption;
    // Hunt cows
    std::vector<Cow*> cow_ptrs;
    for (const auto& cow : ecosystem_state.cow_list) {
        Cow* c = dynamic_cast<Cow*>(cow.get());
        if (c) cow_ptrs.push_back(c);
    }
    _hunt_cows(cow_ptrs);
    age_one_step();
    if (energy <= 0) die_from_starvation();
}

void Tiger::_hunt_cows(const std::vector<Cow*>& cow_list) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> hunt_dist(0.0, 1.0);

    for (auto* cow : cow_list) {
        if (cow->alive && position.distance_to(cow->position) <= hunting_range) {
            if (hunt_dist(gen) < hunting_success_rate) {
                energy = std::min(max_energy, energy + cow->energy);
                cow->die_from_predation("Tiger");
                start_hunting_cooldown();
                break;
            }
        }
    }
}

bool Tiger::can_reproduce() const {
    return Animal::can_reproduce() && age > 30;
}

std::unique_ptr<Species> Tiger::reproduce(const EcosystemStateData& ecosystem_state) {
    Animal::reproduce(ecosystem_state);
    if (!can_reproduce()) return nullptr;
    energy -= reproduction_energy_cost;
    reproduction_cooldown = 800;
    int world_width = ecosystem_state.world_width;
    int world_height = ecosystem_state.world_height;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist_x(-40, 40);
    std::uniform_real_distribution<> dist_y(-40, 40);

    double new_x = std::max(0.0, std::min((double)world_width, position.x + dist_x(gen)));
    double new_y = std::max(0.0, std::min((double)world_height, position.y + dist_y(gen)));
    Position new_position{new_x, new_y};
    return std::make_unique<Tiger>(new_position);
}
