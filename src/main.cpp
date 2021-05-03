#include "app.h"
#include "command_line/command_line.h"

int main(int argc, char* argv[])
{
    CommandLine cli(argc, argv);
    App app(cli);

    while (app.running()) {
        app.next_frame();
        app.poll_events();
        app.render();
    }

    app.shutdown();
}
