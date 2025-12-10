#ifndef __WINOGRONA_BLE_HPP__
#define __WINOGRONA_BLE_HPP__

#include <color.hpp>
#include <vector>


namespace ble {
    inline constexpr const char *SERVICE_UUID = "90CF3289-436B-4281-A074-6F8C56A4E6C6";
    inline constexpr const char *COLOR_CHARACTERISTIC_UUID = "E2BE7B62-69C6-4DAD-9014-CA3FDD062DB1";
    inline constexpr const char *LENGTH_CHARACTERISTIC_UUID = "DA20E0D7-EAED-45BA-B21E-948D2F67F887";

    extern bool is_initialized;
    void setup(std::vector<color::Color> strip_state);
    void save_last_color(color::Color color);
    color::Color load_last_color();
}

#endif