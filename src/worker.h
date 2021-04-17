#pragma once

#include "message_queue.h"
#include "messages.h"

void worker(const int id, MessageQueue<WorkerMessage>& worker_message_queue, MessageQueue<SupervisorMessage>& supervisor_message_queue);
