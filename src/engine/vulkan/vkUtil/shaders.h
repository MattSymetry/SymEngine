#pragma once
#include "../../../common/config.h"
#include <glslang/Include/glslang_c_interface.h>

namespace vkUtil {
    glslang_resource_t get_default_resource();
	std::vector<char> readFile(std::string filename);
    
    std::vector<uint32_t> compileShaderSourceToSpirv(const std::string& shaderSource, const std::string& inputFilename, glslang_stage_t shaderStage);
    
    std::vector<char> prepareShader();
    std::vector<char> endShader();

	vk::ShaderModule createModule(std::string shaderCode, vk::Device device);
    std::string getExecutablePath();
    std::string getExecutableDirectory();
}
