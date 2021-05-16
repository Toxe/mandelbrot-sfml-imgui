#include "event_handler.h"

#include <spdlog/spdlog.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

const Command no_command = [] { spdlog::debug("NoCommand"); };

EventHandler::EventHandler()
{
    commands_[Event::CloseWindow]           = no_command;
    commands_[Event::ResizedWindow]         = no_command;
    commands_[Event::ToggleFullscreen]      = no_command;
    commands_[Event::ToggleHelp]            = no_command;
    commands_[Event::ToggleUIVisibility]    = no_command;
    commands_[Event::ScrollLeft]            = no_command;
    commands_[Event::ScrollRight]           = no_command;
    commands_[Event::ScrollUp]              = no_command;
    commands_[Event::ScrollDown]            = no_command;
    commands_[Event::ZoomIn]                = no_command;
    commands_[Event::ZoomOut]               = no_command;
    commands_[Event::CalculateImage]        = no_command;
    commands_[Event::ColorizeImage]         = no_command;
    commands_[Event::ChangeNumberOfThreads] = no_command;
    commands_[Event::CancelCalculation]     = no_command;
}

void EventHandler::set_command(const Event& event, Command command)
{
    commands_[event] = command;
}

void EventHandler::handle_event(const Event& event)
{
    commands_[event]();
}

void EventHandler::poll_events(sf::RenderWindow& window)
{
    sf::Event event;

    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            handle_event(Event::CloseWindow);
        } else if (event.type == sf::Event::Resized) {
            handle_event(Event::ResizedWindow);
        } else if (event.type == sf::Event::KeyPressed) {
            if (ImGui::GetIO().WantCaptureKeyboard)
                continue;

            if (event.key.code == sf::Keyboard::Escape) {
                handle_event(Event::CloseWindow);
            } else if (event.key.code == sf::Keyboard::F1) {
                handle_event(Event::ToggleHelp);
            } else if (event.key.code == sf::Keyboard::F11) {
                handle_event(Event::ToggleFullscreen);
            } else if (event.key.code == sf::Keyboard::Enter) {
                handle_event(Event::CalculateImage);
            } else if (event.key.code == sf::Keyboard::Space) {
                handle_event(Event::ToggleUIVisibility);
            } else if (event.key.code == sf::Keyboard::Left) {
                handle_event(Event::ScrollLeft);
            } else if (event.key.code == sf::Keyboard::Right) {
                handle_event(Event::ScrollRight);
            } else if (event.key.code == sf::Keyboard::Up) {
                handle_event(Event::ScrollUp);
            } else if (event.key.code == sf::Keyboard::Down) {
                handle_event(Event::ScrollDown);
            } else if (event.key.code == sf::Keyboard::Add) {
                handle_event(Event::ZoomIn);
            } else if (event.key.code == sf::Keyboard::Subtract) {
                handle_event(Event::ZoomOut);
            }
        }
    }
}
