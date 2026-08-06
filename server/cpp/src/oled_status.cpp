#include "oled_status.hpp"

#include <bcm2835.h>
#include <SSD1306_OLED.hpp>

#include <cstdio>

OledStatus::~OledStatus() {
    shutdown();
}

bool OledStatus::init() {
    if (!bcm2835_init()) {
        std::fprintf(stderr, "[OLED] bcm2835_init() fallo (se requiere root). Continuando sin display.\n");
        return false;
    }

    oled_ = new SSD1306(kWidth, kHeight);

    if (!oled_->OLED_I2C_ON()) {
        std::fprintf(stderr, "[OLED] No se pudo iniciar el bus I2C. Continuando sin display.\n");
        bcm2835_close();
        return false;
    }

    oled_->OLEDbegin(BCM2835_I2C_CLOCK_DIVIDER_626, 0x3C, false);

    if (!oled_->OLEDSetBufferPtr(kWidth, kHeight, buffer_.data(), buffer_.size())) {
        std::fprintf(stderr, "[OLED] Error configurando el framebuffer. Continuando sin display.\n");
        oled_->OLED_I2C_OFF();
        bcm2835_close();
        return false;
    }

    ready_ = true;
    oled_->OLEDclearBuffer();
    oled_->OLEDupdate();
    std::fprintf(stderr, "[OLED] Display listo (128x32 I2C 0x3C).\n");
    return true;
}

void OledStatus::show(const std::string& line1, const std::string& line2,
                      const std::string& line3, const std::string& line4) {
    if (!ready_ || oled_ == nullptr) return;

    oled_->OLEDclearBuffer();
    oled_->setTextColor(WHITE);
    oled_->setTextSize(1);

    const char* lines[4] = {line1.c_str(), line2.c_str(), line3.c_str(), line4.c_str()};
    int y = 0;
    for (int i = 0; i < 4; ++i) {
        if (lines[i][0] == '\0') continue;
        oled_->setCursor(0, y);
        oled_->print(std::string(lines[i]));
        y += 8;
    }
    oled_->OLEDupdate();
}

void OledStatus::shutdown() {
    if (!ready_ || oled_ == nullptr) return;
    oled_->OLEDclearBuffer();
    oled_->OLEDupdate();
    oled_->OLEDPowerDown();
    oled_->OLED_I2C_OFF();
    bcm2835_close();
    ready_ = false;
}
