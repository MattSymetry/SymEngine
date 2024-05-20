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
            Buffer(logicalDevice, physicalDevice, params.size, params.usage, params.hostVisible),
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

VkRenderingInfo vkUtil::rendering_info(VkExtent2D swapchainExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
    VkRenderingInfo renderInfo {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, swapchainExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

        return renderInfo;
}

void vkUtil::transition_image(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    } else {
        // Handle other layout transitions here...
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        cmdBuffer,
        sourceStage, destinationStage,
        0, // Flags
        0, nullptr, // Memory barrier count & memory barriers
        0, nullptr, // Buffer memory barrier count & buffer memory barriers
        1, &barrier // Image memory barrier count & image memory barriers
    );
}
