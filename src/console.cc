#include <console.hh>

console::Pixel::Pixel(
    col::FG _fg,
    col::BG _bg,
    char _disp,
    col::INVERT _inv,
    col::BOLD _b,
    col::ITALIC _i,
    col::UNDERLINE _u,
    col::STRIKETHROUGH _s
) noexcept {
    values[0] = static_cast<bool>(_s);
    values[1] = static_cast<bool>(_u);
    values[2] = static_cast<bool>(_i);
    values[3] = static_cast<bool>(_b);
    values[4] = static_cast<bool>(_inv);

    display = _disp;

    for(std::uint8_t i = 0; i < 5; ++i) {
        values[i + 5U]  = static_cast<std::uint8_t>(_bg) >> i & 0b1;
        values[i + 10U] = static_cast<std::uint8_t>(_fg) >> i & 0b1;
    }
}

console::col::FG console::Pixel::fg() const noexcept {
    return static_cast<col::FG>(static_cast<std::uint8_t>(values.to_ulong() >> 10U));
}

console::col::BG console::Pixel::bg() const noexcept {
    return static_cast<col::BG>(static_cast<std::uint8_t>(values.to_ulong() >> 5U & 0b11111));
}

char console::Pixel::displayed() const noexcept {
    return display;
}

console::col::INVERT console::Pixel::inverted() const noexcept {
    return static_cast<col::INVERT>(static_cast<std::uint8_t>(values[4]));
}

console::col::BOLD console::Pixel::bold() const noexcept {
    return static_cast<col::BOLD>(static_cast<std::uint8_t>(values[3]));
}

console::col::ITALIC console::Pixel::italic() const noexcept {
    return static_cast<col::ITALIC>(static_cast<std::uint8_t>(values[2]));
}

console::col::UNDERLINE console::Pixel::underline() const noexcept {
    return static_cast<col::UNDERLINE>(static_cast<std::uint8_t>(values[1]));
}

console::col::STRIKETHROUGH console::Pixel::strikethrough() const noexcept {
    return static_cast<col::STRIKETHROUGH>(static_cast<std::uint8_t>(values[0]));
}

void console::Pixel::fg(col::FG _fg) noexcept {
    for(std::size_t i = 0; i < 5; ++i)
        values[i + 10U] = static_cast<std::uint8_t>(_fg) >> i & 0b1;
}

void console::Pixel::bg(col::BG _bg) noexcept {
    for(std::size_t i = 0; i < 5; ++i)
        values[i + 5U] = static_cast<std::uint8_t>(_bg) >> i & 0b1;
}

void console::Pixel::displayed(char _disp) noexcept {
    display = _disp;
}

void console::Pixel::inverted(col::INVERT _inv) noexcept {
    values[4] = static_cast<bool>(_inv);
}

void console::Pixel::bold(col::BOLD _b) noexcept {
    values[3] = static_cast<bool>(_b);
}

void console::Pixel::italic(col::ITALIC _i) noexcept {
    values[2] = static_cast<bool>(_i);
}

void console::Pixel::underline(col::UNDERLINE _u) noexcept {
    values[1] = static_cast<bool>(_u);
}
void console::Pixel::strikethrough(col::STRIKETHROUGH _s) noexcept {
    values[0] = static_cast<bool>(_s);
}

#ifdef _WIN32
HANDLE                  console::_impl::_hOut;
HANDLE                  console::_impl::_hIn;
DWORD                   console::_impl::_oldhOut;
DWORD                   console::_impl::_oldhIn;
#endif
std::atomic_bool        console::_impl::_failed_exit = false;
std::atomic_bool        console::_impl::_draw_title  = true;
std::atomic_size_t      console::_impl::_consoleX    = 0;
std::atomic_size_t      console::_impl::_consoleY    = 0;
#ifdef _WIN32
std::atomic_size_t      console::_impl::_mouseX  = 0;
std::atomic_size_t      console::_impl::_mouseY  = 0;
std::atomic_bool        console::_impl::_focus_c = true;
#endif
std::atomic_char        console::_impl::_current_key = 0;
console::_impl::_buffer console::_impl::_pbuf;

