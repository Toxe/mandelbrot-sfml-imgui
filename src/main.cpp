#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

#include "app.h"
#include "gradient.h"
#include "supervisor.h"
#include "ui.h"

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);
    UI ui(app, cli);

    const auto gradient = load_gradient("assets/gradients/benchmark.gradient");

    sf::Image image;
    image.create(app.window().getSize().x, app.window().getSize().y);
    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    auto supervisor = supervisor_start(image, texture, cli.num_threads(), gradient);

    while (app.window().isOpen()) {
        app.poll_events();
        ui.render(app, image);
        app.render(sprite);
    }

    ui.shutdown();
    supervisor_shutdown(supervisor);
}
