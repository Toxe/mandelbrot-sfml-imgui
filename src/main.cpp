#include "register_events.h"
#include "app/app.h"
#include "command_line/command_line.h"
#include "event_handler/event_handler.h"
#include "ui/ui.h"

int main(int argc, char* argv[])
{
    CommandLine cli(argc, argv);
    App app(cli);
    UI ui(cli);

    EventHandler event_handler;

    register_events(event_handler, app.window(), ui, app.supervisor());

    ui.set_event_handler(&event_handler);

    while (app.running()) {
        app.next_frame();
        event_handler.poll_events(app.window().window());

        if (app.running()) {
            ui.render(app);
            app.window().render();
        }
    }

    app.shutdown();
}
