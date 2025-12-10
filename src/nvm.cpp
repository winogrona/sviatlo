#include <nvm.hpp>
#include <color.hpp>
#include <strip.hpp>

#include <vector>

#include <Preferences.h>

namespace nvm {
    using std::vector;

    bool is_initialized = false;
    Preferences prefs;

    bool setup_nvm() {
        is_initialized = true;

        if (!prefs.begin(NVM_NAMESPACE_NAME, false /* false = RW mode */)) {
            return false;
        }

        return true;
    }

    void save_last_strip_state(vector<color::Color> colors) {
        printf("[nvm] saving a new strip state: ");

        for (auto color : colors) {
            printf("%x%x%x;", color.r, color.g, color.b);
        }

        printf("\n");

        int i = 0;
        uint8_t bytes[strip::len * 3] = {0};
        for (auto color : colors) {
            bytes[i++] = color.r;
            bytes[i++] = color.g;
            bytes[i++] = color.b;
        }

        prefs.putBytes("last_state", bytes, strip::len * 3);
    }

    vector<color::Color> load_last_strip_state() {
        vector<color::Color> colors(strip::len);
        size_t expected = strip::len * 3;
        std::vector<uint8_t> buf(expected, 0);

        // read up to expected bytes; getBytes returns number of bytes actually read
        size_t read = prefs.getBytes("last_state", buf.data(), expected);

        size_t idx = 0;
        for (size_t i = 0; i + 2 < read && idx < strip::len; i += 3, ++idx) {
            colors[idx].r = buf[i];
            colors[idx].g = buf[i + 1];
            colors[idx].b = buf[i + 2];
        }

        return colors;
    }
}