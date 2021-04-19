#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "gradient.h"

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

struct FractalSection {
    double center_x, center_y, height;
};

// ---- Supervisor messages -----------
struct SupervisorImageRequest {
    int max_iterations;
    int area_size;
    ImageSize image_size;
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
    CalculationArea area;
    std::vector<sf::Uint8>* colorization_buffer;
};

struct SupervisorQuit {};
struct SupervisorCancel {};

using SupervisorMessage = std::variant<SupervisorImageRequest, SupervisorCalculationResults, SupervisorColorizationResults, SupervisorQuit, SupervisorCancel>;

// ---- Worker messages ---------------
struct WorkerCalculate {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::vector<int>* combined_iterations_histogram;
    std::unique_ptr<sf::Uint8[]> pixels;
};

struct WorkerColorize {
    int max_iterations;
    CalculationArea area;
    Gradient* gradient;
    std::vector<int>* combined_iterations_histogram;
    std::vector<CalculationResult>* results_per_point;
    std::vector<float>* equalized_iterations;
    std::vector<sf::Uint8>* colorization_buffer;
};

struct WorkerQuit {};

using WorkerMessage = std::variant<WorkerCalculate, WorkerColorize, WorkerQuit>;
