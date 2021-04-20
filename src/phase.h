#pragma once

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

const char* phase_name(const Phase phase);
