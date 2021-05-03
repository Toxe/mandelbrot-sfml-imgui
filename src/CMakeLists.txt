set(ALL_TARGETS
    mandelbrot
)

add_executable(mandelbrot
    app.cpp app.h
    clock.h
    command_line/command_line.cpp command_line/command_line.h
    duration.h
    gradient/gradient.cpp gradient/gradient.h
    input_value.h
    main.cpp
    mandelbrot/mandelbrot.cpp mandelbrot/mandelbrot.h
    message_queue.h
    messages.h
    stopwatch.h
    supervisor/phase.cpp supervisor/phase.h
    supervisor/supervisor_status.cpp supervisor/supervisor_status.h
    supervisor/supervisor.cpp supervisor/supervisor.h
    ui.cpp ui.h
    window.cpp window.h
    worker/worker.cpp worker/worker.h
)

foreach(target ${ALL_TARGETS})
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
    target_link_libraries(${target} PRIVATE ${SANITIZER_FLAGS})
endforeach()

target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mandelbrot PRIVATE CLI11::CLI11 fmt::fmt spdlog::spdlog spdlog::spdlog_header_only sfml-system sfml-network sfml-graphics sfml-window ImGui-SFML::ImGui-SFML)
