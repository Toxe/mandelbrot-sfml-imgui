#pragma once

#include <variant>
#include <vector>

#include "mandelbrot.h"

// ---- Supervisor messages -----------
struct SupervisorImageRequest {
    int max_iterations;
    ImageSize image_size;
    FractalSection fractal_section;
};

struct SupervisorResultsFromWorker {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
};

struct SupervisorQuit {};

using SupervisorMessage = std::variant<SupervisorImageRequest, SupervisorQuit, SupervisorResultsFromWorker>;

// ---- Worker messages ---------------
struct WorkerCalc {
    int max_iterations;
    ImageSize image_size;
    CalculationArea area;
    FractalSection fractal_section;
    std::vector<CalculationResult>* results_per_point;
    std::vector<int>* combined_iterations_histogram;
};

struct WorkerQuit {};

using WorkerMessage = std::variant<WorkerCalc, WorkerQuit>;
