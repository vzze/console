#include "console.hh"

#include <chrono>
#include <thread>
#include <iostream>

#define ESC "\x1b"
#define CSI "\x1b["
#define OSC "\x1b]"
#define ST  "\x7"

#define FG_BLACK    CSI "30m"
#define FG_RED      CSI "31m"
#define FG_GREEN    CSI "32m"
#define FG_YELLOW   CSI "33m"
#define FG_BLUE     CSI "34m"
#define FG_MAGENTA  CSI "35m"
#define FG_CYAN     CSI "36m"
#define FG_WHITE    CSI "37m"

#define BG_BLACK    CSI "40m"
#define BG_RED      CSI "41m"
#define BG_GREEN    CSI "42m"
#define BG_YELLOW   CSI "43m"
#define BG_BLUE     CSI "44m"
#define BG_MAGENTA  CSI "45m"
#define BG_CYAN     CSI "46m"
#define BG_WHITE    CSI "47m"

#define FG_BRIGHT_BLACK    CSI "90m"
#define FG_BRIGHT_RED      CSI "91m"
#define FG_BRIGHT_GREEN    CSI "92m"
#define FG_BRIGHT_YELLOW   CSI "93m"
#define FG_BRIGHT_BLUE     CSI "94m"
#define FG_BRIGHT_MAGENTA  CSI "95m"
#define FG_BRIGHT_CYAN     CSI "96m"
#define FG_BRIGHT_WHITE    CSI "97m"

#define BG_BRIGHT_BLACK    CSI "100m"
#define BG_BRIGHT_RED      CSI "101m"
#define BG_BRIGHT_GREEN    CSI "102m"
#define BG_BRIGHT_YELLOW   CSI "103m"
#define BG_BRIGHT_BLUE     CSI "104m"
#define BG_BRIGHT_MAGENTA  CSI "105m"
#define BG_BRIGHT_CYAN     CSI "106m"
#define BG_BRIGHT_WHITE    CSI "107m"

#define ALTERNATE_BUFFER CSI "?1049h"
#define MAIN_BUFFER      CSI "?1049l"

#define HIDE_CURSOR     CSI "?25l"
#define BUFFER_POSITION CSI "2;1f"
#define TITLE_SETTINGS  CSI "1;1f" CSI "30;47m"
#define SHOW_CURSOR     CSI "?25h"
#define SOFT_RESET      CSI "!p"

console::Pixel::Pixel(COLORS _fg, COLORS _bg, char _display) : fg(_fg), bg(_bg), display(_display) {}

const char * console::_impl::_colors[] = {
    FG_BLACK,
    FG_RED,
    FG_GREEN,
    FG_YELLOW,
    FG_BLUE,
    FG_MAGENTA,
    FG_CYAN,
    FG_WHITE,

    BG_BLACK,
    BG_RED,
    BG_GREEN,
    BG_YELLOW,
    BG_BLUE,
    BG_MAGENTA,
    BG_CYAN,
    BG_WHITE,

    FG_BRIGHT_BLACK,
    FG_BRIGHT_RED,
    FG_BRIGHT_GREEN,
    FG_BRIGHT_YELLOW,
    FG_BRIGHT_BLUE,
    FG_BRIGHT_MAGENTA,
    FG_BRIGHT_CYAN,
    FG_BRIGHT_WHITE,

    BG_BRIGHT_BLACK,
    BG_BRIGHT_RED,
    BG_BRIGHT_GREEN,
    BG_BRIGHT_YELLOW,
    BG_BRIGHT_BLUE,
    BG_BRIGHT_MAGENTA,
    BG_BRIGHT_CYAN,
    BG_BRIGHT_WHITE,
};
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
console::_impl::_update = [](std::vector<console::Pixel> &, std::size_t, std::size_t, float) -> bool {
    return true;
};

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t)>
console::_impl::_init = [](std::vector<console::Pixel> &, std::size_t, std::size_t) -> bool {
    return true;
};

