#include "window.h"
#include <thread>
#include <functional>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

using namespace std;
Window::Window(int width, int height, bool debug, std::string filename)
{
    _width = width;
    _height = height;
    setupSDLWindow(width, height);
    setupTimer();
    
    _scene = new Scene(glm::vec4(0.0f, 0.0f, width, height));
    if (!filename.empty()) {
		_scene->loadScene(filename);
	}
    
    _engine = new Engine(width, height, _window, _scene);

    _editor = new Editor();

    _tmpMousePosX = _width / 2;
    _tmpMousePosY = _height / 2;
    
    _frameDelay = 1000.0f / _frameCap;
}


void Window::setupSDLWindow(int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    _window = SDL_CreateWindow(
        "SYMYS",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        window_flags
    );

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(_window, &wmInfo)) {
        HWND hwnd = wmInfo.info.win.window;
        BOOL enabled = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled));
    }
    
    _isInitialized = true;
}

bool isCtrlPressed(SDL_Keymod mod) {
    return (mod & KMOD_CTRL) != 0;
}

bool isShiftPressed(SDL_Keymod mod) {
    return (mod & KMOD_LSHIFT) != 0;
}

void Window::run() {
    bool bQuit = false;
    render = true;
    int customWindowWidth = 300;
    _editor->setTheme();
    //std::thread renderThread(std::bind(&Window::renderLoop, this));

    
    while (!bQuit)
    {
        // Events
       glm::vec4 viewp = _editor->GetViewport();
       glm::vec4 viewportBounds = glm::vec4(viewp.x, viewp.x + viewp.z, 0.0f, viewp.w);
       int mouseX, mouseY;
       Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
       _scene->MousePos(mouseX, mouseY);
       bool mouseInViewport = (mouseX > viewportBounds.x && mouseX < viewportBounds.y && mouseY > viewportBounds.z && mouseY < viewportBounds.w);
       if (SDL_GetRelativeMouseMode()) {
           if (true) {
               bool isRightButtonDown = mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT);
               bool isWheelButtonDown = mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
               // Get 
               if (!isRightButtonDown && !isWheelButtonDown) {
                   SDL_SetRelativeMouseMode(SDL_FALSE);
                   SDL_WarpMouseInWindow(_window, _tmpMousePosX, _tmpMousePosY);
               }
               else if(isRightButtonDown) {
                   int deltaX, deltaY;
                   SDL_GetRelativeMouseState(&deltaX, &deltaY);
                   _scene->MouseInput(deltaX, deltaY);
               } 
               else {
                   int deltaX, deltaY;
				   SDL_GetRelativeMouseState(&deltaX, &deltaY);
				   _scene->WheelPressed(deltaX, deltaY);
               }
           }
       }

       if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
           _scene->endAction();
       }

       if (SDL_PollEvent(&e)) {
            
            if (e.type == SDL_QUIT)
            {
                bQuit = true;
                render = false;
               // renderThread.join();
            }
            else if (e.type == SDL_KEYDOWN && !ImGui::IsAnyItemActive())
            {
                _scene->KeyPressed(e.key.keysym.sym);
                SDL_Keymod mod = SDL_GetModState();
                if (isCtrlPressed(mod)) {
                    switch (e.key.keysym.sym) {
                    case SDLK_c:
                        _scene->CtrC();
                        break;
                    case SDLK_v:
                        _scene->CtrV();
                        break;
                    case SDLK_d:
                        _scene->CtrD();
                        break;
                    case SDLK_z:
                        _scene->CtrZ();
						break;
                    case SDLK_y:
						_scene->CtrY();
                        break;
                    case SDLK_s:
                        if (isShiftPressed(mod)) {
							_editor->saveScene(_scene, true);
						}
                        else {
                            _editor->saveScene(_scene);
						}
						break;
                    case SDLK_n:
                        _scene->newScene();
                        break;
                    case SDLK_o:
                        _editor->openScene(_scene);
						break;
                    }
                }
            }
            else if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    cout << "Window resized" << endl;
                    _width = e.window.data1;
                    _height = e.window.data2;
                    ImGuiIO& io = ImGui::GetIO();
                    io.DisplaySize = ImVec2(static_cast<float>(_width), static_cast<float>(_height));
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (mouseInViewport && (e.button.button == SDL_BUTTON_RIGHT || e.button.button == SDL_BUTTON_MIDDLE)) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    SDL_GetMouseState(&_tmpMousePosX, &_tmpMousePosY);
                    SDL_GetRelativeMouseState(nullptr, nullptr);
                }
            }
            else if (mouseInViewport && e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_RIGHT) {
					SDL_SetRelativeMouseMode(SDL_FALSE);
                    SDL_WarpMouseInWindow(_window, _tmpMousePosX, _tmpMousePosY);
				}
			}
            else if (mouseInViewport &&  e.type == SDL_MOUSEWHEEL) {
				_scene->MouseScroll(e.wheel.y);
			}

            SDL_PumpEvents();
            const Uint8* state = SDL_GetKeyboardState(NULL);
            _scene->KeyboardInput((Uint8*)state);

           ImGui_ImplSDL2_ProcessEvent(&e);
       }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        snprintf(_windowTitle, sizeof(_windowTitle), "SYMYS | FPS: %.0f", ImGui::GetIO().Framerate);
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        if (!io.WantCaptureMouse && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            _scene->ClickedInViewPort();
        }

        _editor->Docker();
        _editor->Gizmo(_scene);
        _editor->SettingsPanel(_scene);
        _editor->MenuBar(_scene);
        if (_scene->m_viewport != viewp) 
        {
			_scene->UpdateViewport(viewp);
		}

        ImGui::Render();
        
        _engine->render(_scene);
        _scene->Update(_deltaTime);
        calcFramerate();
        SDL_Delay(1);
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
        _framerate = 1.0f / (_deltaTime / 1000.0f);
        if (_currentFrameTime - _lastFPSupdate > 500 ) {
            //snprintf(_windowTitle, sizeof(_windowTitle), "SYMYS | FPS: %.0f", _framerate);
            SDL_SetWindowTitle(_window, _windowTitle);
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
        SDL_Quit();
    }
}
