#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "messages.h"

void worker(const int id, std::mutex& mtx, std::condition_variable& cv, std::queue<WorkerMessage>& worker_message_queue, std::queue<SupervisorMessage>& supervisor_message_queue);
