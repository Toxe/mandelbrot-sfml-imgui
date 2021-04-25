#include "supervisor_status.h"

void SupervisorStatus::start_calculation(const Phase new_phase)
{
    phase_ = new_phase;

    std::lock_guard<std::mutex> lock(mtx_);
    stopwatch_.start();
}

void SupervisorStatus::stop_calculation(const Phase new_phase)
{
    phase_ = new_phase;

    std::lock_guard<std::mutex> lock(mtx_);
    stopwatch_.stop();
}

[[nodiscard]] Duration SupervisorStatus::calculation_time()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return stopwatch_.time();
}

[[nodiscard]] bool SupervisorStatus::calculation_running()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return stopwatch_.is_running();
}
