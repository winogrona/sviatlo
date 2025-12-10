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
Effect examples

/// green.elk


/// solid_color.elk


/// rainbow.elk

*/

namespace js {
    using std::vector;
    using std::string;
    using color::Color;
    using color::HSV;

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

        jsval_t jsi_rgb_r(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: rgb.r(color: Number)");
            }

            uint32_t color = (uint32_t) js_getnum(args[0]);

            uint8_t r = (color >> 16) & 0xFF;
            return js_mknum((double) r);
        }

        jsval_t jsi_rgb_g(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: rgb.g(color: Number)");
            }

            uint32_t color = (uint32_t) js_getnum(args[0]);
            
            uint8_t g = (color >> 8) & 0xFF;
            return js_mknum((double) g);
        }

        jsval_t jsi_rgb_b(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: rgb.b(color: Number)");
            }

            uint32_t color = (uint32_t) js_getnum(args[0]);
            
            uint8_t b = (color) & 0xFF;
            return js_mknum((double) b);
        }

        jsval_t jsi_hsv_make(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 3) {
                return js_mkerr(js, "wrong number of arguments. expected 3: hsv.make(h: Number, s: Number, v: Number)");
            }

            double h = js_getnum(args[0]);
            double s = js_getnum(args[1]);
            double v = js_getnum(args[2]);

            color::Color color = color::Color(color::HSV{h, s, v});

            return js_mknum((double) color.to_uint32());
        }

         jsval_t jsi_hsv_h(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: hsv.h(color: Number)");
            }

            color::Color color = (uint32_t) js_getnum(args[0]);

            return js_mknum((double) color.to_hsv().h);
        }

        jsval_t jsi_hsv_s(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: hsv.s(color: Number)");
            }

            color::Color color = (uint32_t) js_getnum(args[0]);

            return js_mknum((double) color.to_hsv().s);
        }

        jsval_t jsi_hsv_v(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: hsv.v(color: Number)");
            }

            color::Color color = (uint32_t) js_getnum(args[0]);

            return js_mknum((double) color.to_hsv().v);
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
                return js_mkerr(js, "wrong number of arguments. expected 2: set(index: Number, color: Number)");
            }

            size_t i = (size_t) js_getnum(args[0]);

            if (i >= strip::len) {
                return js_mkerr(js, "index out of bounds");
            }

            strip::leds[i] = color::Color((uint32_t) js_getnum(args[1]));

            return js_mknull();
        }

        jsval_t jsi_get(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 1) {
                return js_mkerr(js, "wrong number of arguments. expected 1: get(index: Number)");
            }

            size_t i = (size_t) js_getnum(args[0]);

            if (i >= strip::len) {
                return js_mkerr(js, "index out of bounds");
            }

            return js_mknum((double) strip::leds[i].to_uint32());
        }

        jsval_t jsi_update(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 0) {
                return js_mkerr(js, "wrong number of arguments. expected 0: update()");
            }

            strip::show();

            return js_mknull();
        }

        jsval_t jsi_millis(struct js *js, jsval_t *args, int nargs) {
            if (nargs != 0) {
                return js_mkerr(js, "wrong number of arguments. expected 0: millis()");
            }

            return js_mknum((double) millis());
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
        js_set(js, js_glob(js), "millis", js_mkfun(js_interface::jsi_millis));

        jsval_t rgb = js_mkobj(js);
        js_set(js, rgb, "make", js_mkfun(js_interface::jsi_rgb_make));
        js_set(js, rgb, "r", js_mkfun(js_interface::jsi_rgb_r));
        js_set(js, rgb, "g", js_mkfun(js_interface::jsi_rgb_g));
        js_set(js, rgb, "b", js_mkfun(js_interface::jsi_rgb_b));
        js_set(js, js_glob(js), "rgb", rgb);

        jsval_t hsv = js_mkobj(js);
        js_set(js, rgb, "make", js_mkfun(js_interface::jsi_hsv_make));
        js_set(js, rgb, "h", js_mkfun(js_interface::jsi_hsv_h));
        js_set(js, rgb, "s", js_mkfun(js_interface::jsi_hsv_s));
        js_set(js, rgb, "v", js_mkfun(js_interface::jsi_hsv_v));
        js_set(js, js_glob(js), "hsv", rgb);

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