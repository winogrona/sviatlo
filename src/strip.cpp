#include <settings.hpp>
#include <nvm.hpp>
#include <color.hpp>
#include <strip.hpp>

#include <FastLED.h>

#include <vector>
#include <array>

namespace strip {
    using std::vector;
    using color::Color;

    CRGB ul_leds[strip::len];

    vector<Color> leds;

    void setup(vector<Color> strip_state) {
        FastLED.addLeds<NEOPIXEL, settings::DATA_PIN>(ul_leds, strip::len);

        leds = strip_state;
    }

    void show() {
        printf("[strip] sending the color info to the strip\n");
        printf("[strip] ");

        int i = 0;
        for (auto &color : leds) {
            ul_leds[i++] = CRGB(color);
            printf("%s %s", color.to_escape_seq_background().c_str(), color::ESCAPE_SEQ_RESET);
        }

        printf("\n");

        FastLED.show();
    }
}