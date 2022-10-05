#include "../src/console.hh"

#include <vector>

template<typename T>
inline T & at2D(std::vector<T> & container, std::size_t x, std::size_t y, std::size_t X) {
    return container.at(y * X + x);
}

void celullar_automata(std::vector<console::Pixel> & pixels, std::size_t & X, std::size_t & Y) {
    static std::vector<std::pair<std::size_t, std::size_t>> to_die;
    static std::vector<std::pair<std::size_t, std::size_t>> to_alive;

    static const auto neighbours = [&](std::size_t x, std::size_t y) {
        std::size_t i = 0;
        try { if(at2D(pixels, x - 1, y - 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x + 1, y + 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x - 1, y + 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x + 1, y - 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}

        try { if(at2D(pixels, x - 1, y, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x + 1, y, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x, y - 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}
        try { if(at2D(pixels, x, y + 1, X).bg == console::COLORS::BG_WHITE) ++i; } catch(...) {}

        return i;
    };

    for(std::size_t y = 0; y < Y; ++y)
        for(std::size_t x = 0; x < X; ++x) {
            auto value = neighbours(x, y);

            if(value == 3) {
                to_alive.push_back({x, y});
            }

            if(value != 2 && value != 3) {
                to_die.push_back({x, y});
            }
        }

    for(auto & alive : to_alive) {
        pixels[alive.second * X + alive.first] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    }

    for(auto & dead : to_die) {
        pixels[dead.second * X + dead.first] = console::Pixel(console::COLORS::FG_BLACK, console::COLORS::BG_BLACK);
    }

    to_alive.clear();
    to_die.clear();
}

constexpr double tickrate = 1.0 / 5.0;
double accumulator = 0.0;

bool Init(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y) {
    for(std::size_t y = 0; y < Y; ++y)
        for(std::size_t x = 0; x < X; ++x)
            pixels[y * X + x] = console::Pixel(console::COLORS::FG_BLACK, console::COLORS::BG_BLACK);

    // R-pentomino
    pixels[X * Y / 2 + 30 - X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 31 - X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 + 29] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 30] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 + 30 + X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    // Gosper's gliding gun
    at2D(pixels, 1, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 2, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 1, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 2, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 11, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 11, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 11, 7, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 12, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 12, 8, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 13, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 13, 9, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 14, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 14, 9, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 15, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 16, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 16, 8, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 17, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 17, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 17, 7, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 18, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 21, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 21, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 21, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 22, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 22, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 22, 5, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 23, 2, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 23, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 25, 1, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 25, 2, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 25, 6, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 25, 7, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 35, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 35, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    at2D(pixels, 36, 3, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    at2D(pixels, 36, 4, X) = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    // Toad
    pixels[X * Y / 2 - 20] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 21] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 22] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 - 20 - X + 1] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 21 - X + 1] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 22 - X + 1] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    // Penta-decathlon
    pixels[X * Y / 2 - X * 4] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1 - X * 4] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 - X * 4] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 - 1 - X * 3] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 - X * 3] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 - X * 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1 - X * 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 - X * 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 - X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1 - X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 - X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 + X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1 + X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 + X] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 - 1 + X * 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 + X * 2] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    pixels[X * Y / 2 + X * 3] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 - 1 + X * 3] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);
    pixels[X * Y / 2 + 1 + X * 3] = console::Pixel(console::COLORS::FG_WHITE, console::COLORS::BG_WHITE);

    return true;
}

bool Update(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y, float deltaTime) {

    accumulator += deltaTime;

    while(accumulator >= tickrate) {
        celullar_automata(pixels, X, Y);
        accumulator -= tickrate;
    }

    return true;
}

int main(void) {
    if(console::init())
        return 0;

    console::set_update_callback(Update);
    console::set_init_callback(Init);

    console::run();

    return console::exit();
}
