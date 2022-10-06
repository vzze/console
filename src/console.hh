#pragma once

#include <atomic>
#include <vector>
#include <string>
#include <mutex>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <sys/ioctl.h>
#include <termios.h>
#endif

#define ESC "\x1b"
#define CSI ESC "["

#define FG_BLACK           CSI "30m"
#define FG_RED             CSI "31m"
#define FG_GREEN           CSI "32m"
#define FG_YELLOW          CSI "33m"
#define FG_BLUE            CSI "34m"
#define FG_MAGENTA         CSI "35m"
#define FG_CYAN            CSI "36m"
#define FG_WHITE           CSI "37m"

#define FG_BRIGHT_BLACK    CSI "90m"
#define FG_BRIGHT_RED      CSI "91m"
#define FG_BRIGHT_GREEN    CSI "92m"
#define FG_BRIGHT_YELLOW   CSI "93m"
#define FG_BRIGHT_BLUE     CSI "94m"
#define FG_BRIGHT_MAGENTA  CSI "95m"
#define FG_BRIGHT_CYAN     CSI "96m"
#define FG_BRIGHT_WHITE    CSI "97m"

#define FG_DEFAULT         CSI "39m"

#define BG_BLACK           CSI "40m"
#define BG_RED             CSI "41m"
#define BG_GREEN           CSI "42m"
#define BG_YELLOW          CSI "43m"
#define BG_BLUE            CSI "44m"
#define BG_MAGENTA         CSI "45m"
#define BG_CYAN            CSI "46m"
#define BG_WHITE           CSI "47m"

#define BG_BRIGHT_BLACK    CSI "100m"
#define BG_BRIGHT_RED      CSI "101m"
#define BG_BRIGHT_GREEN    CSI "102m"
#define BG_BRIGHT_YELLOW   CSI "103m"
#define BG_BRIGHT_BLUE     CSI "104m"
#define BG_BRIGHT_MAGENTA  CSI "105m"
#define BG_BRIGHT_CYAN     CSI "106m"
#define BG_BRIGHT_WHITE    CSI "107m"

#define BG_DEFAULT         CSI "49m"

namespace console {
    namespace col {
        enum class FG : std::uint8_t {
            BLACK          = 0,
            RED            = 1,
            GREEN          = 2,
            YELLOW         = 3,
            BLUE           = 4,
            MAGENTA        = 5,
            CYAN           = 6,
            WHITE          = 7,

            BRIGHT_BLACK   = 8,
            BRIGHT_RED     = 9,
            BRIGHT_GREEN   = 10,
            BRIGHT_YELLOW  = 11,
            BRIGHT_BLUE    = 12,
            BRIGHT_MAGENTA = 13,
            BRIGHT_CYAN    = 14,
            BRIGHT_WHITE   = 15,

            DEFAULT        = 16
        };

        enum class BG : std::uint8_t {
            BLACK          = 0,
            RED            = 1,
            GREEN          = 2,
            YELLOW         = 3,
            BLUE           = 4,
            MAGENTA        = 5,
            CYAN           = 6,
            WHITE          = 7,


            BRIGHT_BLACK   = 8,
            BRIGHT_RED     = 9,
            BRIGHT_GREEN   = 10,
            BRIGHT_YELLOW  = 11,
            BRIGHT_BLUE    = 12,
            BRIGHT_MAGENTA = 13,
            BRIGHT_CYAN    = 14,
            BRIGHT_WHITE   = 15,

            DEFAULT        = 16
        };
    }

    struct Pixel {
        col::FG fg;
        col::BG bg;
        char display;
        Pixel(col::FG _fg = col::FG::BLACK, col::BG _bg = col::BG::BLACK, char _display = ' ');
    };

    namespace _impl {
        inline constexpr char const * const _fg_colors[] = {
            FG_BLACK,
            FG_RED,
            FG_GREEN,
            FG_YELLOW,
            FG_BLUE,
            FG_MAGENTA,
            FG_CYAN,
            FG_WHITE,

            FG_BRIGHT_BLACK,
            FG_BRIGHT_RED,
            FG_BRIGHT_GREEN,
            FG_BRIGHT_YELLOW,
            FG_BRIGHT_BLUE,
            FG_BRIGHT_MAGENTA,
            FG_BRIGHT_CYAN,
            FG_BRIGHT_WHITE,

            FG_DEFAULT
        };

        inline constexpr char const * const _bg_colors[] = {
            BG_BLACK,
            BG_RED,
            BG_GREEN,
            BG_YELLOW,
            BG_BLUE,
            BG_MAGENTA,
            BG_CYAN,
            BG_WHITE,

            BG_BRIGHT_BLACK,
            BG_BRIGHT_RED,
            BG_BRIGHT_GREEN,
            BG_BRIGHT_YELLOW,
            BG_BRIGHT_BLUE,
            BG_BRIGHT_MAGENTA,
            BG_BRIGHT_CYAN,
            BG_BRIGHT_WHITE,

            BG_DEFAULT
        };
#ifdef _WIN32
        extern HANDLE _hOut;
        extern HANDLE _hIn;

