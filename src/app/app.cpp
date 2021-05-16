#include "app.h"

#include <spdlog/spdlog.h>

#include "command_line/command_line.h"

App::App(const CommandLine& cli)
    : window_{Window(cli)}, supervisor_{Supervisor(window_)}, ui_{UI(cli)}
{
    gradient_ = load_gradient("benchmark");
    supervisor_.run(cli.num_threads(), gradient_);
}

void App::next_frame()
{
    elapsed_time_ = frame_time_clock_.restart();
    spdlog::trace("elapsed time: {}s ({} FPS)", elapsed_time_.as_seconds(), elapsed_time_.fps());

    window_.next_frame(elapsed_time_);
}

void App::render()
{
    if (running()) {
        ui_.render(*this);
        window_.render();
    }
}

void App::shutdown()
{
    supervisor_.shutdown();
}
