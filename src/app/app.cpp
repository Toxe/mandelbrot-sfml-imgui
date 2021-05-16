#include "app.h"

#include <spdlog/spdlog.h>

void App::next_frame()
{
    elapsed_time_ = frame_time_clock_.restart();
    spdlog::trace("elapsed time: {}s ({} FPS)", elapsed_time_.as_seconds(), elapsed_time_.fps());
}
