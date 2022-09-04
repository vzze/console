#include "console.hh"

#define ESC "\x1b"
#define CSI "\x1b["
#define OSC "\x1b]"
#define ST  "\x7"

#define BLACK    CSI "30;40m "
#define RED      CSI "31;41m "
#define GREEN    CSI "32;42m "
#define YELLOW   CSI "33;43m "
#define BLUE     CSI "34;44m "
#define MAGENTA  CSI "35;45m "
#define CYAN     CSI "36;46m "
#define WHITE    CSI "37;47m "

#define BRIGHT_BLACK    CSI "90;100m "
#define BRIGHT_RED      CSI "91;101m "
#define BRIGHT_GREEN    CSI "92;102m "
#define BRIGHT_YELLOW   CSI "93;103m "
#define BRIGHT_BLUE     CSI "94;104m "
#define BRIGHT_MAGENTA  CSI "95;105m "
#define BRIGHT_CYAN     CSI "96;106m "
#define BRIGHT_WHITE    CSI "97;107m "

#define ALTERNATE_BUFFER CSI "?1049h"
#define MAIN_BUFFER      CSI "?1049l"

#define HIDE_CURSOR     CSI "?25l"
#define BUFFER_POSITION CSI "2;1f"
#define TITLE_SETTINGS  CSI "1;1f" CSI "30;47m"
#define SHOW_CURSOR     CSI "?25h"
#define SOFT_RESET      CSI "!p"

console::Pixel::Pixel(console::COLORS _col) : color(static_cast<int>(_col)) {}

const char * console::_colors[] = {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE,
};

HANDLE console::_hOut;
HANDLE console::_hIn;

DWORD console::_oldhOut;
DWORD console::_oldhIn;

std::atomic_bool console::_failed_exit = false;

std::atomic_size_t console::_consoleX = 0;
std::atomic_size_t console::_consoleY = 0;

std::atomic_size_t console::_mouseX = 0;
std::atomic_size_t console::_mouseY = 0;

std::atomic_char console::_current_key = 0;

console::_buffer console::_pbuf;

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t, float)>
console::_update = [](std::vector<console::Pixel> &, std::size_t, std::size_t, float) -> bool {
    return true;
};

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t)>
console::_init = [](std::vector<console::Pixel> &, std::size_t, std::size_t) -> bool {
    return true;
};

std::function<void(char)> console::_keycallback = [](char) -> void {};

bool console::_mpressedbuttons[5] = { false };
std::function<void(const bool *, std::size_t, std::size_t)> console::_mousebuttons = [](const bool *, std::size_t, std::size_t) -> void {};

int console::Init() {
    _hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if(_hOut == INVALID_HANDLE_VALUE)
        return 1;

    DWORD dwMode = 0;

    if(!GetConsoleMode(_hOut, &dwMode))
        return 1;

    _oldhOut = dwMode;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;

    if(!SetConsoleMode(_hOut, dwMode))
        return 1;

    _hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(!GetConsoleMode(_hIn, &dwMode))
        return 1;

    _oldhIn = dwMode;

    dwMode = ENABLE_EXTENDED_FLAGS;
    dwMode |= ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;

    if(!SetConsoleMode(_hIn, dwMode))
        return 1;

    std::cout << ALTERNATE_BUFFER HIDE_CURSOR;

    return 0;
}

BOOL console::_ctrlhandler(DWORD ctrltype) {
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

void console::_updateinputs() {
    INPUT_RECORD buf[32];
    DWORD read;

    int mb = 0;

    while(true) {
        if(_failed_exit)
            return;

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
                            _mouseX = buf[i].Event.MouseEvent.dwMousePosition.X;
                            _mouseY = buf[i].Event.MouseEvent.dwMousePosition.Y;
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
                    _consoleX = buf[i].Event.WindowBufferSizeEvent.dwSize.X;
                    _consoleY = buf[i].Event.WindowBufferSizeEvent.dwSize.Y - 1;
                break;
            }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void console::_draw() {
    std::ios_base::sync_with_stdio(false);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;

    std::chrono::duration<float> fps;
    float dFps;

    int counter = 0;

    std::string title;
    std::string buffer;

    while(true) {
        if(_failed_exit)
            return;

        t1 = std::chrono::high_resolution_clock::now();
        fps = t1 - t2;

        dFps = fps.count();
        counter++;

        if(dFps >= 1.0F / 30.0F) {
            title = "V - FPS " + std::to_string((1.0F / dFps) * static_cast<float>(counter)) +
                    " - X: " + std::to_string(_consoleX) +
                    " Y: " + std::to_string(_consoleY) +
                    + " - KEY: " + std::to_string(_current_key) +
                    " - MOUSE: X: " + std::to_string(_mouseX) +
                    + " Y: " + std::to_string(_mouseY);

            counter = 0;
            t2 = t1;
        }

        if(title.size() != _consoleX)
            title.resize(_consoleX, ' ');

        buffer.clear();

        {
            std::scoped_lock lck(_pbuf.mut_read);
            for(auto & p : _pbuf.current)
                buffer += _colors[p.color];
        }

        std::cout << BUFFER_POSITION << buffer << TITLE_SETTINGS << title;
    }
}

void console::SetMouseCallbackFunc(std::function<void(const bool *, std::size_t, std::size_t)> f) {
    _mousebuttons = f;
}

void console::SetKeyCallbackFunc(std::function<void(char)> f) {
    _keycallback = f;
}

void console::SetUpdateFunc(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t, double)> f) {
    _update = f;
}

void console::SetInitFunc(std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t)> f) {
    _init = f;
}

void console::Run() {
    if(!SetConsoleCtrlHandler(console::_ctrlhandler, TRUE))
        return;

    _hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(_hIn == INVALID_HANDLE_VALUE)
        _failed_exit = true;

    std::vector<Pixel> pixels = {};

    std::thread input_controller(_updateinputs);
    input_controller.detach();

    std::cout << TITLE_SETTINGS "Loading...";
    std::this_thread::sleep_for(std::chrono::milliseconds(128));

    pixels.resize(_consoleX * _consoleY);

    if(!_init(pixels, _consoleX, _consoleY))
        return;

    _pbuf.next = _pbuf.current = pixels;

    std::thread renderer(_draw);
    renderer.detach();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> time;
    float dTime;

    while(true) {
        if(_failed_exit)
            break;

        t1 = std::chrono::high_resolution_clock::now();
        time = t1 - t2;

        t2 = t1;
        dTime = time.count();

        if(_consoleX * _consoleY != pixels.size()) [[unlikely]] {
            pixels.resize(_consoleX * _consoleY);

            std::scoped_lock lck(_pbuf.mut_write, _pbuf.mut_read);
            _pbuf.next.resize(pixels.size());
            _pbuf.current.resize(pixels.size());
        }

        if(!_update(pixels, _consoleX, _consoleY, dTime))
            break;

        {
            std::scoped_lock lck(_pbuf.mut_write);
            for(auto & p : pixels) {
                _pbuf.next[&p - pixels.begin().base()] = p;
            }
        }

        {
            std::scoped_lock lck(_pbuf.mut_write, _pbuf.mut_read);
            std::swap(_pbuf.current, _pbuf.next);
        }
    }
}

int console::Exit() {
    std::cout << SOFT_RESET SHOW_CURSOR MAIN_BUFFER;

    if(!SetConsoleMode(_hOut, _oldhOut))
        return 1;

    if(!SetConsoleMode(_hIn, _oldhIn))
        return 1;

    return 0;
}
