#include "console.hh"

#include <chrono>
#include <thread>
#include <iostream>

#define ESC "\x1b"
#define CSI ESC "["

#define ALTERNATE_BUFFER CSI "?1049h"
#define MAIN_BUFFER      CSI "?1049l"

#define HIDE_CURSOR      CSI "?25l"
#define BUFFER_POSITION  CSI "2;1f"
#define TITLE_SETTINGS   CSI "1;1f" CSI "30;47m"
#define SHOW_CURSOR      CSI "?25h"
#define SOFT_RESET       CSI "!p"

console::Pixel::Pixel(col::FG _fg, col::BG _bg, char _display) : fg(_fg), bg(_bg), display(_display) {}

#ifdef _WIN32
HANDLE console::_impl::_hOut;
HANDLE console::_impl::_hIn;

DWORD console::_impl::_oldhOut;
DWORD console::_impl::_oldhIn;
#endif
std::atomic_bool console::_impl::_failed_exit = false;

std::atomic_size_t console::_impl::_consoleX = 0;
std::atomic_size_t console::_impl::_consoleY = 0;
#ifdef _WIN32
std::atomic_size_t console::_impl::_mouseX = 0;
std::atomic_size_t console::_impl::_mouseY = 0;
#endif
std::atomic_char console::_impl::_current_key = 0;

console::_impl::_buffer console::_impl::_pbuf;

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t, float)>
console::_impl::_update_callback = [](std::vector<console::Pixel> &, std::size_t, std::size_t, float) -> bool {
    return true;
};

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t)>
console::_impl::_init_callback = [](std::vector<console::Pixel> &, std::size_t, std::size_t) -> bool {
    return true;
};

std::function<void(char)> console::_impl::_key_callback = [](char) -> void {};
#ifdef _WIN32
bool console::_impl::_mouse_pressed_buttons[5] = { false };

std::function<void(const bool *, std::size_t, std::size_t)>
console::_impl::_mouse_callback = [](const bool *, std::size_t, std::size_t) -> void {};

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
    INPUT_RECORD buf[32];
    DWORD read, i;

    int mb = 0;
#elif defined(__unix__)
    struct winsize w;

    struct termios oldsets, newsets;

    int res;

    char c;

    fd_set set;
    struct timeval tv;

    tcgetattr(fileno(stdin), &oldsets);

    newsets.c_lflag &= (~ICANON & ~ECHO);

    tcsetattr(fileno(stdin), TCSANOW, &newsets);
#endif
    while(true) {
        if(_failed_exit)
            return;
#ifdef _WIN32
        GetNumberOfConsoleInputEvents(_hIn, &read);
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
                    _consoleY = static_cast<std::size_t>(buf[i].Event.WindowBufferSizeEvent.dwSize.Y - 1);
                break;
            }
#elif defined(__unix__)
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        _consoleX = w.ws_col;
        _consoleY = w.ws_row - 1;

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
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

    int counter = 0;

    std::string title;
    std::string buffer;

    const char * fg_code = nullptr;
    const char * bg_code = nullptr;

    while(true) {
        if(_failed_exit)
            return;

        t1 = std::chrono::high_resolution_clock::now();
        fps = t1 - t2;

        dFps = fps.count();
        counter++;

        if(dFps >= 1.0F / 30.0F) {
#ifdef _WIN32
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
            " - X: "           + std::to_string(_consoleX - 1)                               +
            " Y: "             + std::to_string(_consoleY)                                   +
            " - KEY: "         + std::to_string(_current_key)                                +
            " - MOUSE: X: "    + std::to_string(_mouseX)                                     +
            " Y: "             + std::to_string(_mouseY);
#elif defined(__unix__)
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
            " - X: "           + std::to_string(_consoleX - 1)                               +
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
            for(auto & p : _pbuf._current) {
                if(fg_code != _fg_colors[static_cast<std::uint8_t>(p.fg)]) {
                    buffer += _fg_colors[static_cast<std::uint8_t>(p.fg)];
                    fg_code = _fg_colors[static_cast<std::uint8_t>(p.fg)];
                }

                if(bg_code != _bg_colors[static_cast<std::uint8_t>(p.bg)]) {
                    buffer += _bg_colors[static_cast<std::uint8_t>(p.bg)];
                    bg_code = _bg_colors[static_cast<std::uint8_t>(p.bg)];
                }

                buffer += p.display;
            }
        }

        std::cout << BUFFER_POSITION << buffer << TITLE_SETTINGS << title;

        fg_code = bg_code = nullptr;
    }
}

int console::init() {
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
#endif
    std::cout << ALTERNATE_BUFFER HIDE_CURSOR;

    return 1;
}
#ifdef _WIN32
void console::set_mouse_callback(std::function<void(const bool *, std::size_t, std::size_t)> f) {
    _impl::_mouse_callback = f;
}
#endif
void console::set_key_callback(std::function<void(char)> f) {
    _impl::_key_callback = f;
}

void console::set_update_callback(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t, double)> f) {
    _impl::_update_callback = f;
}

void console::set_init_callback(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t)> f) {
    _impl::_init_callback = f;
}

void console::run() {
#ifdef _WIN32
    if(!SetConsoleCtrlHandler(console::_impl::_ctrlhandler, TRUE))
        return;
#endif
    std::vector<Pixel> pixels = {};

    std::thread input_controller(_impl::_updateinputs);
    input_controller.detach();

    std::cout << TITLE_SETTINGS "Loading...";
    std::this_thread::sleep_for(std::chrono::milliseconds(128));

    pixels.resize(_impl::_consoleX * _impl::_consoleY);

    if(!_impl::_init_callback(pixels, _impl::_consoleX, _impl::_consoleY))
        return;

    _impl::_pbuf._next = _impl::_pbuf._current = pixels;

    std::thread renderer(_impl::_draw);
    renderer.detach();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> time;
    float dTime;

    while(true) {
        if(_impl::_failed_exit)
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

        if(!_impl::_update_callback(pixels, _impl::_consoleX, _impl::_consoleY, dTime))
            break;

        {
            std::scoped_lock lck(_impl::_pbuf._mut_write);
            for(auto & p : pixels) {
                _impl::_pbuf._next[static_cast<std::size_t>(&p - pixels.begin().base())] = p;
            }
        }

        {
            std::scoped_lock lck(_impl::_pbuf._mut_write, _impl::_pbuf._mut_read);
            std::swap(_impl::_pbuf._current, _impl::_pbuf._next);
        }
    }

    if(renderer.joinable()) renderer.join();
    if(input_controller.joinable()) input_controller.join();

    std::cout << SOFT_RESET SHOW_CURSOR MAIN_BUFFER; // only switch to main buffer after every thread has finished their job
}

int console::exit() {
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

void console::grid::set_string(std::vector<Pixel> & pixels, std::string_view str, col::FG fg, col::BG bg, std::size_t x, std::size_t y, std::size_t X) {
    set_string(pixels, str, fg, bg, at_2D(x, y, X));
}

void console::grid::set_string(std::vector<Pixel> & pixels, std::string_view str, col::FG fg, col::BG bg, std::size_t pos) {
    static std::size_t i;
    if(str.size() > pixels.size() - pos)
        for(i = 0; i < pixels.size() - pos; ++i) {
            pixels[pos + i].display = str[i];
            pixels[pos + i].bg = bg;
            pixels[pos + i].fg = fg;
        }
    else
        for(i = 0; i < str.size(); ++i) {
            pixels[pos + i].display = str[i];
            pixels[pos + i].bg = bg;
            pixels[pos + i].fg = fg;
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
