#include <console.hh>

void ResizeCallback([[maybe_unused]] std::size_t X, [[maybe_unused]] std::size_t Y) {
    // gets called only if X or Y are modified
}

void KeyCallback([[maybe_unused]] char key) {
    // Do something with the pressed key
}
#ifdef _WIN32
// gets called if either mouse moves or mouse button gets pressed
void MouseButtonCallback([[maybe_unused]] const bool mouse_buttons[5], [[maybe_unused]] std::size_t mouseX, [[maybe_unused]] std::size_t mouseY) {
    // MouseButton1 = mouse_buttons[0]
    // ...
}

void FocusCallback([[maybe_unused]] bool focused) {
    // disable something if terminal is focused or not
}
#endif
// gets called before the main thread starts
bool Init(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y) {
    console::toggle_title();

    console::grid::for_each_0(pixels, X, Y, [](console::Pixel & p) {
        p.fg(console::col::FG::BLACK);
        p.bg(console::col::BG::BLACK);
    });

    console::grid::set_string(
        pixels, "Hello World! This is a test for the new string feature",
        console::col::FG::WHITE, console::col::BG::BLACK,
        console::col::INVERT::DONT_REPLACE, console::col::BOLD::DONT_REPLACE,
        console::col::ITALIC::YES, console::col::UNDERLINE::DONT_REPLACE, console::col::STRIKETHROUGH::DONT_REPLACE,
        0, 0, X
    );
    console::grid::set_string(
        pixels, "underlined",
        console::col::FG::WHITE, console::col::BG::BLACK,
        console::col::INVERT::DONT_REPLACE, console::col::BOLD::DONT_REPLACE,
        console::col::ITALIC::DONT_REPLACE, console::col::UNDERLINE::YES, console::col::STRIKETHROUGH::DONT_REPLACE,
        0, 1, X
    );
    console::grid::set_string(
        pixels, "strikethrough",
        console::col::FG::WHITE, console::col::BG::BLACK,
        console::col::INVERT::DONT_REPLACE, console::col::BOLD::DONT_REPLACE,
        console::col::ITALIC::DONT_REPLACE, console::col::UNDERLINE::DONT_REPLACE, console::col::STRIKETHROUGH::YES,
        0, 2, X
    );
    return true;
}

// gets called every loop in the main thread
bool Update([[maybe_unused]] std::vector<console::Pixel> & pixels, [[maybe_unused]] std::size_t X, [[maybe_unused]] std::size_t Y, [[maybe_unused]] float deltaTime) {

    return true;
}

int main() {
    if(!console::init())
        return 0;

    console::set_init_callback(Init);
    console::set_update_callback(Update);
    console::set_resize_callback(ResizeCallback);
    console::set_key_callback(KeyCallback);
#ifdef _WIN32
    console::set_mouse_callback(MouseButtonCallback);
    console::set_focus_callback(FocusCallback);
#endif
    console::run();
    return console::exit();
}
