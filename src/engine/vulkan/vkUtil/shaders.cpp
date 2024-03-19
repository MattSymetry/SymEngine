#include "shaders.h"
#include "../../logging.h"

#if defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <libgen.h>
    #include <limits.h>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

std::string getExecutablePath() {
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

std::string getExecutableDirectory() {
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
    std::cout << "Reading file: " << executableDir+filename << std::endl;
	if (!file.is_open()) {
		std::stringstream message;
		message << "Failed to load \"" << filename << "\"";
		vkLogging::Logger::get_logger()->print(message.str());
	}

	size_t filesize{ static_cast<size_t>(file.tellg()) };
    std::cout << "Filesize: " << filesize << std::endl;
	std::vector<char> buffer(filesize);
	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();
	return buffer;
}

vk::ShaderModule vkUtil::createModule(std::string filename, vk::Device device) {

	std::vector<char> sourceCode = readFile(filename);

	vk::ShaderModuleCreateInfo moduleInfo = {};
	moduleInfo.flags = vk::ShaderModuleCreateFlags();
	moduleInfo.codeSize = sourceCode.size();
	moduleInfo.pCode = reinterpret_cast<const uint32_t*>(sourceCode.data());

	try {
		return device.createShaderModule(moduleInfo);
	}
	catch (vk::SystemError err) {
		std::stringstream message;
		message << "Failed to create shader module for \"" << filename << "\"";
		vkLogging::Logger::get_logger()->print(message.str());
	}
}
