#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <optional>
#include "ecosystem.h"

// Position struct representing coordinates in 2D space
struct Position {
    double x;
    double y;
    // Calculate Euclidean distance to another position
    double distance_to(const Position& other) const {
        return std::sqrt((x - other.x)*(x - other.x) + (y - other.y)*(y - other.y));
    }
};

// Base class for all species in the ecosystem
class Species {
public:
    Position position;
    double energy;
    double max_energy;
    int age;
    int max_age;
    bool alive;
    int reproduction_cooldown;
    std::string death_reason;
    std::string species_name;
    double reproduction_energy_cost;

    // Constructor
    Species(Position pos, double energy = 100, int max_age = 100, double reproduction_energy_cost = 50);

    // Update species state (virtual for polymorphism)
    virtual void update(const EcosystemStateData& ecosystem_state);
    // Check if species can reproduce
    virtual bool can_reproduce() const;
    // Reproduce to create new individual
    virtual std::unique_ptr<Species> reproduce(const EcosystemStateData& ecosystem_state);
    // Move randomly within world boundaries
    virtual void move_randomly(int world_width, int world_height, double speed = 1.0);
    // Increase age by one step
    virtual void age_one_step();
    // Mark as dead with reason
    virtual void die(const std::string& reason = "Unknown");
    // Mark as dead due to old age
    virtual void die_from_old_age();
    // Mark as dead due to starvation
    virtual void die_from_starvation();
    // Mark as dead due to predation
    virtual void die_from_predation(const std::string& predator_name);
    // Virtual destructor for safe polymorphic deletion
    virtual ~Species() = default;
};

// Animal class, derived from Species, adds movement and hunting logic
class Animal : public Species {
public:
    double movement_speed;
    int energy_consumption;
    double hunting_range;
    double hunting_success_rate;
    double detection_range;
    std::vector<std::string> food_types;
    int hunting_cooldown;
    int hunting_cooldown_duration;

    Animal(Position pos, double energy = 100, int max_age = 100, double reproduction_energy_cost = 50,
           double movement_speed = 1.0, int energy_consumption = 1, double hunting_range = 5.0,
           double hunting_success_rate = 0.5, double detection_range = 500.0,
           std::vector<std::string> food_types = {}, int hunting_cooldown_duration = 0);

    // Find nearest food source
    virtual std::optional<Position> find_nearest_food(const EcosystemStateData& ecosystem_state);
    // Move towards target position
    void move_towards_target(const Position& target_position, int world_width, int world_height);
    // Intelligent movement: move towards food or randomly
    virtual void intelligent_move(const EcosystemStateData& ecosystem_state);
    // Start hunting cooldown
    void start_hunting_cooldown();
};

// Grass class, derived from Species, implements producer logic
class Grass : public Species {
public:
    double base_growth_rate;
    double reproduction_chance;
    double competition_radius;
    double max_competition_effect;

    Grass(Position pos);

    // Calculate nearby grass density (optimized)
    double calculate_nearby_grass_density_optimized(const EcosystemStateData& ecosystem_state);
    // Calculate nearby grass density (fallback)
    double calculate_nearby_grass_density(const EcosystemStateData& ecosystem_state);
    // Get growth rate adjusted for competition
    double get_competition_adjusted_growth_rate(const EcosystemStateData& ecosystem_state);
    // Update grass state
    void update(const EcosystemStateData& ecosystem_state) override;
    // Check if grass can reproduce
    bool can_reproduce() const override;
    // Reproduce to create new grass
    std::unique_ptr<Species> reproduce(const EcosystemStateData& ecosystem_state) override;
};

// Cow class, derived from Animal, implements primary consumer logic
class Cow : public Animal {
public:
    double eating_range;

    Cow(Position pos);

    // Update cow state
    void update(const EcosystemStateData& ecosystem_state) override;
    // Eat grass from list
    void _eat_grass(const std::vector<Grass*>& grass_list);
    // Check if cow can reproduce
    bool can_reproduce() const override;
    // Reproduce to create new cow
    std::unique_ptr<Species> reproduce(const EcosystemStateData& ecosystem_state) override;
};

// Tiger class, derived from Animal, implements secondary consumer logic
class Tiger : public Animal {
public:
    Tiger(Position pos);

    // Update tiger state
    void update(const EcosystemStateData& ecosystem_state) override;
    // Hunt cows from list
    void _hunt_cows(const std::vector<Cow*>& cow_list);
    // Check if tiger can reproduce
    bool can_reproduce() const override;
    // Reproduce to create new tiger
    std::unique_ptr<Species> reproduce(const EcosystemStateData& ecosystem_state) override;
};