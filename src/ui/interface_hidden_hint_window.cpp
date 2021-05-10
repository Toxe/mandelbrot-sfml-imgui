#include "interface_hidden_hint_window.h"

#include <imgui.h>

#include "colors.h"

const std::chrono::milliseconds fade_out_delay = 5000ms;

void InterfaceHiddenHintWindow::show()
{
    if (visible_ || has_been_shown_)
        return;

    visible_ = true;
    has_been_shown_ = true;

    tp_window_shown_ = std::chrono::steady_clock::now();
}

void InterfaceHiddenHintWindow::hide()
{
    if (visible_)
        visible_ = false;
}

void InterfaceHiddenHintWindow::render()
{
    if (visible_) {
        const auto viewport = ImGui::GetMainViewport();
        const ImVec2 window_pos{viewport->Size.x / 2.0f, viewport->Size.y - 50.0f};

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, {0.5f, 1.0f});
        ImGui::SetNextWindowBgAlpha(0.5f);

        ImGui::Begin("interface_hidden_hint_window", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

        ImGui::Text("interface hidden");
        ImGui::Text("press");
        ImGui::SameLine();
        ImGui::TextColored(UserInterface::Colors::light_blue, "Space");
        ImGui::SameLine();
        ImGui::Text("to show again");

        ImGui::End();

        if (std::chrono::steady_clock::now() - tp_window_shown_ >= fade_out_delay)
            visible_ = false;
    }
}
