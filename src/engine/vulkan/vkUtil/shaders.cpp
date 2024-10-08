#pragma once
#include "shaders.h"
#include "../../logging.h"
#include "glslang/Public/ShaderLang.h"

#if defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <libgen.h>
    #include <limits.h>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

glslang_resource_t vkUtil::get_default_resource() {
    glslang_resource_t r = {
        /* .MaxLights = */ 32,
        /* .MaxClipPlanes = */ 6,
        /* .MaxTextureUnits = */ 32,
        /* .MaxTextureCoords = */ 32,
        /* .MaxVertexAttribs = */ 64,
        /* .MaxVertexUniformComponents = */ 4096,
        /* .MaxVaryingFloats = */ 64,
        /* .MaxVertexTextureImageUnits = */ 32,
        /* .MaxCombinedTextureImageUnits = */ 80,
        /* .MaxTextureImageUnits = */ 32,
        /* .MaxFragmentUniformComponents = */ 4096,
        /* .MaxDrawBuffers = */ 32,
        /* .MaxVertexUniformVectors = */ 128,
        /* .MaxVaryingVectors = */ 8,
        /* .MaxFragmentUniformVectors = */ 16,
        /* .MaxVertexOutputVectors = */ 16,
        /* .MaxFragmentInputVectors = */ 15,
        /* .MinProgramTexelOffset = */ -8,
        /* .MaxProgramTexelOffset = */ 7,
        /* .MaxClipDistances = */ 8,
        /* .MaxComputeWorkGroupCountX = */ 65535,
        /* .MaxComputeWorkGroupCountY = */ 65535,
        /* .MaxComputeWorkGroupCountZ = */ 65535,
        /* .MaxComputeWorkGroupSizeX = */ 1024,
        /* .MaxComputeWorkGroupSizeY = */ 1024,
        /* .MaxComputeWorkGroupSizeZ = */ 64,
        /* .MaxComputeUniformComponents = */ 1024,
        /* .MaxComputeTextureImageUnits = */ 16,
        /* .MaxComputeImageUniforms = */ 8,
        /* .MaxComputeAtomicCounters = */ 8,
        /* .MaxComputeAtomicCounterBuffers = */ 1,
        /* .MaxVaryingComponents = */ 60,
        /* .MaxVertexOutputComponents = */ 64,
        /* .MaxGeometryInputComponents = */ 64,
        /* .MaxGeometryOutputComponents = */ 128,
        /* .MaxFragmentInputComponents = */ 128,
        /* .MaxImageUnits = */ 8,
        /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
        /* .MaxCombinedShaderOutputResources = */ 8,
        /* .MaxImageSamples = */ 0,
        /* .MaxVertexImageUniforms = */ 0,
        /* .MaxTessControlImageUniforms = */ 0,
        /* .MaxTessEvaluationImageUniforms = */ 0,
        /* .MaxGeometryImageUniforms = */ 0,
        /* .MaxFragmentImageUniforms = */ 8,
        /* .MaxCombinedImageUniforms = */ 8,
        /* .MaxGeometryTextureImageUnits = */ 16,
        /* .MaxGeometryOutputVertices = */ 256,
        /* .MaxGeometryTotalOutputComponents = */ 1024,
        /* .MaxGeometryUniformComponents = */ 1024,
        /* .MaxGeometryVaryingComponents = */ 64,
        /* .MaxTessControlInputComponents = */ 128,
        /* .MaxTessControlOutputComponents = */ 128,
        /* .MaxTessControlTextureImageUnits = */ 16,
        /* .MaxTessControlUniformComponents = */ 1024,
        /* .MaxTessControlTotalOutputComponents = */ 4096,
        /* .MaxTessEvaluationInputComponents = */ 128,
        /* .MaxTessEvaluationOutputComponents = */ 128,
        /* .MaxTessEvaluationTextureImageUnits = */ 16,
        /* .MaxTessEvaluationUniformComponents = */ 1024,
        /* .MaxTessPatchComponents = */ 120,
        /* .MaxPatchVertices = */ 32,
        /* .MaxTessGenLevel = */ 64,
        /* .MaxViewports = */ 16,
        /* .MaxVertexAtomicCounters = */ 0,
        /* .MaxTessControlAtomicCounters = */ 0,
        /* .MaxTessEvaluationAtomicCounters = */ 0,
        /* .MaxGeometryAtomicCounters = */ 0,
        /* .MaxFragmentAtomicCounters = */ 8,
        /* .MaxCombinedAtomicCounters = */ 8,
        /* .MaxAtomicCounterBindings = */ 1,
        /* .MaxVertexAtomicCounterBuffers = */ 0,
        /* .MaxTessControlAtomicCounterBuffers = */ 0,
        /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
        /* .MaxGeometryAtomicCounterBuffers = */ 0,
        /* .MaxFragmentAtomicCounterBuffers = */ 1,
        /* .MaxCombinedAtomicCounterBuffers = */ 1,
        /* .MaxAtomicCounterBufferSize = */ 16384,
        /* .MaxTransformFeedbackBuffers = */ 4,
        /* .MaxTransformFeedbackInterleavedComponents = */ 64,
        /* .MaxCullDistances = */ 8,
        /* .MaxCombinedClipAndCullDistances = */ 8,
        /* .MaxSamples = */ 4,
        /* .maxMeshOutputVerticesNV = */ 256,
        /* .maxMeshOutputPrimitivesNV = */ 512,
        /* .maxMeshWorkGroupSizeX_NV = */ 32,
        /* .maxMeshWorkGroupSizeY_NV = */ 1,
        /* .maxMeshWorkGroupSizeZ_NV = */ 1,
        /* .maxTaskWorkGroupSizeX_NV = */ 32,
        /* .maxTaskWorkGroupSizeY_NV = */ 1,
        /* .maxTaskWorkGroupSizeZ_NV = */ 1,
        /* .maxMeshViewCountNV = */ 4,
        /* .maxMeshOutputVerticesEXT = */ 256,
        /* .maxMeshOutputPrimitivesEXT = */ 256,
        /* .maxMeshWorkGroupSizeX_EXT = */ 128,
        /* .maxMeshWorkGroupSizeY_EXT = */ 128,
        /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
        /* .maxTaskWorkGroupSizeX_EXT = */ 128,
        /* .maxTaskWorkGroupSizeY_EXT = */ 128,
        /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
        /* .maxMeshViewCountEXT = */ 4,
        /* .maxDualSourceDrawBuffersEXT = */ 1,

        /* .limits = */ {
            /* .nonInductiveForLoops = */ 1,
            /* .whileLoops = */ 1,
            /* .doWhileLoops = */ 1,
            /* .generalUniformIndexing = */ 1,
            /* .generalAttributeMatrixVectorIndexing = */ 1,
            /* .generalVaryingIndexing = */ 1,
            /* .generalSamplerIndexing = */ 1,
            /* .generalVariableIndexing = */ 1,
            /* .generalConstantMatrixVectorIndexing = */ 1,
        }
    };
    return r;
}

