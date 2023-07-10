#include <console.hh>

console::console() noexcept : should_exit{false}, old_sets{} {
    struct termios new_sets{};

    tcgetattr(fileno(stdin), &new_sets);

    old_sets = new_sets;

    new_sets.c_lflag &= static_cast<unsigned>(~ICANON & ~ECHO); // NOLINT(hicpp-signed-bitwise)

    tcsetattr(fileno(stdin), TCSANOW, &new_sets);
}

void console::process_events() noexcept {
    struct winsize window{};

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window); // NOLINT(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

    for(const auto & callback : resize_callbacks)
        if(!callback({ window.ws_col, window.ws_row })) should_exit = true;

    struct timeval time{0, 0};

    fd_set set{};

    FD_ZERO(&set); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    FD_SET(fileno(stdin), &set); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index, hicpp-signed-bitwise)

    const auto res = select(fileno(stdin) + 1, &set, nullptr, nullptr, &time);

    if(res > 0) {
        char key{};
        [[maybe_unused]] const auto x = read(fileno(stdin), &key, 1);

        for(const auto & callback : key_callbacks)
            if(!callback(key)) should_exit = true;
    }
}

console::~console() noexcept {
    tcsetattr(fileno(stdin), TCSANOW, &old_sets);
}