std::atomic<console::col::FG>            console::_impl::_title_fg  = console::col::FG::WHITE;
std::atomic<console::col::BG>            console::_impl::_title_bg  = console::col::BG::DONT_REPLACE;
std::atomic<console::col::INVERT>        console::_impl::_title_inv = console::col::INVERT::DONT_REPLACE;
std::atomic<console::col::BOLD>          console::_impl::_title_b   = console::col::BOLD::DONT_REPLACE;
std::atomic<console::col::ITALIC>        console::_impl::_title_i   = console::col::ITALIC::DONT_REPLACE;
std::atomic<console::col::UNDERLINE>     console::_impl::_title_u   = console::col::UNDERLINE::DONT_REPLACE;
std::atomic<console::col::STRIKETHROUGH> console::_impl::_title_s   = console::col::STRIKETHROUGH::DONT_REPLACE;

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t, float)>
console::_impl::_update_callback = [](std::vector<console::Pixel> &, std::size_t, std::size_t, float) -> bool { return true; };

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t)>
console::_impl::_init_callback = [](std::vector<console::Pixel> &, std::size_t, std::size_t) -> bool { return true; };

std::function<void(std::size_t, std::size_t)> console::_impl::_resize_callback = [](std::size_t, std::size_t) -> void {};
std::function<void(char)> console::_impl::_key_callback = [](char) -> void {};
#ifdef _WIN32
bool console::_impl::_mouse_pressed_buttons[5] = { false };
std::function<void(const bool[5], std::size_t, std::size_t)>
console::_impl::_mouse_callback = [](const bool *, std::size_t, std::size_t) -> void {};
std::function<void(bool)> console::_impl::_focus_callback = [](bool) -> void {};

BOOL console::_impl::_ctrlhandler(DWORD ctrltype) {
    switch(ctrltype) {
        case CTRL_C_EVENT:        _failed_exit = true; return true; break;
        case CTRL_CLOSE_EVENT:    _failed_exit = true; return true; break;
        case CTRL_BREAK_EVENT:    _failed_exit = true; return true; break;
        case CTRL_LOGOFF_EVENT:   _failed_exit = true; return true; break;
        case CTRL_SHUTDOWN_EVENT: _failed_exit = true; return true; break;
        default:                  _failed_exit = true; return true; break;
    }
}
#endif
void console::_impl::_updateinputs() {
#ifdef _WIN32
    INPUT_RECORD * buf;
    DWORD read, i;

    std::size_t mb = 0;
#elif defined(__unix__)
    struct winsize w;

    struct termios oldsets, newsets;

    std::int64_t res;

    char c;

    fd_set set;
    struct timeval tv;

    tcgetattr(fileno(stdin), &oldsets);

    newsets.c_lflag &= static_cast<unsigned>(~ICANON & ~ECHO);

    tcsetattr(fileno(stdin), TCSANOW, &newsets);
#endif
    while(true) {
        if(_failed_exit) [[unlikely]]
            break;
#ifdef _WIN32
        GetNumberOfConsoleInputEvents(_hIn, &read);
        buf = new INPUT_RECORD[read];
        ReadConsoleInput(_hIn, buf, read, &read);

        for(i = 0; i < read; ++i)
            switch(buf[i].EventType) {
                case KEY_EVENT:
                    if(buf[i].Event.KeyEvent.bKeyDown) {
                        _current_key = buf[i].Event.KeyEvent.uChar.AsciiChar;
                        _key_callback(buf[i].Event.KeyEvent.uChar.AsciiChar);
                    }
                break;
                case MOUSE_EVENT:
                    switch(buf[i].Event.MouseEvent.dwEventFlags) {
                        case MOUSE_MOVED:
                            _mouseX = static_cast<std::size_t>(buf[i].Event.MouseEvent.dwMousePosition.X);
                            _mouseY = static_cast<std::size_t>(buf[i].Event.MouseEvent.dwMousePosition.Y);
                            _mouse_callback(_mouse_pressed_buttons, _mouseX, _mouseY);
                        break;
                        case 0:
                            for(mb = 0; mb < 5; ++mb)
                                _mouse_pressed_buttons[mb] = (buf[i].Event.MouseEvent.dwButtonState & (1 << mb)) > 0;
                            _mouse_callback(_mouse_pressed_buttons, _mouseX, _mouseY);
                        break;
                    }
                break;
                case WINDOW_BUFFER_SIZE_EVENT:
                    _consoleX = static_cast<std::size_t>(buf[i].Event.WindowBufferSizeEvent.dwSize.X);
                    _consoleY = static_cast<std::size_t>(buf[i].Event.WindowBufferSizeEvent.dwSize.Y);
                    _resize_callback(_consoleX, _consoleY);
                break;
                case FOCUS_EVENT:
                    _focus_c = buf[i].Event.FocusEvent.bSetFocus;
                    _focus_callback(_focus_c);
                break;
            }
        delete[] buf;
#elif defined(__unix__)
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        if(_consoleX != w.ws_col || _consoleY != w.ws_row) {
            _consoleX = w.ws_col;
            _consoleY = w.ws_row;
            _resize_callback(_consoleX, _consoleY);
        }

        tv.tv_sec  = 10;
        tv.tv_usec = 0;

        FD_ZERO(&set);
        FD_SET(fileno(stdin), &set);

        res = select(fileno(stdin) + 1, &set, NULL, NULL, &tv);

        if(res > 0) {
            [[maybe_unused]] auto x = read(fileno(stdin), &c, 1);
            _current_key = c;
            _key_callback(c);
        }
#endif
    }
#ifdef __unix__
    tcsetattr(fileno(stdin), TCSANOW, &oldsets);
#endif
}

