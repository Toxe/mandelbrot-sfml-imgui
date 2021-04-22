#include "app.h"
#include "cli.h"
#include "gradient.h"
#include "ui.h"

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);
    UI ui(cli);

    while (app.running()) {
        app.next_frame();
        app.poll_events(ui);

        if (app.running()) {
            ui.render(app);
            app.render();
        }
    }

    app.shutdown();
}
