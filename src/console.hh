#pragma once

#include <atomic>
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>

#include <windows.h>

namespace console {
    enum class COLORS {
        BLACK   = 0,
        RED     = 1,
        GREEN   = 2,
        YELLOW  = 3,
        BLUE    = 4,
        MAGENTA = 5,
        CYAN    = 6,
        WHITE   = 7,

        BRIGHT_BLACK   = 8,
        BRIGHT_RED     = 9,
        BRIGHT_GREEN   = 10,
        BRIGHT_YELLOW  = 11,
        BRIGHT_BLUE    = 12,
        BRIGHT_MAGENTA = 13,
        BRIGHT_CYAN    = 14,
        BRIGHT_WHITE   = 15,
    };

    struct Pixel {
        int color;
        Pixel(COLORS _col = COLORS::BLACK);
    };

    extern const char * _colors[];

    extern HANDLE _hOut;
    extern HANDLE _hIn;

    extern DWORD _oldhOut;
    extern DWORD _oldhIn;

    extern std::atomic_bool _failed_exit;

    extern std::atomic_size_t _consoleX;
    extern std::atomic_size_t _consoleY;

    extern std::atomic_size_t _mouseX;
    extern std::atomic_size_t _mouseY;

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
    extern bool _mpressedbuttons[5];
    extern std::function<void(const bool *, std::size_t, std::size_t)> _mousebuttons;

    int Init();

    BOOL _ctrlhandler(DWORD ctrltype);

    void _updateinputs();

    void _draw();

    void SetMouseCallbackFunc(std::function<void(const bool *, std::size_t, std::size_t)> f);

    void SetKeyCallbackFunc(std::function<void(char)> f);

    void SetUpdateFunc(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t, double)> f);

    void SetInitFunc(std::function<bool(std::vector<Pixel> &, std::size_t, std::size_t)> f);

    void Run();

    int Exit();
}
