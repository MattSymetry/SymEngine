#pragma once
#include "../../../common/config.h"

class Buffer {
public:
	vk::Buffer buffer, stagingBuffer;
	vk::DeviceMemory deviceMemory, stagingMemory;
	size_t size, stagingSize;
	vk::DescriptorBufferInfo descriptor;

	Buffer(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, size_t size, vk::BufferUsageFlags usage);

	void create_resources(vk::Device logicalDevice);

	void blit(void* data, size_t _size, vk::Queue queue, vk::CommandBuffer commandBuffer, vk::Fence fence);

	void destroy(vk::Device logicalDevice);

	void* getWriteLocation() { return writeLocation; }

private:
	void* writeLocation;
};

struct BufferInitParams {
    size_t size;
    vk::BufferUsageFlagBits usage;
    vk::DescriptorType descriptorType;
    void* dataPtr = nullptr;
};

struct BufferSetup {
    Buffer buffer;
    uint32_t dstBinding;
    vk::DescriptorType descriptorType;
    void* dataPtr = nullptr;
    size_t dataSize = 0;
};
