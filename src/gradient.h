#pragma once

#include <string>
#include <vector>

#include "pixel_color.h"

bool equal_enough(float a, float b) noexcept;

struct GradientColor {
    float pos;
    float r, g, b;
    bool operator==(const float p) const noexcept { return equal_enough(p, pos); }
    bool operator<(const GradientColor& col) const noexcept { return pos < col.pos; }
};

struct Gradient {
    std::vector<GradientColor> colors;
};

Gradient load_gradient(const std::string& filename);
PixelColor color_from_gradient(const Gradient& gradient, const float pos) noexcept;
