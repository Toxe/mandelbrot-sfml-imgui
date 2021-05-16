#include "register_events.h"

#include "event_handler/event_handler.h"
#include "supervisor/supervisor.h"
#include "ui/ui.h"
#include "window/window.h"

#include "supervisor/supervisor_commands.h"
#include "ui/ui_commands.h"
#include "window/window_commands.h"

void register_events(EventHandler& event_handler, Window& window, UI& ui, Supervisor& supervisor)
{
    // Window
    event_handler.set_command(Event::CloseWindow,      CloseWindowCommand(window));
    event_handler.set_command(Event::ResizedWindow,    ResizedWindowCommand(window));
    event_handler.set_command(Event::ToggleFullscreen, ToggleFullscreenCommand(window));

    // UI
    event_handler.set_command(Event::ToggleHelp,         ToggleHelpCommand(ui));
    event_handler.set_command(Event::ToggleUIVisibility, ToggleUIVisibilityCommand(ui));

    event_handler.set_command(Event::ScrollLeft,  ScrollLeftCommand(window, ui, supervisor));
    event_handler.set_command(Event::ScrollRight, ScrollRightCommand(window, ui, supervisor));
    event_handler.set_command(Event::ScrollUp,    ScrollUpCommand(window, ui, supervisor));
    event_handler.set_command(Event::ScrollDown,  ScrollDownCommand(window, ui, supervisor));
    event_handler.set_command(Event::ZoomIn,      ZoomInCommand(window, ui, supervisor));
    event_handler.set_command(Event::ZoomOut,     ZoomOutCommand(window, ui, supervisor));

    event_handler.set_command(Event::CalculateImage,        CalculateImageCommand(window, ui, supervisor));
    event_handler.set_command(Event::ColorizeImage,         ColorizeImageCommand(window, ui, supervisor));
    event_handler.set_command(Event::ChangeNumberOfThreads, ChangeNumberOfThreadsCommand(ui, supervisor));

    // Supervisor
    event_handler.set_command(Event::CancelCalculation, CancelCalculationCommand(supervisor));
}
