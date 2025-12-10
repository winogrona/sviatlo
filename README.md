# sviatlo
esp32-powered WIP led strip firmware with custom JS effects controlled over Bletooth Low Energy (BLE).

# Setup
Import the directory in PlatformIO IDE, edit include/settings.hpp to customize the advertised device name, the data gpio pin number and stuff like that.

# Effect example
```js
name = "Green"
author = "winogrona"
version = 1
description = "A test effect"

print("Hello from green.elk!")

let loop = function() {
    for (let i = 0; i < LEN; i++) {
        set(i, 0x00FF00)
    }

    update()
    sleep(1000)
    print("Still alive!")
}
```

# API Reference 

```js
/// Debug print
print(...)

/// 24bit uint color from rgb values
rgb.make(r: Number, g: Number, b: Number): Number

/// 24bit uint color from hsv values
hsv.make(h: Number, s: Number, v: Number): Number

/// Get the red value
rgb.r(color: Number): Number

/// Get the green value
rgb.g(color: Number): Number

/// Get the blue value
rgb.b(color: Number): Number

/// Get the HSV hue
hsv.h(color: Number): Number

/// Get the HSV saturation
hsv.s(color: Number): Number

/// Get the HSV value
hsv.v(color: Number): Number

/// Add a slider parameter
slider(
    id: String,
    display_name: String,
    description: String,
    default: Number,
    min: Number,
    max: Number,
    step: Number
)

/// Add a color selector parameter
colorwheel(
    id: String,
    display_name: String,
    description: String,
    default: Number
)

/// Get a parameter at runtime
param.<param_name>

/// Set a pixel value
set(index: Number, color: Number)

/// Update the strip
update()

/// Sleep for N milliseconds
sleep(millis: Number)

/// Get strip size
```
size()

