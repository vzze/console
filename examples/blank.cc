#include "../src/console.hh"

void KeyCallback(char key) {
    // Do something with the pressed key
}
#ifdef _WIN32
// gets called if either mouse moves or mouse button gets pressed
void MouseButtonCallback(const bool mouse_buttons[5], std::size_t mouseX, std::size_t mouseY) {
    // MouseButton1 = mouse_buttons[0]
    // ...
}
#endif
// gets called before the main thread starts
bool Init(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y) {
    console::grid::for_each_0(pixels, X, Y, [](console::Pixel & p) {
        p.fg = console::COLORS::FG_BLACK;
        p.bg = console::COLORS::BG_BLACK;
    });
    console::grid::set_string(pixels, "Hello World!", console::COLORS::FG_WHITE, console::COLORS::BG_BLACK, X - 1, 0, X);
    return true;
}

// gets called every loop in the main thread
bool Update(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y, float deltaTime) {

    return true;
}

int main() {
    console::init();

    console::set_init_callback(Init);
    console::set_update_callback(Update);
    console::set_key_callback(KeyCallback);
#ifdef _WIN32
    console::set_mouse_callback(MouseButtonCallback);
#endif
    console::run();
    console::exit();
}
