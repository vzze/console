#pragma once

#include <atomic>
#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <chrono>

#include <windows.h>

namespace console {
    extern const char * _COLORS[];

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

    struct Event {
        std::size_t mouseX = 0;
        std::size_t mouseY = 0;
        char lastKey = 0;
    };

    int Init();

    extern HANDLE _hOut;
    extern HANDLE _hIn;
    extern DWORD _oldhOut;
    extern DWORD _oldhIn;

    extern std::atomic_bool _failed_exit;

    extern std::atomic_size_t _consoleX;
    extern std::atomic_size_t _consoleY;
    extern std::atomic_char _current_key;

    extern std::mutex _mut;
    extern std::string _buffer;

    BOOL _ctrlhandler(DWORD ctrltype);

    extern std::atomic_size_t _mouseX;
    extern std::atomic_size_t _mouseY;

    void UpdateInputs();

    void Draw();

    extern std::function<bool(std::vector<Pixel>&,std::size_t,std::size_t, double, Event)> _update;
    extern std::function<bool(std::vector<Pixel>&,std::size_t,std::size_t)> _init;

    template<typename Func>
    void SetUpdateFunc(Func f) { _update = f; }
    template<typename Func>
    void SetInitFunc(Func f) { _init = f; }

    void Run();

    int Exit();
}
