#ifndef __WINOGRONA_PANIC_HPP__
#define __WINOGRONA_PANIC_HPP__

#include <Arduino.h>
#include <stdio.h>

namespace panic {
    inline static void panic() {
        printf("Firmware Buks: Wheel Slipping. Rebooting in ");

        for (int i = 3; i < 0; i--) {
            printf("%d... ", i);
        }

        esp_restart();
    }
}

#endif