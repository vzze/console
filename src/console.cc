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

const char * console::_COLORS[] = {
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

#define ALTERNATE_BUFFER CSI "?1049h"
#define MAIN_BUFFER      CSI "?1049l"

console::Pixel::Pixel(console::COLORS _col) : color(static_cast<int>(_col)) {}

HANDLE console::_hOut;
HANDLE console::_hIn;

DWORD console::_oldhOut;
DWORD console::_oldhIn;

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
    dwMode &= ~(ENABLE_PROCESSED_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT);

    if(!SetConsoleMode(_hIn, dwMode))
        return 1;

    std::cout << ALTERNATE_BUFFER;

    // Hide the cursor
    std::cout << CSI "?25l";

    return 0;
}

std::atomic_bool console::_failed_exit = false;

std::atomic_size_t console::_consoleX = 0;
std::atomic_size_t console::_consoleY = 0;

std::mutex console::_mut = {};
std::string console::_buffer = {};

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

std::atomic_size_t console::_mouseX;
std::atomic_size_t console::_mouseY;
std::atomic_char console::_current_key;

void console::UpdateInputs() {
    INPUT_RECORD buf[32];
    DWORD read;

    while(true) {
        if(_failed_exit)
            return;

        ReadConsoleInput(_hIn, buf, 32, &read);

        for(DWORD i = 0; i < read; ++i)
            switch(buf[i].EventType) {
                case KEY_EVENT:
                    if(buf[i].Event.KeyEvent.bKeyDown)
                        _current_key = buf[i].Event.KeyEvent.uChar.AsciiChar;
                break;
                case MOUSE_EVENT:
                    _mouseX = buf[i].Event.MouseEvent.dwMousePosition.X;
                    _mouseY = buf[i].Event.MouseEvent.dwMousePosition.Y;
                break;
                case WINDOW_BUFFER_SIZE_EVENT:
                    _consoleX = buf[i].Event.WindowBufferSizeEvent.dwSize.X;
                    _consoleY = buf[i].Event.WindowBufferSizeEvent.dwSize.Y - 1;
                break;
            }

        // technically 1000hz rate?
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void console::Draw() {
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = t1;
    std::chrono::duration<float> fps;
    int counter = 0;
    std::string title;

    while(true) {
        if(_failed_exit.load())
            return;

        t1 = std::chrono::high_resolution_clock::now();
        fps = t1 - t2;
        counter++;

        if(fps.count() >= 1.0 / 30.0) {
            title = "V - FPS " + std::to_string((1.0F / fps.count()) * static_cast<float>(counter)) +
                    " - X: " + std::to_string(_consoleX) +
                    " Y: " + std::to_string(_consoleY) +
                    + " - KEY: " + std::to_string(_current_key) +
                    " - MOUSE: X: " + std::to_string(_mouseX) +
                    + " Y: " + std::to_string(_mouseY);

            counter = 0;
            t2 = t1;
        }

        std::cout << CSI "2;1f" << _buffer;
        title.resize(_consoleX, ' ');
        std::cout << CSI "1;1f" CSI "30;47m" << title;
    }
}

std::function<bool(std::vector<console::Pixel> &, std::size_t, std::size_t, double, console::Event)>
console::_update = [](std::vector<console::Pixel> &, std::size_t, std::size_t, double, console::Event) -> bool {
    return true;
};

std::function<bool(std::vector<console::Pixel>&,std::size_t,std::size_t)>
console::_init = [](std::vector<console::Pixel> &, std::size_t, std::size_t) -> bool {
    return true;
};
void console::Run() {
    if(!SetConsoleCtrlHandler(console::_ctrlhandler, TRUE))
        return;

    _hIn = GetStdHandle(STD_INPUT_HANDLE);

    if(_hIn == INVALID_HANDLE_VALUE)
        _failed_exit = true;

    std::vector<Pixel> pixels = {};

    std::this_thread::sleep_for(std::chrono::milliseconds(384));

    std::thread input_controller(UpdateInputs);
    input_controller.detach();

    std::cout << "Loading...";
    std::this_thread::sleep_for(std::chrono::milliseconds(512));
    pixels.resize(_consoleX * _consoleY);
    if(!_init(pixels, _consoleX, _consoleY))
        return;

    std::thread renderer(Draw);
    renderer.detach();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time;
    double dTime;

    while(true) {
        if(_failed_exit.load())
            break;

        t1 = std::chrono::high_resolution_clock::now();
        time = t1 - t2;

        t2 = t1;
        dTime = time.count();

        if(_consoleX * _consoleY != pixels.size())
            pixels.resize(_consoleX * _consoleY);

        if(!_update(pixels, _consoleX.load(), _consoleY.load(), dTime, { _mouseX.load(), _mouseY.load(), _current_key.load() }))
            break;

        std::lock_guard<std::mutex> lck(_mut);
        _buffer.clear();
        for(auto & p : pixels)
            _buffer += _COLORS[p.color];
    }
}

int console::Exit() {
    // Soft Reset the Console
    std::cout << CSI "!p";

    // Show the cursor
    std::cout << CSI "?25h";

    std::cout << MAIN_BUFFER;

    if(!SetConsoleMode(_hOut, _oldhOut))
        return 1;

    if(!SetConsoleMode(_hIn, _oldhIn))
        return 1;

    return 0;
}
