#pragma once

#include <memory>

#include <SFML/Graphics.hpp>

class App {
    std::unique_ptr<sf::RenderWindow> window_;

public:
    App();

    sf::RenderWindow& window() const { return *window_; };
    void poll_events();
    void render(sf::Sprite& sprite);
};
