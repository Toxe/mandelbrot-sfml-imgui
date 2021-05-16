#pragma once

#include <spdlog/spdlog.h>

#include "ui.h"
#include "event_handler/command.h"
#include "supervisor/supervisor.h"

Command ToggleHelpCommand(UI& ui)
{
    return [&] {
        spdlog::debug("ToggleHelpCommand");
        ui.toggle_help();
    };
}

Command ToggleUIVisibilityCommand(UI& ui)
{
    return [&] {
        spdlog::debug("ToggleUIVisibilityCommand");
        ui.toggle_visibility();
    };
}

Command ScrollLeftCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollLeftCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image(window.size(), -window.size().height / 8, 0);
            supervisor.calculate_image(image_request);
        }
    };
}

Command ScrollRightCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollRightCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image(window.size(), window.size().height / 8, 0);
            supervisor.calculate_image(image_request);
        }
    };
}

Command ScrollUpCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollUpCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image(window.size(), 0, -window.size().height / 8);
            supervisor.calculate_image(image_request);
        }
    };
}

Command ScrollDownCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollDownCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image(window.size(), 0, window.size().height / 8);
            supervisor.calculate_image(image_request);
        }
    };
}

Command ZoomInCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ZoomInCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.zoom_image(window.size(), 2.0);
            supervisor.calculate_image(image_request);
        }
    };
}

Command ZoomOutCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ZoomOutCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.zoom_image(window.size(), 0.5);
            supervisor.calculate_image(image_request);
        }
    };
}

Command CalculateImageCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("CalculateImageCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.calculate_image(window.size());
            supervisor.calculate_image(image_request);
        }
    };
}
