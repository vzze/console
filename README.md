### Cross-platform wrapper for the terminal API

```cpp
#include <console.hh>

console instance{};

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
```

### Dependencies
* A C++ compiler with `c++20` support (`gcc 13.1.1`, `clang 16.0.6`, `msvc 19.36.32535` or later versions)
* `cmake` 3.9 or later

### Supported platforms
* `win32`
* `unix`

### Building
For each supported platform there exists a `configure` and `build` script, run them in this order.
