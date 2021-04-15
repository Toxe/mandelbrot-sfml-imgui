#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "mandelbrot.h"

// ---- Supervisor messages -----------
struct SupervisorImageRequest {
    int max_iterations;
    int area_size;
    ImageSize image_size;
    FractalSection fractal_section;
};

struct SupervisorResultsFromWorker {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::unique_ptr<sf::Uint8[]> pixels;
};

struct SupervisorQuit {};
struct SupervisorCancel {};

using SupervisorMessage = std::variant<SupervisorImageRequest, SupervisorResultsFromWorker, SupervisorQuit, SupervisorCancel>;

// ---- Worker messages ---------------
struct WorkerCalc {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::vector<int>* combined_iterations_histogram;
    std::unique_ptr<sf::Uint8[]> pixels;
};

struct WorkerQuit {};

using WorkerMessage = std::variant<WorkerCalc, WorkerQuit>;
