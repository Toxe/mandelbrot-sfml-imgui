#pragma once

#include <future>

#include "app.h"

enum class Phase {
    Starting,
    Idle,
    RequestSent,
    RequestReceived,
    Waiting,
    Coloring,
    Shutdown,
    Canceled,
};

struct SupervisorImageRequest;

std::future<void> supervisor_start(App& app, const int num_threads, const Gradient& gradient);
void supervisor_stop();
void supervisor_shutdown(std::future<void>& supervisor);
void supervisor_calc_image(const SupervisorImageRequest& image_request);
const char* supervisor_phase_name(const Phase p);
void supervisor_cancel_render();
