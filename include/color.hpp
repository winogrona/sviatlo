#ifndef __WINOGRONA_COLOR_HPP__
#define __WINOGRONA_COLOR_HPP__

#include <stdint.h>
#include <string>
#include <stdio.h>

#include <FastLED.h>

namespace color {
    using std::string;

    inline constexpr const char * ESCAPE_SEQ_RESET = "\x1b[0m";

    struct Color;

    struct HSV {
        double h; // 0.0 - 1.0
        double s; // 0.0 - 1.0
        double v; // 0.0 - 1.0

        inline constexpr HSV() : h(0), s(0), v(0) {};
        inline constexpr HSV(double h, double s, double v) : h(h), s(s), v(v) {};
        HSV(Color rgb);
        Color to_rgb();
    };

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        inline constexpr Color() : r(0), g(0), b(0) {};

        Color(HSV hsv);
        Color(uint8_t r, uint8_t g, uint8_t b);
        Color(uint32_t color);
        string to_hex();
        string to_escape_seq_foreground();
        string to_escape_seq_background();
        HSV to_hsv();

        inline operator CRGB() {
            return CRGB(r, g, b);
        }

        inline constexpr uint32_t to_uint32() {
            return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        }

        inline constexpr operator uint32_t() {
            return to_uint32();
        }
    };
}

#endif