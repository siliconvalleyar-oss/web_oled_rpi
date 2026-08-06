#pragma once

#include <array>
#include <cstdint>
#include <string>

class SSD1306;

class OledStatus {
public:
    OledStatus() = default;
    ~OledStatus();

    OledStatus(const OledStatus&) = delete;
    OledStatus& operator=(const OledStatus&) = delete;

    bool init();
    bool ready() const { return ready_; }

    void show(const std::string& line1,
              const std::string& line2 = "",
              const std::string& line3 = "",
              const std::string& line4 = "");

    void shutdown();

    void disable();

private:
    static constexpr int kWidth = 128;
    static constexpr int kHeight = 32;
    std::array<uint8_t, kWidth * (kHeight / 8)> buffer_{};
    SSD1306* oled_ = nullptr;
    bool ready_ = false;
};
