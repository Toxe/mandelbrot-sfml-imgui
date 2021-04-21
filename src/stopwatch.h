#pragma once

#include <SFML/System/Clock.hpp>

class Stopwatch {
    bool running_ = false;
    float elapsed_time_ = 0.0;
    sf::Clock clock_;

public:
    [[nodiscard]] bool is_running() const { return running_; }

    void start()
    {
        running_ = true;
        elapsed_time_ = 0.0;
        clock_.restart();
    }

    void stop()
    {
        running_ = false;
        elapsed_time_ = clock_.getElapsedTime().asSeconds();
    }

    [[nodiscard]] float time()
    {
        if (!running_)
            return elapsed_time_;

        return elapsed_time_ = clock_.getElapsedTime().asSeconds();
    }
};
