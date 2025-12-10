#ifndef __WINOGRONA_STRIP_HPP__
#define __WINOGRONA_STRIP_HPP__

#include <settings.hpp>
#include <color.hpp>

#include <vector>

#include <FastLED.h>

namespace strip {
    using std::vector;
    using color::Color;

    const int len = settings::LEDS_NUM;
    extern vector<Color> leds;

    void setup(vector<Color> strip_state);
    void show();
}

#endif