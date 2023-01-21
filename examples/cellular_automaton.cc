#include <console.hh>

void celullar_automata(std::vector<console::Pixel> & pixels, std::size_t & X, std::size_t & Y) {
    static std::vector<std::pair<std::size_t, std::size_t>> to_die;
    static std::vector<std::pair<std::size_t, std::size_t>> to_alive;

    static const auto neighbours = [&](std::size_t x, std::size_t y) {
        std::size_t i = 0;
        try { if(console::grid::at_2D(pixels, x - 1, y - 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x + 1, y + 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x - 1, y + 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x + 1, y - 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}

        try { if(console::grid::at_2D(pixels, x - 1, y, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x + 1, y, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x, y - 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}
        try { if(console::grid::at_2D(pixels, x, y + 1, X).bg() == console::col::BG::WHITE) ++i; } catch(...) {}

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
        pixels[alive.second * X + alive.first] = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    }

    for(auto & dead : to_die) {
        pixels[dead.second * X + dead.first] = console::Pixel(console::col::FG::BLACK, console::col::BG::BLACK);
    }

    to_alive.clear();
    to_die.clear();
}

constexpr double tickrate = 1.0 / 5.0;
double accumulator = 0.0;

bool Init(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y) {
    console::set_title_options(console::col::FG::CYAN);

    console::grid::for_each_0(pixels, X, Y, [](console::Pixel & p) {
        p = console::Pixel(console::col::FG::BLACK, console::col::BG::BLACK);
    });

    // Gosper's gliding gun
    console::grid::at_2D(pixels, 1, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 2, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 1, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 2, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 11, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 11, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 11, 7, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 12, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 12, 8, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 13, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 13, 9, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 14, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 14, 9, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 15, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 16, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 16, 8, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 17, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 17, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 17, 7, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 18, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 21, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 21, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 21, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 22, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 22, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 22, 5, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 23, 2, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 23, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 25, 1, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 25, 2, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 25, 6, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 25, 7, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 35, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 35, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

    console::grid::at_2D(pixels, 36, 3, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);
    console::grid::at_2D(pixels, 36, 4, X) = console::Pixel(console::col::FG::WHITE, console::col::BG::WHITE);

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
    if(!console::init())
        return 0;

    console::set_update_callback(Update);
    console::set_init_callback(Init);

    console::run();

    return console::exit();
}
