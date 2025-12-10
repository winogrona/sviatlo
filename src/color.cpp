#include <color.hpp>

#include <string>

namespace color {
    using std::string;

    HSV::HSV(Color rgb) {
        *this = rgb.to_hsv();
    }

    Color HSV::to_rgb() {
        return Color(*this);
    }

    Color::Color(HSV hsv) {
        double hh = hsv.h * 6.0;
        double ss = hsv.s;
        double vv = hsv.v;

        int i = (int)hh;
        double ff = hh - i;
        double p = vv * (1.0 - ss);
        double q = vv * (1.0 - (ss * ff));
        double t = vv * (1.0 - (ss * (1.0 - ff)));

        double r_, g_, b_;
        switch (i % 6) {
            case 0:
                r_ = vv; g_ = t; b_ = p;
                break;
            case 1:
                r_ = q; g_ = vv; b_ = p;
                break;
            case 2:
                r_ = p; g_ = vv; b_ = t;
                break;
            case 3:
                r_ = p; g_ = q; b_ = vv;
                break;
            case 4:
                r_ = t; g_ = p; b_ = vv;
                break;
            case 5:
            default:
                r_ = vv; g_ = p; b_ = q;
                break;
        }

        r = static_cast<uint8_t>(r_ * 255);
        g = static_cast<uint8_t>(g_ * 255);
        b = static_cast<uint8_t>(b_ * 255);
    }

    Color::Color(uint8_t r, uint8_t g, uint8_t b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    Color::Color(uint32_t color) {
        r = (color >> 16) & 0xFF;
        g = (color >> 8) & 0xFF;
        b = color & 0xFF;
    }

    string Color::to_hex() {
        char hex[] = "000000";
        sprintf(hex, "%02x%02x%02x", r, g, b);
        return string(hex);
    }

    string Color::to_escape_seq_foreground() {
        char code[] = "\x1b[38;2;255;255;255m";
        sprintf(code, "\x1b[38;2;%d;%d;%dm", r, g, b);
        return string(code);
    }

    string Color::to_escape_seq_background() {
        char code[] = "\x1b[48;2;255;255;255m";
        sprintf(code, "\x1b[48;2;%d;%d;%dm", r, g, b);
        return string(code);
    }

    HSV Color::to_hsv() {
        HSV hsv;
        double rd = r / 255.0;
        double gd = g / 255.0;
        double bd = b / 255.0;

        double max = rd > gd ? (rd > bd ? rd : bd) : (gd > bd ? gd : bd);
        double min = rd < gd ? (rd < bd ? rd : bd) : (gd < bd ? gd : bd);
        double delta = max - min;

        hsv.v = max;

        if (max == 0) {
            hsv.s = 0;
            hsv.h = 0;
            return hsv;
        }

        hsv.s = delta / max;

        if (delta == 0) {
            hsv.h = 0;
            return hsv;
        }

        if (max == rd) {
            hsv.h = (gd - bd) / delta;
        } else if (max == gd) {
            hsv.h = 2 + (bd - rd) / delta;
        } else {
            hsv.h = 4 + (rd - gd) / delta;
        }

        hsv.h /= 6;
        if (hsv.h < 0) {
            hsv.h += 1;
        }

        return hsv;
    }
}