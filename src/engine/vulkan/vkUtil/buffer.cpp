#pragma once
#include "buffer.h"
#include "memory.h"
#include "../../scene.h"


Buffer::Buffer(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, size_t size, vk::BufferUsageFlags usage, bool hostVisible = false) {

	//Make Staging Buffer
	vkUtil::BufferInputChunk input;
	input.logicalDevice = logicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached;
	input.physicalDevice = physicalDevice;
	input.size = size;
	input.usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
	vkUtil::BufferOutputChunk  chunk = vkUtil::createBuffer(input);
    stagingBuffer = chunk.buffer;
    stagingMemory = chunk.memory;
    stagingSize = chunk.size;

	//Make Device Buffer
	if (hostVisible) {
		input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached;
		input.usage = usage | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
	} else {
		input.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		input.usage = usage | vk::BufferUsageFlagBits::eTransferDst;
	}
	chunk = vkUtil::createBuffer(input);
    buffer = chunk.buffer;
    deviceMemory = chunk.memory;
	size = chunk.size;

	create_resources(logicalDevice);
}

void Buffer::create_resources(vk::Device logicalDevice) {

	writeLocation = logicalDevice.mapMemory(stagingMemory, 0, stagingSize);

    descriptor.buffer = buffer;
    descriptor.offset = 0;
    descriptor.range = stagingSize;

}

void Buffer::blit(void* data, size_t size, vk::Queue queue, vk::CommandBuffer commandBuffer, vk::Fence fence) {

	memcpy(writeLocation, data, size);
	vkUtil::copyBuffer( 
                       stagingBuffer, buffer, size,
		queue, commandBuffer, fence
	);

}
 
void Buffer::destroy(vk::Device logicalDevice) {

	logicalDevice.unmapMemory(stagingMemory);
	logicalDevice.freeMemory(stagingMemory);
	logicalDevice.destroyBuffer(stagingBuffer);

	logicalDevice.freeMemory(deviceMemory);
	logicalDevice.destroyBuffer(buffer);

}
