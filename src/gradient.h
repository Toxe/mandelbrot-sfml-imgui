#pragma once

#include <string>
#include <vector>

#include <SFML/Graphics/Color.hpp>

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

Gradient load_gradient(const std::string& filename);
sf::Color color_from_gradient(const Gradient& gradient, const float pos) noexcept;
std::vector<std::string> list_available_gradients();
