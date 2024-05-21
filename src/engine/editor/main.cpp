#include "window.h"

std::vector<std::string> parseCommandLineArgs(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return args;
}

int main(int argc, char* argv[])
{
    auto args = parseCommandLineArgs(argc, argv);
    std::string filename = "";
    if (args.size() > 1) {
        filename = args[1];
    }
    Window* renderView = new Window(1280, 720, false, filename);
    
    renderView->run();
    delete renderView;

	return 0;
}
