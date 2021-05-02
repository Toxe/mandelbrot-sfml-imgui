#include "gradient.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include "mandelbrot.h"

const std::string gradients_directory = "assets/gradients";

// Compare two float values for "enough" equality.
bool equal_enough(float a, float b) noexcept
{
    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    a = std::abs(a);
    b = std::abs(b);

    return std::abs(a - b) <= std::max(a, b) * epsilon;
}

Gradient load_gradient(const std::string& name)
{
    auto path = std::filesystem::path{gradients_directory} / (name + ".gradient");

    spdlog::debug("loading gradient: {}", path);

    Gradient gradient{name};
    gradient.colors.push_back(GradientColor{0.0f, 0.0f, 0.0f, 0.0f});
    gradient.colors.push_back(GradientColor{1.0f, 1.0f, 1.0f, 1.0f});

    std::ifstream in{path};
    std::string line;

    if (!in.is_open())
        throw std::runtime_error("unable to open gradient file");

    const std::regex re{R"(([0-9]*\.?[0-9]+):\s*([0-9]*\.?[0-9]+),\s*([0-9]*\.?[0-9]+),\s*([0-9]*\.?[0-9]+)$)"};
    std::smatch m;

    while (std::getline(in, line)) {
        if (std::regex_match(line, m, re)) {
            float pos = std::stof(m[1]);
            auto col = std::find(gradient.colors.begin(), gradient.colors.end(), pos);

            if (col != gradient.colors.end())
                *col = GradientColor{pos, std::stof(m[2]), std::stof(m[3]), std::stof(m[4])};
            else
                gradient.colors.push_back({pos, std::stof(m[2]), std::stof(m[3]), std::stof(m[4])});
        }
    }

    std::sort(gradient.colors.begin(), gradient.colors.end());
    return gradient;
}

sf::Color color_from_gradient_range(const GradientColor& left, const GradientColor& right, const float pos) noexcept
{
    const float relative_pos_between_colors = (pos - left.pos) / (right.pos - left.pos);
    return sf::Color{static_cast<sf::Uint8>(255.0f * std::lerp(left.r, right.r, relative_pos_between_colors)),
                     static_cast<sf::Uint8>(255.0f * std::lerp(left.g, right.g, relative_pos_between_colors)),
                     static_cast<sf::Uint8>(255.0f * std::lerp(left.b, right.b, relative_pos_between_colors))};
}

sf::Color color_from_gradient(const Gradient& gradient, const float pos) noexcept
{
    const auto end = gradient.colors.cend();

    const auto it = std::adjacent_find(gradient.colors.cbegin(), end,
        [&](const GradientColor& left, const GradientColor& right) { return left.pos <= pos && pos <= right.pos; });

    if (it != end)
        return color_from_gradient_range(*it, *(it + 1), pos);
    else
        return sf::Color::Black;
}

std::vector<Gradient> load_available_gradients()
{
    std::vector<Gradient> available_gradients;

    for (const auto& p : std::filesystem::directory_iterator(gradients_directory))
        available_gradients.push_back(load_gradient(p.path().filename().replace_extension("").string()));

    std::sort(available_gradients.begin(), available_gradients.end(),
        [](const Gradient& a, const Gradient& b) { return a.name_ < b.name_; });

    return available_gradients;
}
