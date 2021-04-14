#include <thread>

#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

#include "app.h"
#include "gradient.h"
#include "supervisor.h"
#include "ui.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);

    const auto gradient = load_gradient("assets/gradients/benchmark.gradient");

    App app;
    UI ui(app);

    sf::Image image;
    image.create(app.window().getSize().x, app.window().getSize().y);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    auto supervisor = supervisor_start(image, texture, std::thread::hardware_concurrency(), gradient);

    sf::Clock clock;

    while (app.window().isOpen()) {
        app.poll_events();
        ui.render(app, clock, image);
        app.render(sprite);
    }

    ui.shutdown();
    supervisor_shutdown(supervisor);
}
