#pragma once

#include <spdlog/spdlog.h>

#include "ui.h"
#include "event_handler/command.h"
#include "supervisor/supervisor.h"
#include "ui/ui.h"
#include "window/window.h"

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
            SupervisorImageRequest image_request = ui.scroll_image_params(window.size(), -window.size().height / 8, 0);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ScrollRightCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollRightCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image_params(window.size(), window.size().height / 8, 0);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ScrollUpCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollUpCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image_params(window.size(), 0, -window.size().height / 8);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ScrollDownCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ScrollDownCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.scroll_image_params(window.size(), 0, window.size().height / 8);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ZoomInCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ZoomInCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.zoom_image_params(window.size(), 2.0);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ZoomOutCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ZoomOutCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.zoom_image_params(window.size(), 0.5);
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command CalculateImageCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("CalculateImageCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorImageRequest image_request = ui.calculate_image_params(window.size());
            supervisor.calculate_image(image_request);
            ui.set_needs_to_recalculate_image(false);
        }
    };
}

Command ColorizeImageCommand(Window& window, UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ColorizeImageCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            SupervisorColorize colorize = ui.colorize_image_params(window.size());
            supervisor.colorize(colorize);
        }
    };
}

Command ChangeNumberOfThreadsCommand(UI& ui, Supervisor& supervisor)
{
    return [&] {
        spdlog::debug("ChangeNumberOfThreadsCommand");

        if (supervisor.status().phase() == Phase::Idle) {
            int num_threads = ui.num_threads_input_value();
            ui.set_num_threads_input_value_changed(false);

            supervisor.restart(num_threads);
        }
    };
}
