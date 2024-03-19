#include "window.h"
#include <thread>
#include <functional>

using namespace std;
Window::Window(int width, int height, bool debug)
{
    _width = width;
    _height = height;
    setupSDLWindow(width, height);
    setupTimer();
    
    _scene = new Scene();
    
    _engine = new Engine(width, height, _window, _scene);
    
    _frameDelay = 1000.0f / _frameCap;
}


void Window::setupSDLWindow(int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    _window = SDL_CreateWindow(
        "SymEngine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        window_flags
    );
    
    _isInitialized = true;
}

void Window::run() {
    bool bQuit = false;
    render = true;
    std::thread renderThread(std::bind(&Window::renderLoop, this));

    
    while (!bQuit)
    {
        // Events
       if (SDL_WaitEvent(&e)) {
            
            if (e.type == SDL_QUIT)
            {
                bQuit = true;
                render = false;
                renderThread.join();
            }
            else if (e.type == SDL_KEYDOWN)
            {
                _scene->KeyPressed(e.key.keysym.sym);
            }
            else if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    cout << "Window resized" << endl;
                    _width = e.window.data1;
                    _height = e.window.data2;
                }
            }

            SDL_PumpEvents();
            const Uint8* state = SDL_GetKeyboardState(NULL);
            _scene->KeyboardInput((Uint8*)state);

            //SDL_Delay(6);saddddc
       }
    }
}

void Window::renderLoop()
{
    while (render)
    {
		_engine->render(_scene);
        _scene->Update(_deltaTime);
        calcFramerate();
	}
}

void Window::setupTimer() {
    _lastFrameTime = SDL_GetTicks();
    _currentFrameTime = SDL_GetTicks();
    _numFrames = 0;
    _lastFPSupdate = 0;
}

void Window::calcFramerate()
{
    _currentFrameTime = SDL_GetTicks();
    _deltaTime = _currentFrameTime - _lastFrameTime;

    if (_deltaTime >= 1) {
        float framerate = 1.0f / (_deltaTime / 1000.0f);
        if (_currentFrameTime - _lastFPSupdate > 500 ) {
            snprintf(_windowTitle, sizeof(_windowTitle), "SymEngine | FPS: %.0f", framerate);
            //SDL_SetWindowTitle(_window, _windowTitle);
            _lastFPSupdate = _currentFrameTime;
        }
        _lastFrameTime = _currentFrameTime;
        _numFrames = -1;
    }

    ++_numFrames;
}

void Window::frameCap()
{
    if (_frameDelay > _deltaTime)
    {
        SDL_Delay(_frameDelay - _deltaTime);
    }
}

// Destructor
Window::~Window() {
    delete _engine;
    delete _scene;
    if (_isInitialized) {
        SDL_DestroyWindow(_window);
    }
}
