#pragma once

#include <string>
#include <vector>

namespace sf {
    class Color;
}

bool equal_enough(float a, float b) noexcept;

struct GradientColor {
    float pos;
    float r, g, b;
    bool operator==(const float p) const noexcept { return equal_enough(p, pos); }
    bool operator<(const GradientColor& col) const noexcept { return pos < col.pos; }
};

struct Gradient {
    std::string name_;
    std::vector<GradientColor> colors;

    Gradient() : name_{""} {}
    Gradient(std::string name) : name_{name} {}
};

Gradient load_gradient(const std::string& name);
sf::Color color_from_gradient(const Gradient& gradient, const float pos) noexcept;
std::vector<Gradient> load_available_gradients();