std::function<void(char)> console::_impl::_keycallback = [](char) -> void {};
#ifdef _WIN32
bool console::_impl::_mpressedbuttons[5] = { false };
std::function<void(const bool *, std::size_t, std::size_t)> console::_impl::_mousebuttons = [](const bool *, std::size_t, std::size_t) -> void {};

BOOL console::_impl::_ctrlhandler(DWORD ctrltype) {
    switch(ctrltype) {
        case CTRL_C_EVENT:
            _failed_exit = true;
            return true;
        break;
        case CTRL_CLOSE_EVENT:
            _failed_exit = true;
            return true;
        break;
        case CTRL_BREAK_EVENT:
            _failed_exit = true;
            return true;
        break;
        case CTRL_LOGOFF_EVENT:
            _failed_exit = true;
            return true;
        break;
        case CTRL_SHUTDOWN_EVENT:
            _failed_exit = true;
            return true;
        break;
        default:
            _failed_exit = true;
            return true;
    }
}
#endif
void console::_impl::_updateinputs() {
#ifdef _WIN32
    INPUT_RECORD buf[32];
    DWORD read;

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
        ReadConsoleInput(_hIn, buf, 32, &read);

        for(DWORD i = 0; i < read; ++i)
            switch(buf[i].EventType) {
                case KEY_EVENT:
                    if(buf[i].Event.KeyEvent.bKeyDown) {
                        _current_key = buf[i].Event.KeyEvent.uChar.AsciiChar;
                        _keycallback(buf[i].Event.KeyEvent.uChar.AsciiChar);
                    }
                break;
                case MOUSE_EVENT:
                    switch(buf[i].Event.MouseEvent.dwEventFlags) {
                        case MOUSE_MOVED:
                            _mouseX = static_cast<std::size_t>(buf[i].Event.MouseEvent.dwMousePosition.X);
                            _mouseY = static_cast<std::size_t>(buf[i].Event.MouseEvent.dwMousePosition.Y);
                            _mousebuttons(_mpressedbuttons, _mouseX, _mouseY);
                        break;
                        case 0:
                            for(mb = 0; mb < 5; ++mb)
                                _mpressedbuttons[mb] = (buf[i].Event.MouseEvent.dwButtonState & (1 << mb)) > 0;
                            _mousebuttons(_mpressedbuttons, _mouseX, _mouseY);
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
            read(fileno(stdin), &c, 1);
            _current_key = c;
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
                    " - X: " + std::to_string(_consoleX - 1) +
                    " Y: " + std::to_string(_consoleY) +
                    + " - KEY: " + std::to_string(_current_key) +
                    " - MOUSE: X: " + std::to_string(_mouseX) +
                    + " Y: " + std::to_string(_mouseY);
#elif defined(__unix__)
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
                    " - X: " + std::to_string(_consoleX - 1) +
                    " Y: " + std::to_string(_consoleY)
                    + " - KEY: " + std::to_string(_current_key);
#endif
            counter = 0;
            t2 = t1;
        }

        if(title.size() != _consoleX)
            title.resize(_consoleX, ' ');

        buffer.clear();

        {
            std::scoped_lock lck(_pbuf.mut_read);
            for(auto & p : _pbuf.current) {
                if(fg_code != _colors[static_cast<int>(p.fg)]) {
                    buffer += _colors[static_cast<int>(p.fg)];
                    fg_code = _colors[static_cast<int>(p.fg)];
                }

                if(bg_code != _colors[static_cast<int>(p.bg)]) {
                    buffer += _colors[static_cast<int>(p.bg)];
                    bg_code = _colors[static_cast<int>(p.bg)];
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

    if(_impl::_hOut == INVALID_HANDLE_VALUE)
        return 1;

    DWORD dwMode = 0;

    if(!GetConsoleMode(_impl::_hOut, &dwMode))
        return 1;

    _impl::_oldhOut = dwMode;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

    if(!SetConsoleMode(_impl::_hOut, dwMode))
        return 1;

    _impl::_hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(!GetConsoleMode(_impl::_hIn, &dwMode))
        return 1;

    _impl::_oldhIn = dwMode;

    dwMode = ENABLE_EXTENDED_FLAGS;
    dwMode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;

    if(!SetConsoleMode(_impl::_hIn, dwMode))
        return 1;
#endif
    std::cout << ALTERNATE_BUFFER HIDE_CURSOR;

    return 0;
}
#ifdef _WIN32
void console::set_mouse_callback(std::function<void(const bool *, std::size_t, std::size_t)> f) {
    _impl::_mousebuttons = f;
}
#endif
void console::set_key_callback(std::function<void(char)> f) {
    _impl::_keycallback = f;
}

void console::set_update_callback(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t, double)> f) {
    _impl::_update = f;
}

void console::set_init_callback(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t)> f) {
    _impl::_init = f;
}

void console::run() {
#ifdef _WIN32
    if(!SetConsoleCtrlHandler(console::_impl::_ctrlhandler, TRUE))
        return;

    _impl::_hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(_impl::_hIn == INVALID_HANDLE_VALUE)
        _impl::_failed_exit = true;
#endif
    std::vector<Pixel> pixels = {};

    std::thread input_controller(_impl::_updateinputs);
    input_controller.detach();

    std::cout << TITLE_SETTINGS "Loading...";
    std::this_thread::sleep_for(std::chrono::milliseconds(128));

    pixels.resize(_impl::_consoleX * _impl::_consoleY);

    if(!_impl::_init(pixels, _impl::_consoleX, _impl::_consoleY))
        return;

    _impl::_pbuf.next = _impl::_pbuf.current = pixels;

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

            std::scoped_lock lck(_impl::_pbuf.mut_write, _impl::_pbuf.mut_read);
            _impl::_pbuf.next.resize(pixels.size());
            _impl::_pbuf.current.resize(pixels.size());
        }

        if(!_impl::_update(pixels, _impl::_consoleX, _impl::_consoleY, dTime))
            break;

        {
            std::scoped_lock lck(_impl::_pbuf.mut_write);
            for(auto & p : pixels) {
                _impl::_pbuf.next[static_cast<std::size_t>(&p - pixels.begin().base())] = p;
            }
        }

        {
            std::scoped_lock lck(_impl::_pbuf.mut_write, _impl::_pbuf.mut_read);
            std::swap(_impl::_pbuf.current, _impl::_pbuf.next);
        }
    }
}

