#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "mandelbrot.h"
#include "supervisor.h"

void worker(const int id, std::mutex& mtx, std::condition_variable& cv, std::queue<WorkUnit>& work_queue, std::queue<WorkResult>& results_queue);
