#include <console.hh>

bool console::refresh() noexcept {
    try {
        std::cout << buffer << std::flush;
        buffer.clear();
        return true;
    } catch(...) { return false; }
}

void console::main_loop() noexcept {
    std::ios_base::sync_with_stdio(false);

    write("\x1b[?1049h");
    write("\x1b[?25l");

    should_exit = !refresh();

    while(!should_exit) {
        process_events();

        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }

    write("\x1b[?1049l");
    write("\x1b[!p");
    write("\x1b[?25h");

    refresh();
}