int console::exit() {
    _impl::_failed_exit = true;
    std::cout << SOFT_RESET SHOW_CURSOR MAIN_BUFFER;
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

void console::grid::set_string(std::vector<Pixel> & pixels, std::string_view str, COLORS fg, COLORS bg, std::size_t x, std::size_t y, std::size_t X) {
    set_string(pixels, str, fg, bg, at_2D(x, y, X));
}

void console::grid::set_string(std::vector<Pixel> & pixels, std::string_view str, COLORS fg, COLORS bg, std::size_t pos) {
    if(str.size() > pixels.size() - pos)
        for(std::size_t i = 0; i < pixels.size() - pos; ++i) {
            pixels[pos + i].display = str[i];
            pixels[pos + i].bg = bg;
            pixels[pos + i].fg = fg;
        }
    else
        for(std::size_t i = 0; i < str.size(); ++i) {
            pixels[pos + i].display = str[i];
            pixels[pos + i].bg = bg;
            pixels[pos + i].fg = fg;
        }
}

void console::grid::for_each_0(std::vector<Pixel> & pixels, std::size_t X, std::size_t Y, std::function<void (Pixel &)> func) {
    std::size_t x = 0, y = 0;

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
    std::size_t x = X - 1, y = 0;

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
    std::size_t x = X - 1, y = Y - 1;

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
    std::size_t x = 0, y = Y - 1;

    while(x != X) {
        while(y >= 0) {
            func(pixels[y * X + x]);
            --y;
        }
        x++;
        y = Y - 1;
    }
}
