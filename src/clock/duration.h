#pragma once

#include <chrono>

class Duration {
    std::chrono::nanoseconds ns_;

public:
    Duration() : ns_{0} {}
    Duration(const std::chrono::nanoseconds ns) : ns_{ns} {}

    long long as_microseconds() const { return std::chrono::duration_cast<std::chrono::microseconds>(ns_).count(); }
    float as_seconds() const { return std::chrono::duration<float>{ns_}.count(); }
    float fps() const { return 1.0f / as_seconds(); }
};
