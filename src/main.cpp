#define FASTLED_ESP32_I2S
#include <FastLED.h>

#include <ble.hpp>
#include <settings.hpp>
#include <nvm.hpp>
#include <strip.hpp>
#include <panic.hpp>
#include <js.hpp>

#include <Arduino.h>

using std::string;
using std::vector;

void setup() { 
    Serial.begin(115200);

    printf("[main] winogrona strop 01 v0.0.2\n");

    if (! nvm::setup_nvm()) {
      printf("[main] failed to setup nvm.\n");
      panic::panic();
    }

    printf("[main] nvm is up.\n");

    auto colors = nvm::load_last_strip_state();


    printf("[main] restored a strip state: ");
    for (auto color : colors) {
      printf("%x%x%x;", color.r, color.g, color.b);
    }
    printf("\n");

    // Requires nvm
    strip::setup(colors);
    strip::show();
    printf("[main] the strip is up.\n");

    // Requires nvm
    ble::setup(colors);
    printf("[main] ble is up.\n");

    js::setup_js();
    printf("[main] js is up.\n");

    string script = R"(
name = "Rainbow";
author = "winogrona";
version = 1;
description = "A rainbow effect";

let h;
let s = 1;
let v = 1;

for (let i = 0; i < LEN; i++) {
  h = i / LEN;
  set(i, hsv.make(h, s, v));
  print("LED ", i, ": H=", h, " S=", s, " V=", v);
  print(" Color: ", rgb.r(get(i)), ",", rgb.g(get(i)), ",", rgb.b(get(i)));
}

update();

let last_time = millis();
let cycle_ms = 10000;

let loop = function() {
  let elapsed = millis() - last_time;

  if (elapsed > cycle_ms) {
    last_time = millis();
    return;
  }

  for (let i = 0; i < LEN; i++) {
    color = get(i)
    h = hsv.h() + (elapsed / cycle_ms);

    if (h > 1) {
      h = h - 1;
    }

    set(i, hsv.make(h, s, v));
  }

  update();
}
)";

    printf("[main] js test: atteming to execute: '%s'.\n", script.c_str());

    js::run_effect(script);

    printf("[main] setup done.\n");
}

void loop() { 
  delay(1000);
}