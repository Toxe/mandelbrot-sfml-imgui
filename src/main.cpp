#include "app.h"
#include "gradient.h"
#include "supervisor.h"
#include "ui.h"

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);
    UI ui(cli, app.window());

    Supervisor supervisor(app, cli.num_threads());
    supervisor.run();

    while (app.window().isOpen()) {
        app.next_frame(supervisor);
        app.poll_events(supervisor, ui);
        ui.render(app, supervisor);
        app.render();
    }

    ui.shutdown();
    supervisor.shutdown();
}
