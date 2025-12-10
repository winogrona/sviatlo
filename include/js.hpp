#include <string>
#include <array>

namespace js {
    inline constexpr const char* TASK_NAME = "jstask";
    inline constexpr size_t TASK_STACK_SIZE = 4096;
    inline constexpr int TASK_PRIORITY = 1;
    inline constexpr int TASK_CORE = 1;

    inline constexpr int HEAP_SIZE = 40960;

    using std::string;

    void setup_js();
    void run_effect(string &code);
    void stop_effect();

    inline constexpr std::array<const char*, 8> elk_type_names = {
        "Undefined", "Null", "Bool (true)", "Bool (false)", "String", "Number", "Error", "Priv"
    };

    inline constexpr const char* elk_typename_to_string(int type) {
        return elk_type_names[static_cast<size_t>(type)];
    }
}