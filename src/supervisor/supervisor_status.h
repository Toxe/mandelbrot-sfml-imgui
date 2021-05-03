#pragma once

#include <atomic>
#include <mutex>

#include "clock/stopwatch.h"
#include "supervisor/phase.h"

class SupervisorStatus {
    std::atomic<Phase> phase_;
    Stopwatch stopwatch_;

    std::mutex mtx_;

public:
    SupervisorStatus() : phase_{Phase::Starting} {}

    [[nodiscard]] Phase phase() const { return phase_; };
    void set_phase(const Phase phase) { phase_ = phase; };

    void start_calculation(const Phase new_phase);
    void stop_calculation(const Phase new_phase);
    [[nodiscard]] Duration calculation_time();
    [[nodiscard]] bool calculation_running();
};