void console::_impl::_draw() {
    std::ios_base::sync_with_stdio(false);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

    std::chrono::duration<float> fps;
    float dFps;

    std::int32_t counter = 0;

    std::string title;
    std::string buffer;

    const char * fg_code = nullptr;       std::uint8_t fg;
    const char * bg_code = nullptr;       std::uint8_t bg;
    const char * pos_ne  = nullptr;       std::uint8_t pn;
    const char * bold    = nullptr;       std::uint8_t b;
    const char * italic  = nullptr;       std::uint8_t i;
    const char * underline = nullptr;     std::uint8_t u;
    const char * strikethrough = nullptr; std::uint8_t s;

    while(true) {
        if(_failed_exit) [[unlikely]]
            break;

        t1 = std::chrono::high_resolution_clock::now();
        fps = t1 - t2;

        dFps = fps.count();
        counter++;

        if(dFps >= 1.0F / 30.0F) {
#ifdef _WIN32
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
            " - "              + std::to_string(_focus_c)                                    +
            " - X: "           + std::to_string(_consoleX)                                   +
            " Y: "             + std::to_string(_consoleY)                                   +
            " - KEY: "         + std::to_string(_current_key)                                +
            " - MOUSE: X: "    + std::to_string(_mouseX)                                     +
            " Y: "             + std::to_string(_mouseY);
#elif defined(__unix__)
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
            " - X: "           + std::to_string(_consoleX)                                   +
            " Y: "             + std::to_string(_consoleY)                                   +
            " - KEY: "         + std::to_string(_current_key);
#endif
            counter = 0;
            t2 = t1;
        }

        if(title.size() != _consoleX)
            title.resize(_consoleX, ' ');

        buffer.clear();

        {
            std::scoped_lock lck(_pbuf._mut_read);

            if(_draw_title)
                grid::set_string(
                    _pbuf._current, title, _title_fg, _title_bg,
                    _title_inv, _title_b, _title_i,
                    _title_u, _title_s, 0
                );

            for(auto & p : _pbuf._current) {
                fg = static_cast<std::uint8_t>(p.fg());
                bg = static_cast<std::uint8_t>(p.bg());
                pn = static_cast<std::uint8_t>(p.inverted());
                b = static_cast<std::uint8_t>(p.bold());
                i = static_cast<std::uint8_t>(p.italic());
                u = static_cast<std::uint8_t>(p.underline());
                s = static_cast<std::uint8_t>(p.strikethrough());

                if(fg_code != _fg_colors[fg]) {
                    buffer += _fg_colors[fg];
                    fg_code = _fg_colors[fg];
                }

                if(bg_code != _bg_colors[bg]) {
                    buffer += _bg_colors[bg];
                    bg_code = _bg_colors[bg];
                }

                if(pos_ne != _pos_ne[pn]) {
                    buffer += _pos_ne[pn];
                    pos_ne = _pos_ne[pn];
                }

                if(bold != _bold[b]) {
                    buffer += _bold[b];
                    bold = _bold[b];
                }

                if(italic != _italic[i]) {
                    buffer += _italic[i];
                    italic = _italic[i];
                }

                if(underline != _underline[u]) {
                    buffer += _underline[u];
                    underline = _underline[u];
                }

                if(strikethrough != _strikethrough[s]) {
                    buffer += _strikethrough[s];
                    strikethrough = _strikethrough[s];
                }

                buffer += p.displayed();
            }
        }

        std::cout << seq::BUFFER_POSITION << buffer;

        fg_code = bg_code = pos_ne = bold = italic = underline = strikethrough = nullptr;
    }
}

