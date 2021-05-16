#include "register_events.h"
#include "app/app.h"
#include "command_line/command_line.h"
#include "event_handler/event_handler.h"
#include "supervisor/supervisor.h"
#include "ui/ui.h"

int main(int argc, char* argv[])
{
    CommandLine cli(argc, argv);
    App app(cli);
    UI ui(cli);

    Supervisor supervisor(app.window());
    supervisor.run(cli.num_threads());

    EventHandler event_handler;
    register_events(event_handler, app.window(), ui, supervisor);
    ui.set_event_handler(&event_handler);

    while (app.running()) {
        app.next_frame();
        event_handler.poll_events(app.window().window());

        if (app.running()) {
            ui.render(app, supervisor);
            app.window().render();
        }
    }

    supervisor.shutdown();
}
