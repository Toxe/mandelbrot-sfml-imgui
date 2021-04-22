#pragma once

template <typename T>
class InputValue {
    T value_;
    bool changed_;

public:
    InputValue() : value_{0}, changed_{false} {};
    InputValue(const T value) : value_{value}, changed_{false} {};

    [[nodiscard]] T& get() { return value_; };

    T& set(T new_value) {
        changed_ = true;
        return value_ = new_value;
    };

    T& reset(T new_value) {
        changed_ = false;
        return value_ = new_value;
    };

    T& operator =(const T& new_value) { return set(new_value); };

    [[nodiscard]] bool changed() const { return changed_; };
    void changed(bool f) { changed_ = f; };
};
