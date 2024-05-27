#pragma once
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__) && defined(__MACH__)
    #define VK_USE_PLATFORM_MACOS_MVK
#endif
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>


#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../../resources/resource.h"
#include "IconsMaterialDesignIcons.h"

//--------- Assets -------------//

enum class pipelineType {
    COMPUTE,
    COMPUTE2
};

// String Processing functions

std::vector<std::string> split(std::string line, std::string delimiter);

// Random functions

float random_float();

float random_float(float min, float max);
