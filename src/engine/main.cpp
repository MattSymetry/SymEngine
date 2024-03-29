#include "window.h"

int main(int argc, char* argv[])
{
    Window* renderView = new Window(1280, 720, false);
    
    renderView->run();
    delete renderView;

	return 0;
}
