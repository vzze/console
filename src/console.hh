#pragma once

#include <atomic>
#include <vector>
#include <string>
#include <mutex>
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <sys/ioctl.h>
#include <termios.h>
#endif

namespace console {
    namespace seq {
        inline constexpr char const * const ESC               = "\x1b";
        inline constexpr char const * const CSI               = "\x1b[";

        inline constexpr char const * const FG_BLACK          = "\x1b[30m";
        inline constexpr char const * const FG_RED            = "\x1b[31m";
        inline constexpr char const * const FG_GREEN          = "\x1b[32m";
        inline constexpr char const * const FG_YELLOW         = "\x1b[33m";
        inline constexpr char const * const FG_BLUE           = "\x1b[34m";
        inline constexpr char const * const FG_MAGENTA        = "\x1b[35m";
        inline constexpr char const * const FG_CYAN           = "\x1b[36m";
        inline constexpr char const * const FG_WHITE          = "\x1b[37m";

        inline constexpr char const * const FG_BRIGHT_BLACK   = "\x1b[90m";
        inline constexpr char const * const FG_BRIGHT_RED     = "\x1b[91m";
        inline constexpr char const * const FG_BRIGHT_GREEN   = "\x1b[92m";
        inline constexpr char const * const FG_BRIGHT_YELLOW  = "\x1b[93m";
        inline constexpr char const * const FG_BRIGHT_BLUE    = "\x1b[94m";
        inline constexpr char const * const FG_BRIGHT_MAGENTA = "\x1b[95m";
        inline constexpr char const * const FG_BRIGHT_CYAN    = "\x1b[96m";
        inline constexpr char const * const FG_BRIGHT_WHITE   = "\x1b[97m";

        inline constexpr char const * const FG_DEFAULT        = "\x1b[39m";

        inline constexpr char const * const BG_BLACK          = "\x1b[40m";
        inline constexpr char const * const BG_RED            = "\x1b[41m";
        inline constexpr char const * const BG_GREEN          = "\x1b[42m";
        inline constexpr char const * const BG_YELLOW         = "\x1b[43m";
        inline constexpr char const * const BG_BLUE           = "\x1b[44m";
        inline constexpr char const * const BG_MAGENTA        = "\x1b[45m";
        inline constexpr char const * const BG_CYAN           = "\x1b[46m";
        inline constexpr char const * const BG_WHITE          = "\x1b[47m";

        inline constexpr char const * const BG_BRIGHT_BLACK   = "\x1b[100m";
        inline constexpr char const * const BG_BRIGHT_RED     = "\x1b[101m";
        inline constexpr char const * const BG_BRIGHT_GREEN   = "\x1b[102m";
        inline constexpr char const * const BG_BRIGHT_YELLOW  = "\x1b[103m";
        inline constexpr char const * const BG_BRIGHT_BLUE    = "\x1b[104m";
        inline constexpr char const * const BG_BRIGHT_MAGENTA = "\x1b[105m";
        inline constexpr char const * const BG_BRIGHT_CYAN    = "\x1b[106m";
        inline constexpr char const * const BG_BRIGHT_WHITE   = "\x1b[107m";

        inline constexpr char const * const BG_DEFAULT        = "\x1b[49m";


        inline constexpr char const * const ALTERNATE_BUFFER  = "\x1b[?1049h";
        inline constexpr char const * const MAIN_BUFFER       = "\x1b[?1049l";

        inline constexpr char const * const HIDE_CURSOR       = "\x1b[?25l";
        inline constexpr char const * const BUFFER_POSITION   = "\x1b[1;1f";
        inline constexpr char const * const SHOW_CURSOR       = "\x1b[?25h";
        inline constexpr char const * const SOFT_RESET        = "\x1b[!p";
    }

    namespace col {
        // set of values used to create Pixels representing the 16 ansi colors
        // DONT_REPLACE should only be used with set_string
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

            DEFAULT        = 16,

            DONT_REPLACE   = 17
        };

        // set of values used to create Pixels representing the 16 ansi colors
        // DONT_REPLACE should only be used with set_string;
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

            DEFAULT        = 16,

            DONT_REPLACE   = 17
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
            seq::FG_BLACK,
            seq::FG_RED,
            seq::FG_GREEN,
            seq::FG_YELLOW,
            seq::FG_BLUE,
            seq::FG_MAGENTA,
            seq::FG_CYAN,
            seq::FG_WHITE,

            seq::FG_BRIGHT_BLACK,
            seq::FG_BRIGHT_RED,
            seq::FG_BRIGHT_GREEN,
            seq::FG_BRIGHT_YELLOW,
            seq::FG_BRIGHT_BLUE,
            seq::FG_BRIGHT_MAGENTA,
            seq::FG_BRIGHT_CYAN,
            seq::FG_BRIGHT_WHITE,

            seq::FG_DEFAULT
        };

        inline constexpr char const * const _bg_colors[] = {
            seq::BG_BLACK,
            seq::BG_RED,
            seq::BG_GREEN,
            seq::BG_YELLOW,
            seq::BG_BLUE,
            seq::BG_MAGENTA,
            seq::BG_CYAN,
            seq::BG_WHITE,

            seq::BG_BRIGHT_BLACK,
            seq::BG_BRIGHT_RED,
            seq::BG_BRIGHT_GREEN,
            seq::BG_BRIGHT_YELLOW,
            seq::BG_BRIGHT_BLUE,
            seq::BG_BRIGHT_MAGENTA,
            seq::BG_BRIGHT_CYAN,
            seq::BG_BRIGHT_WHITE,

            seq::BG_DEFAULT
        };
#ifdef _WIN32
        extern HANDLE _hOut;
        extern HANDLE _hIn;

        extern DWORD _oldhOut;
        extern DWORD _oldhIn;
#endif
        extern std::atomic_bool _failed_exit;
        extern std::atomic_bool _draw_title;

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

    /* toggles inbetween drawing or not drawing the title; */
    void toggle_title();

    /* false if not being drawn true otherwise; */
    bool title_state();

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
