#include <js.hpp>
#include <strip.hpp>
#include <panic.hpp>
#include <logging.hpp>

#include <string>
#include <vector>
#include <array>
#include <optional>

#include <stdio.h>

#include <Arduino.h>

using std::string;
using std::vector;


    /*
    API functions:

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
    size()


    Effect examples

    /// green.elk

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

    /// solid_color.elk

    name = "Test"
    author = "winogrona"
    version = 1
    description = "A test effect"

    slider("brightness", "Brightness", "Brightness of the color", 0, 1, 0.01)
    colorwheel("color", "Color", "Color of the effect", 0xff8525)

    let loop = function() {
        for (let i = 0; i < LEN; i++) {
            r = rgb.r(param.color) * param.brightness
            g = rgb.g(param.color) * param.brightness
            b = rgb.b(param.color) * param.brightness

            set(i, rgb.make(r, g, b))
        }

        update()
    }

    /// rainbow.elk

    name = "Rainbow"
    author = "winogrona"
    version = 1
    description = "A test effect"

    for (let i = 0; i < LEN; i++) {
        h = i / LEN
        s = 1 / LEN
        v = 1 / LEN
    }

    let loop = function() {
        for (let i = 0; i < LEN; i++) {
            r = rgb.r(param.color)
        }
    }
    */

namespace js {
    #include <elk.h>

    TaskHandle_t jsTask = NULL;

    std::optional<std::function<void()>> on_effect_loaded_callback = {};
    std::optional<std::function<void()>> on_effect_error_callback = {};

    char *js_memory = NULL;
    bool effect_running = false;
    char const *script = NULL;
    logging::CircularBuffer<string> log_buffer(100);

    inline constexpr const char LOOP_FUNCTION_CALL[] = "loop();";
    inline constexpr const char VALUE_PLACEHOLDER[] = "__PLACEHOLDER__";

    static inline void push_to_effect_log(string text) {
        log_buffer.push("[" + std::to_string(millis()) + "] " + text);
    }

    string jsval_format(struct js *js, jsval_t val) {
        int type = js_type(val);
        char *str;
        unsigned int len = 0;

        switch (type) {
                    case JS_UNDEF:
                        return "undefined";

                    case JS_NULL:
                        return "null";

                    case JS_TRUE:
                        return "true";

                    case JS_FALSE:
                        return "false";

                    case JS_ERR:
                        return string(js_str(js, val)); // I KNOW for sure it doesn't allocate space on the heap in that exact case
                        // There's no other way to get the error text

                    case JS_STR:
                        return "\"" + string(js_getstr(js, val, NULL)) + "\"";
                    
                    case JS_NUM:
                        return std::to_string(js_getnum(val));

                    case JS_PRIV:
                        return "[something]";
                    
                    default:
                        return "[unknown (bug)]";
                }
        
        return "[unreachable]";
    }

    extern "C" {
    namespace js_interface {
        jsval_t jsi_print(struct js *js, jsval_t *args, int nargs) {
            string result = "";

            for (int i = 0; i < nargs; i++) {
                result += jsval_format(js, args[i]) + " ";
            }

            printf("[js.elk_io] %s \n", result.c_str());
            push_to_effect_log(result);

            return js_mknull();
        }

        jsval_t jsi_rgb_make(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 3) {
                return js_mkerr(js, "wrong number of arguments. expected 3: rgb.make(r: Number, g: Number, b: Number)");
            }

            int r = (int)js_getnum(args[0]);
            int g = (int)js_getnum(args[1]);
            int b = (int)js_getnum(args[2]);

            return js_mknum((r << 16) | (g << 8) | b);
        }

        jsval_t jsi_sleep(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: sleep(millis: Number)");
            }

            delay((int)js_getnum(args[0]));

            return js_mknull();
        }

        jsval_t jsi_set(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 2) {
                return js_mkerr(js, "wrong number of arguments. expected 2: set(pixel: Number, color: Number)");
            }

            size_t i = (size_t) js_getnum(args[0]);

            if (i >= strip::len) {
                return js_mkerr(js, "index out of bounds");
            }

            strip::leds[i] = color::Color((uint32_t) js_getnum(args[1]));

