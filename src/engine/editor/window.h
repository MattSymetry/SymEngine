#pragma once
#include "../common/config.h"
#include "../engine/engine.h"
#include "../engine/scene.h"
#include "editor.h"

class Window {
private:
    Engine* _engine;
    struct SDL_Window* _window{ nullptr };
    Editor* _editor;
    SDL_Event e;
    Scene* _scene;
    char _windowTitle[100];
    bool _isInitialized = false;
    std::atomic<bool> render = false;
    
    int _lastFrameTime, _currentFrameTime, _lastFPSupdate;
    int _numFrames;
    float _frameDelay;
    int _frameCap = 120;
    int _deltaTime;
    float _framerate;
    
    int _width, _height, _tmpMousePosX, _tmpMousePosY;
    
    void setupSDLWindow(int width, int height);
    void setupTimer();
    void calcFramerate();
    void frameCap();
    void renderLoop();
    
public:
    Window(int widht, int height, bool debug);
    ~Window();
    void run();
};
