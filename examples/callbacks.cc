#include <console.hh>

console instance{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool key(const char key) {
    if(key == 'q') return false;

    instance.set_cursor_pos({1, 1});
    instance.write("Pressed: ");
    instance.write(key);

    return instance.refresh();
}

bool resize(const console::coord coords) {
    instance.set_cursor_pos({1, 2});
    instance.write(std::format("Current console size: {}/{}", coords.row, coords.column));

    return instance.refresh();
}

bool notify_alive() {
    static int counter = 0;
    instance.set_cursor_pos({1, 3});
    instance.write(std::format("UI has been notified {} times.", counter));

    ++counter;

    return instance.refresh();
}

int main() {
    constexpr auto notify_timer = std::chrono::milliseconds{200};

    instance.add_key_callback(key);
    instance.add_resize_callback(resize);
    instance.add_notify_alive_callback(notify_alive);

    instance.main_loop(notify_timer);
}
