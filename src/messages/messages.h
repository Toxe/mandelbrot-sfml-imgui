#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "gradient/gradient.h"

struct CalculationResult {
    int iter;
    float distance_to_next_iteration;
};

struct ImageSize {
    int width, height;
};

struct CalculationArea {
    int x, y;
    int width, height;
};

struct Scroll {
    int x, y;
};

struct FractalSection {
    double center_x, center_y, height;
};

// ---- Supervisor messages -----------
struct SupervisorImageRequest {
    int max_iterations;
    int tile_size;
    ImageSize image_size;
    CalculationArea area;
    Scroll scroll;
    FractalSection fractal_section;
};

struct SupervisorCalculationResults {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::unique_ptr<sf::Uint8[]> pixels;
};

struct SupervisorColorizationResults {
    int start_row;
    int num_rows;
    int row_width;
    std::vector<sf::Uint8>* colorization_buffer;
};

struct SupervisorColorize {
    int max_iterations;
    ImageSize image_size;
    Gradient gradient;
};

struct SupervisorQuit {};
struct SupervisorCancel {};

using SupervisorMessage = std::variant<SupervisorImageRequest, SupervisorCalculationResults, SupervisorColorizationResults, SupervisorColorize, SupervisorQuit, SupervisorCancel>;

// ---- Worker messages ---------------
struct WorkerCalculate {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::unique_ptr<sf::Uint8[]> pixels;
};

struct WorkerColorize {
    int max_iterations;
    int start_row;
    int num_rows;
    int row_width;
    Gradient* gradient;
    std::vector<CalculationResult>* results_per_point;
    std::vector<float>* equalized_iterations;
    std::vector<sf::Uint8>* colorization_buffer;
};

struct WorkerQuit {};

using WorkerMessage = std::variant<WorkerCalculate, WorkerColorize, WorkerQuit>;
