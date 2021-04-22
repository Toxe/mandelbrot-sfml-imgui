#include "app.h"
#include "cli.h"
#include "gradient.h"
#include "supervisor.h"
#include "ui.h"

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);
    UI ui(cli, app.window());

    Supervisor supervisor(cli.num_threads(), app.window(), app.gradient());
    supervisor.run();

    while (app.running()) {
        app.next_frame(supervisor);
        app.poll_events(supervisor, ui);

        if (app.running()) {
            ui.render(app, supervisor);
            app.render();
        }
    }

    supervisor.shutdown();
}
