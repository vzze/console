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

namespace console {
    enum class COLORS : int {
        FG_BLACK          = 0,
        FG_RED            = 1,
        FG_GREEN          = 2,
        FG_YELLOW         = 3,
        FG_BLUE           = 4,
        FG_MAGENTA        = 5,
        FG_CYAN           = 6,
        FG_WHITE          = 7,

        BG_BLACK          = 8,
        BG_RED            = 9,
        BG_GREEN          = 10,
        BG_YELLOW         = 11,
        BG_BLUE           = 12,
        BG_MAGENTA        = 13,
        BG_CYAN           = 14,
        BG_WHITE          = 15,

        FG_BRIGHT_BLACK   = 16,
        FG_BRIGHT_RED     = 17,
        FG_BRIGHT_GREEN   = 18,
        FG_BRIGHT_YELLOW  = 19,
        FG_BRIGHT_BLUE    = 20,
        FG_BRIGHT_MAGENTA = 21,
        FG_BRIGHT_CYAN    = 22,
        FG_BRIGHT_WHITE   = 23,

        BG_BRIGHT_BLACK   = 24,
        BG_BRIGHT_RED     = 25,
        BG_BRIGHT_GREEN   = 26,
        BG_BRIGHT_YELLOW  = 27,
        BG_BRIGHT_BLUE    = 28,
        BG_BRIGHT_MAGENTA = 29,
        BG_BRIGHT_CYAN    = 30,
        BG_BRIGHT_WHITE   = 31,
    };

    struct Pixel {
        COLORS fg, bg;
        char display;
        Pixel(COLORS _fg = COLORS::FG_BLACK, COLORS _bg = COLORS::BG_BLACK, char _display = ' ');
    };

    namespace _impl {
        extern const char * _colors[];
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
            std::vector<Pixel> next, current;
            mutable std::mutex mut_read;
            std::mutex mut_write;
        };

        extern _buffer _pbuf;

        extern std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t, float)> _update;
        extern std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t)> _init;
        extern std::function<void(char)> _keycallback;
#ifdef _WIN32
        extern bool _mpressedbuttons[5];
        extern std::function<void(const bool *, std::size_t, std::size_t)> _mousebuttons;

        BOOL _ctrlhandler(DWORD ctrltype);
#endif
        void _updateinputs();

        void _draw();
    }

    int init();
#ifdef _WIN32
    void set_mouse_callback(std::function<void(const bool *, std::size_t, std::size_t)>);
#endif
    void set_key_callback(std::function<void(char)>);

    void set_update_callback(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t, double)>);

    void set_init_callback(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t)>);

    void run();

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

        // parameters: pixel buffer, string, x coord, y coord, number of columns (X);
        // throws if out of bounds;
        void set_string(std::vector<Pixel> &, std::string_view, COLORS fg, COLORS bg, std::size_t, std::size_t, std::size_t);

        // parameters: pixel buffer, string, vector position;
        // throws if out of bounds;
        void set_string(std::vector<Pixel> &, std::string_view, COLORS fg, COLORS bg, std::size_t);

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
