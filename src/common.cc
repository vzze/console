#include <console.hh>

bool console::refresh() noexcept {
    try {
        std::cout << buffer << std::flush;
        buffer.clear();
        return true;
    } catch(...) { return false; }
}

void console::init_alternate_buffer() noexcept {
    std::ios_base::sync_with_stdio(false);

    write("\x1b[?1049h");
    write("\x1b[?25l");

    should_exit = !refresh();
}

void console::main_buffer() noexcept {
    write("\x1b[?1049l");
    write("\x1b[!p");
    write("\x1b[?25h");

    refresh();
}

void console::main_loop() noexcept {
    init_alternate_buffer();

    while(!should_exit) {
        process_events();

        std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }

    main_buffer();
}

void console::main_loop(const std::chrono::milliseconds timer) noexcept {
    init_alternate_buffer();

    auto time_point1 = std::chrono::high_resolution_clock::now();
    auto time_point2 = std::chrono::high_resolution_clock::now();

    std::chrono::milliseconds passed{0};

    while(!should_exit) {
        time_point1 = std::chrono::high_resolution_clock::now();
        passed += std::chrono::duration_cast<std::chrono::milliseconds>(time_point1 - time_point2);
        time_point2 = time_point1;

        process_events();

        if(!should_exit && passed >= timer) {
            passed -= timer;

            for(const auto & callback : notify_alive_callbacks)
                if(!callback()) {
                    should_exit = true;
                    break;
                }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds{1});
        ++passed;
    }

    main_buffer();
}
