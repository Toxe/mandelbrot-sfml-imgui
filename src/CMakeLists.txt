add_executable(mandelbrot
    main.cpp
    register_events.cpp register_events.h
    app/app.cpp app/app.h
    clock/clock.h
    clock/duration.h
    clock/stopwatch.h
    command_line/command_line.cpp command_line/command_line.h
    event_handler/event_handler.cpp event_handler/event_handler.h
    event_handler/events.h
    gradient/gradient.cpp gradient/gradient.h
    mandelbrot/mandelbrot.cpp mandelbrot/mandelbrot.h
    messages/message_queue.h
    messages/messages.h
    supervisor/phase.cpp supervisor/phase.h
    supervisor/supervisor_commands.h
    supervisor/supervisor_status.cpp supervisor/supervisor_status.h
    supervisor/supervisor.cpp supervisor/supervisor.h
    ui/colors.h
    ui/input_value.h
    ui/interface_hidden_hint_window.cpp ui/interface_hidden_hint_window.h
    ui/ui_commands.h
    ui/ui.cpp ui/ui.h
    window/window_commands
    window/window.cpp window/window.h
    worker/worker.cpp worker/worker.h
)

set_target_properties(mandelbrot PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(mandelbrot PUBLIC cxx_std_20)
target_compile_options(mandelbrot PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mandelbrot PRIVATE ${SANITIZER_FLAGS} CLI11::CLI11 fmt::fmt spdlog::spdlog spdlog::spdlog_header_only sfml-system sfml-network sfml-graphics sfml-window ImGui-SFML::ImGui-SFML)
