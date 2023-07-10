#include <console.hh>

console instance{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

bool key(const char key) {
    if(key == 'q') return false;

    return instance.refresh();
}

bool resize([[maybe_unused]] const console::coord coords) {
    return instance.refresh();
}

int main() {
    instance.main_loop();
}
