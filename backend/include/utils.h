#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <optional>
#include <Eigen/Dense>

// Position struct representing coordinates in 2D space
struct Position {
    double x;
    double y;
    // Calculate Euclidean distance to another position
    double distance_to(const Position& other) const {
        return std::sqrt((x - other.x)*(x - other.x) + (y - other.y)*(y - other.y));
    }
};

// Ecosystem state snapshot for simulation and frontend
struct EcosystemStateData {
    int world_width;
    int world_height;
    std::vector<std::shared_ptr<Species>> grass_list;
    std::vector<std::shared_ptr<Species>> cow_list;
    std::vector<std::shared_ptr<Species>> tiger_list;
    int time_step;
    Eigen::MatrixXd grass_positions_array; // Corresponds to numpy array in Python
    std::vector<std::shared_ptr<Species>> alive_grass_objects;
};