std::string vkUtil::getExecutablePath() {
#if defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        // Buffer size is too small.
        return "";
    }
#elif defined(_WIN32)
    char path[MAX_PATH];
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule != NULL) {
        // Use GetModuleFileName() with a NULL HMODULE to get the path of the executable
        GetModuleFileName(hModule, path, MAX_PATH);
    }
#endif

    return std::string(path);
}

std::string vkUtil::getExecutableDirectory() {
    std::string executablePath = getExecutablePath();
    if (executablePath.empty()) {
        return "";
    }

#if defined(__APPLE__)
    char* pathPtr = strdup(executablePath.c_str());
    std::string directory = std::string(dirname(pathPtr));
    free(pathPtr);
#elif defined(_WIN32)
    size_t pos = executablePath.find_last_of("\\/");
    std::string directory = executablePath.substr(0, pos);
#endif

    return directory;
}

std::vector<char> vkUtil::readFile(std::string filename) {
    std::string executableDir = getExecutableDirectory();
	std::ifstream file(executableDir+filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::stringstream message;
		message << "Failed to load \"" << filename << "\"";
		vkLogging::Logger::get_logger()->print(message.str());
	}

	size_t filesize{ static_cast<size_t>(file.tellg()) };
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer; 
}

std::vector<uint32_t> vkUtil::compileShaderSourceToSpirv(std::string& shaderSource, const std::string& inputFilename, glslang_stage_t shaderStage, bool onlyCheckCode, char** error) {
    std::vector<uint32_t> resultingSpirv;
    if (onlyCheckCode) {
        std::vector<char> sourceCode = prepareShader();
        std::vector<char> tmp(shaderSource.begin(), shaderSource.end());
        sourceCode.insert(sourceCode.end(), tmp.begin(), tmp.end());
        std::string mapCode = "SDFData map(in vec3 pos) {return SDFData(vec4(1.0, 0.0, 0.0, 0.0), -1);} \n";
        tmp = std::vector<char>(mapCode.begin(), mapCode.end());
        sourceCode.insert(sourceCode.end(), tmp.begin(), tmp.end());
        tmp = endShader();
        sourceCode.insert(sourceCode.end(), tmp.begin(), tmp.end());

        std::string str(sourceCode.begin(), sourceCode.end());
        shaderSource = str;
    }
    const char* shaderCode = shaderSource.c_str();
    static const auto defaultResources = get_default_resource();

    glslang_input_t input = {};
    input.language = GLSLANG_SOURCE_GLSL;
    input.stage = shaderStage;
    input.client = GLSLANG_CLIENT_VULKAN;
    input.client_version = GLSLANG_TARGET_VULKAN_1_3;
    input.target_language = GLSLANG_TARGET_SPV;
    input.target_language_version = GLSLANG_TARGET_SPV_1_3;
    input.code = shaderCode;
    input.default_version = 100;
    input.default_profile = GLSLANG_ES_PROFILE;
    input.force_default_version_and_profile = false;
    input.forward_compatible = false;
    input.messages = GLSLANG_MSG_DEFAULT_BIT;
    input.resource = &defaultResources;
     
    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input))
    {
        if (onlyCheckCode)
        {
           *error = new char[strlen(glslang_shader_get_info_log(shader)) + 1];
           strcpy(*error, glslang_shader_get_info_log(shader));
		}
        else {
            std::cout << "\nERROR:   Failed to preprocess shader[" << inputFilename << "] of kind["
                << "\n         Log[" << glslang_shader_get_info_log(shader) << "]"
                << "\n         Debug-Log[" << glslang_shader_get_info_debug_log(shader) << "]" << std::endl;
        }
        return resultingSpirv;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        if (onlyCheckCode)
        {
            *error = new char[strlen(glslang_shader_get_info_log(shader)) + 1];
            strcpy(*error, glslang_shader_get_info_log(shader));
        }
        else {
            std::cout << "\nERROR:   Failed to parse shader[" << inputFilename << "] of kind["
                << "\n         Log[" << glslang_shader_get_info_log(shader) << "]"
                << "\n         Debug-Log[" << glslang_shader_get_info_debug_log(shader) << "]" << std::endl;
        }
        return resultingSpirv;
    }

    if (onlyCheckCode)
    {
        return resultingSpirv;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        std::cout << "\nERROR:   Failed to link shader[" << inputFilename << "] of kind["
                  << "\n         Log[" << glslang_shader_get_info_log(shader) << "]"
                  << "\n         Debug-Log[" << glslang_shader_get_info_debug_log(shader) << "]" << std::endl;
        return resultingSpirv;
    }

    glslang_program_SPIRV_generate(program, input.stage);

    if (glslang_program_SPIRV_get_messages(program))
    {
        printf("%s", glslang_program_SPIRV_get_messages(program));
        std::cout << "\nINFO:    Got messages for shader[" << inputFilename
                  << "\n         Message[" << glslang_program_SPIRV_get_messages(program) << "]" << std::endl;
    }

    auto* spirvDataPtr = glslang_program_SPIRV_get_ptr(program);
    const auto spirvNumWords = glslang_program_SPIRV_get_size(program);
    resultingSpirv.insert(std::end(resultingSpirv), spirvDataPtr, spirvDataPtr + spirvNumWords);

    glslang_program_delete(program);
    glslang_shader_delete(shader);
    return resultingSpirv;
} 

