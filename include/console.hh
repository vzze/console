#ifndef CONSOLE_HH
#define CONSOLE_HH

#include <string_view>
#include <functional>
#include <iostream>
#include <utility>
#include <cstdint>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <format>
#include <array>

#ifdef _WIN32
#include <windows.h>
#elif defined(__unix__)
#include <sys/ioctl.h>
#include <termios.h>
#endif

struct console {
    public:
        struct coord {
            std::uint32_t column, row;

            friend auto operator <=> (const coord &, const coord &) = default;
        };
    private:
        template<typename Key, typename Value, std::size_t Size>
        struct map {
            std::array<std::pair<Key, Value>, Size> data;

            consteval Value operator [] (const Key & key) const {
                const auto itr = std::ranges::find_if(data, [&](const auto & value) {
                    return value.first == key;
                });

                if(itr == data.end()) {
                    throw std::logic_error{"Key does not exist."};
                }

                return itr->second;
            }
        };

        bool should_exit;

        std::string buffer;

        void process_events() noexcept;

        void init_alternate_buffer() noexcept;
        void main_buffer() noexcept;

        std::vector<std::function<bool(const char)>> key_callbacks;
        std::vector<std::function<bool(const coord)>> resize_callbacks;
        std::vector<std::function<bool()>> notify_alive_callbacks;
#ifdef _WIN32
        HANDLE in_handle;
        HANDLE out_handle;

        DWORD old_in_mode;
        DWORD old_out_mode;
#elif defined(__unix__)
        struct termios old_sets;
#endif
    public:
        static constexpr map<std::string_view, std::string_view, 17> FG{{{ // NOLINT(readability-identifier-length)
            { "BLACK"  , "\x1b[30m" },
            { "RED"    , "\x1b[31m" },
            { "GREEN"  , "\x1b[32m" },
            { "YELLOW" , "\x1b[33m" },
            { "BLUE"   , "\x1b[34m" },
            { "MAGENTA", "\x1b[35m" },
            { "CYAN"   , "\x1b[36m" },
            { "WHITE"  , "\x1b[37m" },

            { "BRIGHT_BLACK"  , "\x1b[90m" },
            { "BRIGHT_RED"    , "\x1b[91m" },
            { "BRIGHT_GREEN"  , "\x1b[92m" },
            { "BRIGHT_YELLOW" , "\x1b[93m" },
            { "BRIGHT_BLUE"   , "\x1b[94m" },
            { "BRIGHT_MAGENTA", "\x1b[95m" },
            { "BRIGHT_CYAN"   , "\x1b[96m" },
            { "BRIGHT_WHITE"  , "\x1b[97m" },

            { "DEFAULT", "\x1b[39m" }
        }}};

        static constexpr map<std::string_view, std::string_view, 17> BG{{{ // NOLINT(readability-identifier-length)
            { "BLACK"  , "\x1b[40m" },
            { "RED"    , "\x1b[41m" },
            { "GREEN"  , "\x1b[42m" },
            { "YELLOW" , "\x1b[43m" },
            { "BLUE"   , "\x1b[44m" },
            { "MAGENTA", "\x1b[45m" },
            { "CYAN"   , "\x1b[46m" },
            { "WHITE"  , "\x1b[47m" },

            { "BRIGHT_BLACK"  , "\x1b[100m" },
            { "BRIGHT_RED"    , "\x1b[101m" },
            { "BRIGHT_GREEN"  , "\x1b[102m" },
            { "BRIGHT_YELLOW" , "\x1b[103m" },
            { "BRIGHT_BLUE"   , "\x1b[104m" },
            { "BRIGHT_MAGENTA", "\x1b[105m" },
            { "BRIGHT_CYAN"   , "\x1b[106m" },
            { "BRIGHT_WHITE"  , "\x1b[107m" },

            { "DEFAULT", "\x1b[49m" }
        }}};

        static constexpr map<std::string_view, char, 11> DEC{{{
            /* ┘ */ { "br", 'j' },
            /* ┐ */ { "tr", 'k' },
            /* ┌ */ { "tl", 'l' },
            /* └ */ { "bl", 'm' },
            /* ┼ */ { "cr", 'n' },
            /* ─ */ { "ho", 'q' },
            /* │ */ { "ve", 'x' },
            /* ├ */ { "t1", 't' },
            /* ┤ */ { "t2", 'u' },
            /* ┴ */ { "t3", 'v' },
            /* ┬ */ { "t4", 'w' },
        }}};

        console() noexcept;

        console(const console &) = default;
        console(console &&) = default;

        console & operator = (const console &) = default;
        console & operator = (console &&) = default;

        template<typename Writable>
        void write(Writable && writable) {
            try {
                if constexpr(requires{ buffer + writable; })
                    buffer += writable;
                else
                    buffer += std::format("{}", std::forward<Writable>(writable));
            } catch(...) { should_exit = true; }
        }

        template<typename Callable>
        void add_key_callback(Callable && callable) noexcept {
            try {
                key_callbacks.emplace_back(std::forward<Callable>(callable));
            } catch(...) { should_exit = true; }
        }

        template<typename Callable>
        void add_resize_callback(Callable && callable) noexcept {
            try {
                resize_callbacks.emplace_back(std::forward<Callable>(callable));
            } catch(...) { should_exit = true; }
        }

        template<typename Callable>
        void add_notify_alive_callback(Callable && callable) noexcept {
            try {
                notify_alive_callbacks.emplace_back(std::forward<Callable>(callable));
            } catch(...) { should_exit = true; }
        }

        bool refresh() noexcept;

        void main_loop(const std::chrono::milliseconds) noexcept;
        void main_loop() noexcept;

        void set_cursor_pos(const coord);

        template<typename Writable>
        void print_at_pos(const coord coords, Writable && writable) {
            set_cursor_pos(coords);
            write(std::forward<Writable>(writable));
        }

        void insert_char(const std::uint32_t);
        void delete_char(const std::uint32_t);
        void erase_char(const std::uint32_t);
        void insert_line(const std::uint32_t);
        void delete_line(const std::uint32_t);

        void dec_mode();
        void ascii_mode();

        enum class DISPLAY : std::uint32_t {
            CURSOR_TO_EOD  = 0,
            BEG_TO_CURSOR  = 1,
            ENTIRE_DISPLAY = 2,
        };

        void erase_in_display(const DISPLAY);

        enum class LINE : std::uint32_t {
            CURSOR_TO_EOL = 0,
            BEG_TO_CURSOR = 1,
            ENTIRE_LINE   = 2
        };

        void erase_in_line(const LINE);

        ~console() noexcept;
};

#endif