std::int32_t console::init() {
#ifdef _WIN32
    _impl::_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if(_impl::_hOut == INVALID_HANDLE_VALUE) return 0;

    DWORD dwMode = 0;

    if(!GetConsoleMode(_impl::_hOut, &dwMode)) return 0;

    _impl::_oldhOut = dwMode;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

    if(!SetConsoleMode(_impl::_hOut, dwMode)) return 0;

    _impl::_hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(!GetConsoleMode(_impl::_hIn, &dwMode)) return 0;

    _impl::_oldhIn = dwMode;

    dwMode = ENABLE_EXTENDED_FLAGS;
    dwMode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;

    if(!SetConsoleMode(_impl::_hIn, dwMode)) return 0;

    if(!SetConsoleCtrlHandler(console::_impl::_ctrlhandler, TRUE)) return 0;
#endif
    std::cout << seq::ALTERNATE_BUFFER << seq::HIDE_CURSOR;

    return 1;
}
#ifdef _WIN32
void console::set_mouse_callback (std::function<void(const bool *, std::size_t, std::size_t)> f) { _impl::_mouse_callback = f; }
void console::set_focus_callback (std::function<void(bool)> f) { _impl::_focus_callback = f; }
#endif
void console::set_resize_callback(std::function<void(std::size_t, std::size_t)> f) { _impl::_resize_callback = f; }
void console::set_key_callback   (std::function<void(char)> f) { _impl::_key_callback = f; }
void console::set_update_callback(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t, double)> f) { _impl::_update_callback = f; }
void console::set_init_callback  (std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t)> f) { _impl::_init_callback = f; }

void console::run() {
    std::vector<Pixel> pixels = {};

    std::thread input_controller(_impl::_updateinputs);
    input_controller.detach();

    while(_impl::_consoleX == 0 || _impl::_consoleY == 0) {}

    pixels.resize(_impl::_consoleX * _impl::_consoleY);

    if(!_impl::_init_callback(pixels, _impl::_consoleX, _impl::_consoleY)) [[unlikely]]
        return;

    _impl::_pbuf._next = _impl::_pbuf._current = pixels;

    std::thread renderer(_impl::_draw);
    renderer.detach();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> time;
    float dTime;

    while(true) {
        if(_impl::_failed_exit) [[unlikely]]
            break;

        t1 = std::chrono::high_resolution_clock::now();
        time = t1 - t2;

        t2 = t1;
        dTime = time.count();

        if(_impl::_consoleX * _impl::_consoleY != pixels.size()) [[unlikely]] {
            pixels.resize(_impl::_consoleX * _impl::_consoleY);

            std::scoped_lock lck(_impl::_pbuf._mut_write, _impl::_pbuf._mut_read);
            _impl::_pbuf._next.resize(pixels.size());
            _impl::_pbuf._current.resize(pixels.size());
        }

        if(!_impl::_update_callback(pixels, _impl::_consoleX, _impl::_consoleY, dTime)) [[unlikely]]
            break;

        {
            std::scoped_lock lck(_impl::_pbuf._mut_write);
            for(auto & p : pixels) {
                _impl::_pbuf._next[static_cast<std::size_t>(&p - &*pixels.begin())] = p;
            }
        }

        {
            std::scoped_lock lck(_impl::_pbuf._mut_write, _impl::_pbuf._mut_read);
            std::swap(_impl::_pbuf._current, _impl::_pbuf._next);
        }
    }

    if(renderer.joinable()) renderer.join();
    if(input_controller.joinable()) input_controller.join();

    std::cout.flush();

    std::cout << seq::SOFT_RESET << seq::SHOW_CURSOR << seq::MAIN_BUFFER; // only switch to main buffer after every thread has finished their job
}

void console::toggle_title() {
    _impl::_draw_title = !_impl::_draw_title;
}

bool console::title_state() {
    return _impl::_draw_title;
}

void console::set_title_options(
    col::FG fg, col::BG bg, col::INVERT inv, col::BOLD b,
    col::ITALIC i, col::UNDERLINE u, col::STRIKETHROUGH s
) {
    _impl::_title_fg = fg;
    _impl::_title_bg = bg;
    _impl::_title_inv = inv;
    _impl::_title_b = b;
    _impl::_title_i = i;
    _impl::_title_u = u;
    _impl::_title_s = s;
}

std::int32_t console::exit() {
    _impl::_failed_exit = true;
#ifdef _WIN32
    if(!SetConsoleMode(_impl::_hOut, _impl::_oldhOut))
        return 1;

    if(!SetConsoleMode(_impl::_hIn, _impl::_oldhIn))
        return 1;
#endif
    return 0;
}

