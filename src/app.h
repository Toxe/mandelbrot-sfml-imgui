#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

#include "cli.h"

class App {
    CLI cli_;
    std::unique_ptr<sf::RenderWindow> window_;

public:
    App(const CLI& cli);

    const CLI& cli() const { return cli_; }
    sf::RenderWindow& window() const { return *window_; };

    void poll_events();
    void render(sf::Sprite& sprite);
};
