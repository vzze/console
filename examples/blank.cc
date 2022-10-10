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
bool Update(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y, float deltaTime) {

    return true;
}

int main() {
    if(!console::init())
        return 0;

    console::set_init_callback(Init);
    console::set_update_callback(Update);
    console::set_key_callback(KeyCallback);
#ifdef _WIN32
    console::set_mouse_callback(MouseButtonCallback);
#endif
    console::run();
    return console::exit();
}
