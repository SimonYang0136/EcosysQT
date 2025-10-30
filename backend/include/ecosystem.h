/*
Ecosystem Data Model
Manages the entire ecosystem state and data (C++ migration)
*/

// #pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <optional>
#include "species.h"
#include "utils.h"


// Enum for species type (for statistics, registry, etc.)
enum class SpeciesType {
    GRASS,
    COW,
    TIGER
};

// Mapping functions between species type and string
SpeciesType species_type_from_name(const std::string& name);
std::string name_from_species_type(SpeciesType type);

// Position data for serialization/statistics (frontend use)
struct PositionData {
    double x;
    double y;
};

// Individual data for serialization/statistics (frontend use)
struct BaseIndividualData {
    int id;
    PositionData position;
    double energy;
    int age;
    bool alive;
    std::optional<double> max_energy;
};

// Population data for frontend/statistics
struct SpeciesPopulationData {
    std::map<std::string, std::vector<BaseIndividualData>> species_data;
};

// Statistics for each species (for population tracking)
class SpeciesStatistics {
public:
    std::map<SpeciesType, int> statistics;

    SpeciesStatistics();
    void increment(SpeciesType type, int count = 1);
    void set_count(SpeciesType type, int count);
    int get_count(SpeciesType type) const;
    void reset();

    // Property-like accessors for compatibility
    int grass() const;
    void set_grass(int value);
    int cow() const;
    void set_cow(int value);
    int tiger() const;
    void set_tiger(int value);
};



// Registry for all species types and individuals
class SpeciesRegistry {
public:
    struct SpeciesInfo {
        std::string name;
        std::vector<std::shared_ptr<Species>> list;
        int initial_count;
    };

    std::map<std::string, SpeciesInfo> registry;

    SpeciesRegistry(const struct EcosystemConfig& config);
    void register_species(const std::string& name, std::shared_ptr<Species> prototype, int initial_count);
    std::vector<std::shared_ptr<Species>>& get_species_list(const std::string& name);
    const std::vector<std::shared_ptr<Species>>& get_species_list(const std::string& name) const;
    int get_initial_count(const std::string& name) const;
    std::vector<std::string> get_all_species_names() const;
    void add_individual(const std::string& name, std::shared_ptr<Species> individual);
    void extend_individuals(const std::string& name, const std::vector<std::shared_ptr<Species>>& individuals);
    void clear_species(const std::string& name);
    void clear_all();
    int get_species_count(const std::string& name) const;
    int get_total_count() const;
    void filter_alive(const std::string& name);
    void filter_all_alive();
};

// Ecosystem configuration (simulation parameters)
struct EcosystemConfig {
    int world_width;
    int world_height;
    int initial_grass;
    int initial_cows;
    int initial_tigers;
    EcosystemConfig(int w = 800, int h = 600, int g = 100, int c = 10, int t = 1)
        : world_width(w), world_height(h), initial_grass(g), initial_cows(c), initial_tigers(t) {}
};

// Ecosystem state manager (simulation core)
class EcosystemState {
public:
    EcosystemConfig config;
    int time_step;
    SpeciesRegistry species_registry;
    SpeciesStatistics births;
    SpeciesStatistics deaths;
    std::vector<std::map<SpeciesType, int>> population_history;

    EcosystemState(const EcosystemConfig& config);

    void initialize_populations();
    EcosystemStateData get_ecosystem_state() const;
    void update_species(const EcosystemStateData& state);
    void handle_reproduction();
    void update_statistics();
    void cleanup_dead();
    SpeciesStatistics get_species_counts() const;
    SpeciesPopulationData get_species_data() const;
    void reset(const EcosystemConfig& config);
    std::vector<std::string> check_extinction() const;
};