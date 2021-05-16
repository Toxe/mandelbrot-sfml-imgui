#include "register_events.h"
#include "app/app.h"
#include "command_line/command_line.h"
#include "event_handler/event_handler.h"
#include "supervisor/supervisor.h"
#include "ui/ui.h"
#include "window/window.h"

int main(int argc, char* argv[])
{
    CommandLine cli(argc, argv);

    App app;
    UI ui(cli);
    Window window(cli);
    Supervisor supervisor(cli, window);

    EventHandler event_handler;
    register_events(event_handler, window, ui, supervisor);
    ui.set_event_handler(&event_handler);

    while (window.is_open()) {
        app.next_frame();
        window.next_frame(app.elapsed_time());

        event_handler.poll_events(window.window());

        if (window.is_open()) {
            ui.render(app.elapsed_time(), supervisor.status(), window.size());
            window.render();
        }
    }

    supervisor.shutdown();
}
