#pragma once

#include <imgui.h>

namespace UserInterface::Colors {

const static ImVec4 light_blue{100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f};
const static ImVec4 light_gray{0.7f, 0.7f, 0.7f, 1.0f};
const static ImVec4 yellow{1.0f, 1.0f, 0.0f, 1.0f};

const static ImVec4 phase_default{1.0f, 1.0f, 1.0f, 1.0f};
const static ImVec4 phase_idle{0.0f, 1.0f, 0.0f, 1.0f};
const static ImVec4 phase_canceled{1.0f, 0.0f, 0.0f, 1.0f};

}