std::vector<char> vkUtil::prepareShader() {
    std::vector<char> shader = LoadShaderResource(IDR_SHADER_DEF);
    auto tmp = LoadShaderResource(IDR_SHADER_CSG);
    shader.insert(shader.end(), tmp.begin(), tmp.end());
    
    return shader;
}

std::vector<char> vkUtil::endShader(bool useForOut) {
    return LoadShaderResource((useForOut) ? IDR_SHADER_RENDEROUT : IDR_SHADER_RENDER);
}

vk::ShaderModule vkUtil::createModule(std::string shaderCode, vk::Device device, bool useForOut) {

    vk::ShaderModuleCreateInfo moduleInfo = {};
    moduleInfo.flags = vk::ShaderModuleCreateFlags();
    std::vector<char> sourceCode = prepareShader();
    std::vector<char> tmp(shaderCode.begin(), shaderCode.end());
    sourceCode.insert(sourceCode.end(), tmp.begin(), tmp.end());
    tmp = endShader(useForOut);
    sourceCode.insert(sourceCode.end(), tmp.begin(), tmp.end());
    
    std::string str(sourceCode.begin(), sourceCode.end());
    auto sourceCodeUnit = compileShaderSourceToSpirv(str, "GenCode", GLSLANG_STAGE_COMPUTE);
    moduleInfo.codeSize = sourceCodeUnit.size() * sizeof(decltype(sourceCodeUnit)::value_type);
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCodeUnit.data());

	try {
		auto shaderMod = device.createShaderModule(moduleInfo);
        return shaderMod;
	}
	catch (vk::SystemError err) {
		std::stringstream message;
		message << "Failed to create shader module for \"" << "GenCode" << "\"";
		vkLogging::Logger::get_logger()->print(message.str());
	}
}

std::vector<char> vkUtil::LoadShaderResource(UINT resourceID) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), TEXT("SHADER"));
    if (hRes == NULL) {
        std::cerr << "Failed to find resource." << std::endl;
        return std::vector<char>();
    }
    HGLOBAL hResData = LoadResource(NULL, hRes);
    if (hResData == NULL) {
        std::cerr << "Failed to load resource." << std::endl;
        return std::vector<char>();
    }
    DWORD dataSize = SizeofResource(NULL, hRes);
    void* pResData = LockResource(hResData);
    if (pResData == NULL) {
        std::cerr << "Failed to lock resource." << std::endl;
        return std::vector<char>();
    }
    return std::vector<char>(static_cast<char*>(pResData), static_cast<char*>(pResData) + dataSize);
}
