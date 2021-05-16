#pragma once

#include <spdlog/spdlog.h>

#include "event_handler/command.h"
#include "supervisor/supervisor.h"

Command CancelCalculationCommand(Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("CancelCalculationCommand");

        if (supervisor.status().phase() == Phase::Calculating)
            supervisor.cancel_calculation();
    };
}
