#ifndef __WINOGRONA_COLOR_HPP__
#define __WINOGRONA_COLOR_HPP__

#include <stdint.h>
#include <string>
#include <stdio.h>

#include <FastLED.h>

namespace color {
    using std::string;

    inline constexpr const char * ESCAPE_SEQ_RESET = "\x1b[0m";

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        Color() {
            r = 0;
            g = 0;
            b = 0;
        }

        Color(uint8_t r, uint8_t g, uint8_t b) {
            this->r = r;
            this->g = g;
            this->b = b;
        }

        Color(uint32_t color) {
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;
        }

        string to_hex() {
            char hex[] = "000000";
            sprintf(hex, "%02x%02x%02x", r, g, b);
            return string(hex);
        }

        string to_escape_seq_foreground() {
            char code[] = "\x1b[38;2;255;255;255m";
            sprintf(code, "\x1b[38;2;%d;%d;%dm", r, g, b);
            return string(code);
        }

        string to_escape_seq_background() {
            char code[] = "\x1b[48;2;255;255;255m";
            sprintf(code, "\x1b[48;2;%d;%d;%dm", r, g, b);
            return string(code);
        }

        operator CRGB() {
            return CRGB(r, g, b);
        }
    };
}

#endif