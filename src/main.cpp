#include "app.h"
#include "cli.h"
#include "gradient.h"

int main(int argc, char* argv[])
{
    CLI cli(argc, argv);
    App app(cli);

    while (app.running()) {
        app.next_frame();
        app.poll_events();
        app.render();
    }

    app.shutdown();
}