            return js_mknull();
        }

        jsval_t jsi_update(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 0) {
                return js_mkerr(js, "wrong number of arguments. expected 0: update()");
            }

            strip::show();

            return js_mknull();
        }
    }}

    void crash() {
        effect_running = false;

        if (on_effect_error_callback.has_value()) {
            printf("[js] executing the effect error callback function\n");
            on_effect_error_callback.value()();
        } else {
            printf("[js] no effect error callback function defined\n");
        }

        printf("[js.effect_task] exiting\n");
        vTaskDelete(NULL); // kms
    }

    void jsi_init(js *js) {
        js_set(js, js_glob(js), "print", js_mkfun(js_interface::jsi_print));
        js_set(js, js_glob(js), "set", js_mkfun(js_interface::jsi_set));
        js_set(js, js_glob(js), "sleep", js_mkfun(js_interface::jsi_sleep));
        js_set(js, js_glob(js), "update", js_mkfun(js_interface::jsi_update));

        jsval_t rgb = js_mkobj(js);
        js_set(js, rgb, "make", js_mkfun(js_interface::jsi_rgb_make));
        js_set(js, js_glob(js), "rgb", rgb);

        js_set(js, js_glob(js), "name", js_mkstr(js, VALUE_PLACEHOLDER, sizeof(VALUE_PLACEHOLDER) - 1));
        js_set(js, js_glob(js), "author", js_mkstr(js, VALUE_PLACEHOLDER, sizeof(VALUE_PLACEHOLDER) - 1));
        js_set(js, js_glob(js), "version", js_mkstr(js, VALUE_PLACEHOLDER, sizeof(VALUE_PLACEHOLDER) - 1));
        js_set(js, js_glob(js), "description", js_mkstr(js, VALUE_PLACEHOLDER, sizeof(VALUE_PLACEHOLDER) - 1));

        js_set(js, js_glob(js), "LEN", js_mknum(strip::len));
    }

    void _run_effect(void *params) {
        printf("[js.effect_task] resetting the heap memory\n");
        memset(js_memory, 0, HEAP_SIZE);

        struct js *js = js_create(js_memory, HEAP_SIZE);
        jsi_init(js);

        printf("[js.effect_task] executing the script\n");

        push_to_effect_log("initializing the effect");
        jsval_t v = js_eval(js, script, strlen(script));

        const char *result = js_str(js, v);

        printf("[js.effect_task] script returned %s\n", result);
        if (js_type(v) == JS_ERR) {
            printf("[js.effect_task] effect crashed with an error: %s\n", result);
            push_to_effect_log(string("Fatal error occured during execution: ") + string(result));

            crash();
            return;
        }

        if (on_effect_loaded_callback.has_value()) {
            printf("[js.effect_task] executing the effect loaded callback function\n");
            on_effect_loaded_callback.value()();
        } else {
            printf("[js.effect_task] no effect loaded callback function defined\n");
        }

        printf("[js.effect_task] effect loaded. executing the loop function\n");
        push_to_effect_log("starting the effect");
        effect_running = true;

        while (true) {
            printf("[js.effect_task] executing the loop function\n");
            v = js_eval(js, LOOP_FUNCTION_CALL, sizeof(LOOP_FUNCTION_CALL) - 1);

            if (js_type(v) == JS_ERR) {
                const char *result = js_str(js, v);
                printf("[js.effect_task] effect crashed with an error: %s\n", result);
                push_to_effect_log(string("Fatal error occured during execution: ") + string(result));
                break;
            }
        }

        crash();
    }

    void run_effect(string &code) {
        script = strdup(code.c_str());

        if (!effect_running) {
            printf("[js] creating the effect task\n");
            xTaskCreatePinnedToCore(
                _run_effect,
                TASK_NAME,
                TASK_STACK_SIZE,
                NULL, // params
                TASK_PRIORITY,
                &jsTask,
                TASK_CORE
            );
        } else {
            printf("[js] run_effect called, but an effect is already running\n");
        }
    }

    void stop_effect() {
        if (jsTask != NULL) {
            printf("[js] deleting the effect task\n");
            vTaskDelete(jsTask);
            free((void *) script);
        } else {
            printf("[js] stop_effect called, but no effect is running\n");
        }
    }

    void setup_js() {
        js_memory = (char *) malloc(HEAP_SIZE);

        if (js_memory == NULL) {
            printf("[js] Failed to allocate the memory for the heap\n");
            panic::panic();
        }

        printf("[js] Allocated %d bytes for the Elk heap\n", HEAP_SIZE);
    }
}