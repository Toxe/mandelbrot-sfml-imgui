#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "mandelbrot.h"
#include "task_master.h"

void task_worker(const int id, std::mutex& mtx, std::condition_variable& cv, std::queue<WorkUnit>& work_queue, std::queue<WorkResult>& results_queue);
