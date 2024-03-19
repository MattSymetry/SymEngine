#include "frame.h"
#include "memory.h"
#include "../vkImage/image.h"

vkUtil::SwapChainFrame::SwapChainFrame(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, uint32_t width, uint32_t height)
{
	this->height = height;
	this->width = width;
}

void vkUtil::SwapChainFrame::AddBuffers(const std::vector<BufferInitParams>& bufferParams, vk::Device logicalDevice, vk::PhysicalDevice physicalDevice)
{
    uint32_t i = std::max(static_cast<uint32_t>(bufferSetups.size()), 1u);
    for (const auto& params : bufferParams) {
        BufferSetup bufferSetup = {
            Buffer(logicalDevice, physicalDevice, params.size, params.usage),
            i, // dstBinding
            params.descriptorType,
            params.dataPtr,
            params.size
        };
        bufferSetups.push_back(bufferSetup);
        i++;
    }
}

void vkUtil::SwapChainFrame::make_descriptor_resources(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice) {
	
	colorBufferDescriptor.imageLayout = vk::ImageLayout::eGeneral;
	colorBufferDescriptor.imageView = imageView;
	colorBufferDescriptor.sampler = nullptr;
	
}

void vkUtil::SwapChainFrame::record_write_operations() {

	vk::WriteDescriptorSet colorBufferOp;

	colorBufferOp.dstSet = descriptorSet[pipelineType::COMPUTE];
	colorBufferOp.dstBinding = 0;
	colorBufferOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
	colorBufferOp.descriptorCount = 1;
	colorBufferOp.descriptorType = vk::DescriptorType::eStorageImage;
	colorBufferOp.pImageInfo = &colorBufferDescriptor;
    writeOps.push_back(colorBufferOp);
    
    for (auto& bufferSetup : bufferSetups) {
        vk::WriteDescriptorSet bufferOp;
        bufferOp.dstSet = descriptorSet[pipelineType::COMPUTE];
        bufferOp.dstBinding = bufferSetup.dstBinding;
        bufferOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
        bufferOp.descriptorCount = 1;
        bufferOp.descriptorType = bufferSetup.descriptorType;
        bufferOp.pBufferInfo = &bufferSetup.buffer.descriptor;
        writeOps.push_back(bufferOp);
    }
}

void vkUtil::SwapChainFrame::write_descriptor_set() {
	logicalDevice.updateDescriptorSets(writeOps, nullptr);
}

void vkUtil::SwapChainFrame::destroy() {

	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroyFence(inFlight);
	logicalDevice.destroySemaphore(imageAvailable);
	logicalDevice.destroySemaphore(renderFinished);
    for (auto& bufferSetup : bufferSetups) {
        bufferSetup.buffer.destroy(logicalDevice);
    }
}