        extern DWORD _oldhOut;
        extern DWORD _oldhIn;
#endif
        extern std::atomic_bool _failed_exit;

        extern std::atomic_size_t _consoleX;
        extern std::atomic_size_t _consoleY;
#ifdef _WIN32
        extern std::atomic_size_t _mouseX;
        extern std::atomic_size_t _mouseY;
#endif
        extern std::atomic_char _current_key;

        struct _buffer {
            std::vector<Pixel> _next, _current;
            std::mutex _mut_read;
            std::mutex _mut_write;
        };

        extern _buffer _pbuf;

        extern std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t, float)> _update_callback;
        extern std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t)> _init_callback;
        extern std::function<void(char)> _key_callback;
#ifdef _WIN32
        extern bool _mouse_pressed_buttons[5];
        extern std::function<void(const bool *, std::size_t, std::size_t)> _mouse_callback;

        BOOL _ctrlhandler(DWORD ctrltype);
#endif
        void _updateinputs();

        void _draw();
    }

    /* returns nonzero value if successful; */
    int init();
#ifdef _WIN32
    void set_mouse_callback(std::function<void(const bool *, std::size_t, std::size_t)>);
#endif
    void set_key_callback(std::function<void(char)>);

    void set_update_callback(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t, double)>);

    void set_init_callback(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t)>);

    void run();

    /* returns zero if successful; */
    int exit();

    // collection of utility functions to interact with the pixel buffer
    namespace grid {
        // parameters: pixel buffer, x coord, y coord, number of columns (X);
        // throws if out of bounds;
        // returns Pixel reference;
        Pixel & at_2D(std::vector<Pixel> &, std::size_t, std::size_t, std::size_t);

        // parameters: x coord, y coord, number of columns (X);
        // converts to coords to vector position: pixels[at_2D(12, 12, X)];
        std::size_t at_2D(std::size_t, std::size_t, std::size_t) noexcept;

        // parameters: pixel buffer, string, x coord, y coord, fg,
        // bg, number of columns (X);
        // throws if out of bounds;
        // if string is larger than remaining space in vector its cut;
        // using escape sequences like \n or \t WILL break the pixel buffer;
        void set_string(std::vector<Pixel> &, std::string_view, col::FG fg, col::BG bg, std::size_t, std::size_t, std::size_t);

        // parameters: pixel buffer, string, fg,
        // bg, vector position;
        // throws if out of bounds;
        // if string is larger than remaining space in vector its cut;
        // using escape sequences like \n or \t WILL break the pixel buffer;
        void set_string(std::vector<Pixel> &, std::string_view, col::FG fg, col::BG bg, std::size_t);

        // parameters: pixel buffer, number of columns (X), number of lines (Y), function with side effect;
        // 0 degree for_each applied on a pixel buffer;
        void for_each_0(std::vector<Pixel> &, std::size_t, std::size_t, std::function<void(Pixel &)>);

        // parameters: pixel buffer, number of columns (X), number of lines (Y), function with side effect;
        // 90 degree for_each applied on a pixel buffer;
        void for_each_90(std::vector<Pixel> &, std::size_t, std::size_t, std::function<void(Pixel &)>);

        // parameters: pixel buffer, number of columns (X), number of lines (Y), function with side effect;
        // 180 degree for_each applied on a pixel buffer;
        void for_each_180(std::vector<Pixel> &, std::size_t, std::size_t, std::function<void(Pixel &)>);

        // parameters: pixel buffer, number of columns (X), number of lines (Y), function with side effect;
        // 270 degree for_each applied on a pixel buffer;
        void for_each_270(std::vector<Pixel> &, std::size_t, std::size_t, std::function<void(Pixel &)>);
    }
}

#undef ESC
#undef CSI
#undef OSC
#undef ST

#undef FG_BLACK
#undef FG_RED
#undef FG_GREEN
#undef FG_YELLOW
#undef FG_BLUE
#undef FG_MAGENTA
#undef FG_CYAN
#undef FG_WHITE

#undef FG_BRIGHT_BLACK
#undef FG_BRIGHT_RED
#undef FG_BRIGHT_GREEN
#undef FG_BRIGHT_YELLOW
#undef FG_BRIGHT_BLUE
#undef FG_BRIGHT_MAGENTA
#undef FG_BRIGHT_CYAN
#undef FG_BRIGHT_WHITE

#undef FG_DEFAULT

#undef BG_BLACK
#undef BG_RED
#undef BG_GREEN
#undef BG_YELLOW
#undef BG_BLUE
#undef BG_MAGENTA
#undef BG_CYAN
#undef BG_WHITE

#undef BG_BRIGHT_BLACK
#undef BG_BRIGHT_RED
#undef BG_BRIGHT_GREEN
#undef BG_BRIGHT_YELLOW
#undef BG_BRIGHT_BLUE
#undef BG_BRIGHT_MAGENTA
#undef BG_BRIGHT_CYAN
#undef BG_BRIGHT_WHITE

#undef BG_DEFAULT