console::Pixel & console::grid::at_2D(std::vector<Pixel> & pixels, std::size_t x, std::size_t y, std::size_t X) { return pixels.at(y * X + x); }

std::size_t console::grid::at_2D(std::size_t x, std::size_t y, std::size_t X) noexcept { return y * X + x; }

void console::grid::set_string(
    std::vector<Pixel> & pixels, std::string_view str, col::FG fg, col::BG bg,
    col::INVERT inv, col::BOLD b, col::ITALIC i, col::UNDERLINE u, col::STRIKETHROUGH s,
    std::size_t x, std::size_t y, std::size_t X
) {
    set_string(pixels, str, fg, bg, inv, b, i, u, s, at_2D(x, y, X));
}

void console::grid::set_string(
    std::vector<Pixel> & pixels, std::string_view str, col::FG fg, col::BG bg,
    col::INVERT inv, col::BOLD b, col::ITALIC _i, col::UNDERLINE u, col::STRIKETHROUGH s,
    std::size_t pos
) {
    static std::size_t i;
    if(str.size() > pixels.size() - pos)
        for(i = 0; i != pixels.size() - pos; ++i) {
            pixels[pos + i].displayed(str[i]);

            if(bg != col::BG::DONT_REPLACE           ) pixels[pos + i].bg(bg           ) ;
            if(fg != col::FG::DONT_REPLACE           ) pixels[pos + i].fg(fg           ) ;
            if(inv != col::INVERT::DONT_REPLACE      ) pixels[pos + i].inverted(inv    ) ;
            if(b != col::BOLD::DONT_REPLACE          ) pixels[pos + i].bold(b          ) ;
            if(_i != col::ITALIC::DONT_REPLACE       ) pixels[pos + i].italic(_i       ) ;
            if(u != col::UNDERLINE::DONT_REPLACE     ) pixels[pos + i].underline(u     ) ;
            if(s != col::STRIKETHROUGH::DONT_REPLACE ) pixels[pos + i].strikethrough(s ) ;
        }
    else
        for(i = 0; i != str.size(); ++i) {
            pixels[pos + i].displayed(str[i]);

            if(bg != col::BG::DONT_REPLACE           ) pixels[pos + i].bg(bg           ) ;
            if(fg != col::FG::DONT_REPLACE           ) pixels[pos + i].fg(fg           ) ;
            if(inv != col::INVERT::DONT_REPLACE      ) pixels[pos + i].inverted(inv    ) ;
            if(b != col::BOLD::DONT_REPLACE          ) pixels[pos + i].bold(b          ) ;
            if(_i != col::ITALIC::DONT_REPLACE       ) pixels[pos + i].italic(_i       ) ;
            if(u != col::UNDERLINE::DONT_REPLACE     ) pixels[pos + i].underline(u     ) ;
            if(s != col::STRIKETHROUGH::DONT_REPLACE ) pixels[pos + i].strikethrough(s ) ;
        }
}


void console::grid::for_each_0(std::vector<Pixel> & pixels, std::size_t X, std::size_t Y, std::function<void (Pixel &)> func) {
    static std::size_t x, y; x = y = 0;

    while(y != Y) {
        while(x != X) {
            func(pixels[y * X + x]);
            ++x;
        }
        ++y;
        x = 0;
    }
}

void console::grid::for_each_90(std::vector<Pixel> & pixels, std::size_t X, std::size_t Y, std::function<void (Pixel &)> func) {
    static std::size_t x, y; x = X - 1; y = 0;

    while(x >= 0) {
        while(y != Y) {
            func(pixels[y * X + x]);
            ++y;
        }
        --x;
        y = 0;
    }
}

void console::grid::for_each_180(std::vector<Pixel> & pixels, std::size_t X, std::size_t Y, std::function<void (Pixel &)> func) {
    static std::size_t x, y; x = X - 1; y = Y - 1;

    while(y >= 0) {
        while(x >= 0) {
            func(pixels[y * X + x]);
            --x;
        }
        --y;
        x = X - 1;
    }
}

void console::grid::for_each_270(std::vector<Pixel> & pixels, std::size_t X, std::size_t Y, std::function<void (Pixel &)> func) {
    static std::size_t x, y; x = 0; y = Y - 1;

    while(x != X) {
        while(y >= 0) {
            func(pixels[y * X + x]);
            --y;
        }
        x++;
        y = Y - 1;
    }
}
