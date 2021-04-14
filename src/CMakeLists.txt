set(ALL_TARGETS
    mandelbrot
)

add_executable(mandelbrot main.cpp
                          messages.h
                          app.cpp app.h
                          cli.cpp cli.h
                          mandelbrot.cpp mandelbrot.h
                          gradient.cpp gradient.h
                          stopwatch.h
                          supervisor.cpp supervisor.h
                          ui.cpp ui.h
                          worker.cpp worker.h
)

foreach(target ${ALL_TARGETS})
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    target_compile_features(${target} PUBLIC cxx_std_20)
    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} ${DEFAULT_COMPILER_OPTIONS_AND_WARNINGS})
    target_link_libraries(${target} PRIVATE ${SANITIZER_FLAGS})
endforeach()

target_link_libraries(mandelbrot PRIVATE clipp::clipp fmt::fmt spdlog::spdlog spdlog::spdlog_header_only sfml-system sfml-network sfml-graphics sfml-window ImGui-SFML::ImGui-SFML)
