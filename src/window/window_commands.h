#pragma once

#include <spdlog/spdlog.h>

#include "window.h"
#include "event_handler/command.h"

Command CloseWindowCommand(Window& window)
{
    return [&] {
        spdlog::debug("CloseWindowCommand");
        window.close();
    };
}

Command ResizedWindowCommand(Window& window)
{
    return [&] {
        spdlog::debug("ResizedWindowCommand");
        window.resized_window();
    };
}

Command ToggleFullscreenCommand(Window& window)
{
    return [&] {
        spdlog::debug("ToggleFullscreenCommand");
        window.toggle_fullscreen();
    };
}
