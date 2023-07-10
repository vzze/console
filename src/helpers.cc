#include <console.hh>

void console::set_cursor_pos(const coord coords) {
    write(std::format("\x1b[{};{}H", coords.row, coords.column));
}

void console::insert_char(const std::uint32_t count) {
    write(std::format("\x1b[{}@", count));
}

void console::delete_char(const std::uint32_t count) {
    write(std::format("\x1b[{}P", count));
}

void console::erase_char(const std::uint32_t count) {
    write(std::format("\x1b[{}X", count));
}

void console::insert_line(const std::uint32_t count) {
    write(std::format("\x1b[{}L", count));
}

void console::delete_line(const std::uint32_t count) {
    write(std::format("\x1b[{}M", count));
}

void console::erase_in_display(const DISPLAY mode) {
    write(std::format("\x1b[{}J", static_cast<std::uint32_t>(mode)));
}

void console::erase_in_line(const LINE mode) {
    write(std::format("\x1b[{}K", static_cast<std::uint32_t>(mode)));
}

void console::dec_mode() {
    write("\x1b(0");
}

void console::ascii_mode() {
    write("\x1b(B");
}
