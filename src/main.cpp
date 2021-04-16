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
    auto supervisor = supervisor_start(app, cli.num_threads(), gradient);

    while (app.window().isOpen()) {
        app.next_frame();
        app.poll_events();
        ui.render(app);
        app.render();
    }

    ui.shutdown();
    supervisor_shutdown(supervisor);
}
