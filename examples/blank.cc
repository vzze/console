#include "../src/console.hh"

void KeyCallback(char key) {
    // Do something with the pressed key
}

// only gets called when a mouse button is pressed
void MouseButtonCallback(const bool mouse_buttons[5], std::size_t mouseX, std::size_t mouseY) {
    // MouseButton1 = mouse_buttons[0]
    // ...
}

// gets called before the main thread starts
bool Init(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y) {

    return true;
}

// gets called every loop in the main thread
bool Update(std::vector<console::Pixel> & pixels, std::size_t X, std::size_t Y, float deltaTime) {

    return true;
}

int main() {
    console::Init();

    console::SetInitFunc(Init);
    console::SetUpdateFunc(Update);
    console::SetKeyCallbackFunc(KeyCallback);
    console::SetMouseCallbackFunc(MouseButtonCallback);

    console::Run();

    console::Exit();
}
