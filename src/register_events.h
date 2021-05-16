#pragma once

class EventHandler;
class Supervisor;
class UI;
class Window;

void register_events(EventHandler& event_handler, Window& window, UI& ui, Supervisor& supervisor);
